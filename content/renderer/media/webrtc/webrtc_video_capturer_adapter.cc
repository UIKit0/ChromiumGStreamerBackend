// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/renderer/media/webrtc/webrtc_video_capturer_adapter.h"

#include "base/bind.h"
#include "base/memory/aligned_memory.h"
#include "base/trace_event/trace_event.h"
#include "content/renderer/media/webrtc/webrtc_video_frame_adapter.h"
#include "media/base/timestamp_constants.h"
#include "media/base/video_util.h"
#include "third_party/libyuv/include/libyuv/convert_from.h"
#include "third_party/libyuv/include/libyuv/scale.h"
#include "third_party/webrtc/common_video/include/video_frame_buffer.h"
#include "third_party/webrtc/common_video/rotation.h"
#include "third_party/webrtc/media/engine/webrtcvideoframe.h"

namespace content {
namespace {

// Empty method used for keeping a reference to the original media::VideoFrame.
// The reference to |frame| is kept in the closure that calls this method.
void ReleaseOriginalFrame(const scoped_refptr<media::VideoFrame>& frame) {
}

}  // anonymous namespace

WebRtcVideoCapturerAdapter::WebRtcVideoCapturerAdapter(bool is_screencast)
    : is_screencast_(is_screencast),
      running_(false) {
  thread_checker_.DetachFromThread();
}

WebRtcVideoCapturerAdapter::~WebRtcVideoCapturerAdapter() {
  DVLOG(3) << " WebRtcVideoCapturerAdapter::dtor";
}

cricket::CaptureState WebRtcVideoCapturerAdapter::Start(
    const cricket::VideoFormat& capture_format) {
  DCHECK(thread_checker_.CalledOnValidThread());
  DCHECK(!running_);
  DVLOG(3) << " WebRtcVideoCapturerAdapter::Start w = " << capture_format.width
           << " h = " << capture_format.height;

  running_ = true;
  return cricket::CS_RUNNING;
}

void WebRtcVideoCapturerAdapter::Stop() {
  DCHECK(thread_checker_.CalledOnValidThread());
  DVLOG(3) << " WebRtcVideoCapturerAdapter::Stop ";
  DCHECK(running_);
  running_ = false;
  SetCaptureFormat(NULL);
  SignalStateChange(this, cricket::CS_STOPPED);
}

bool WebRtcVideoCapturerAdapter::IsRunning() {
  DCHECK(thread_checker_.CalledOnValidThread());
  return running_;
}

bool WebRtcVideoCapturerAdapter::GetPreferredFourccs(
    std::vector<uint32_t>* fourccs) {
  DCHECK(thread_checker_.CalledOnValidThread());
  DCHECK(!fourccs || fourccs->empty());
  if (fourccs)
    fourccs->push_back(cricket::FOURCC_I420);
  return fourccs != NULL;
}

bool WebRtcVideoCapturerAdapter::IsScreencast() const {
  return is_screencast_;
}

bool WebRtcVideoCapturerAdapter::GetBestCaptureFormat(
    const cricket::VideoFormat& desired,
    cricket::VideoFormat* best_format) {
  DCHECK(thread_checker_.CalledOnValidThread());
  DVLOG(3) << " GetBestCaptureFormat:: "
           << " w = " << desired.width
           << " h = " << desired.height;

  // Capability enumeration is done in MediaStreamVideoSource. The adapter can
  // just use what is provided.
  // Use the desired format as the best format.
  best_format->width = desired.width;
  best_format->height = desired.height;
  best_format->fourcc = cricket::FOURCC_I420;
  best_format->interval = desired.interval;
  return true;
}

void WebRtcVideoCapturerAdapter::OnFrameCaptured(
    const scoped_refptr<media::VideoFrame>& input_frame) {
  DCHECK(thread_checker_.CalledOnValidThread());
  TRACE_EVENT0("video", "WebRtcVideoCapturerAdapter::OnFrameCaptured");
  if (!(input_frame->IsMappable() &&
        (input_frame->format() == media::PIXEL_FORMAT_I420 ||
         input_frame->format() == media::PIXEL_FORMAT_YV12 ||
         input_frame->format() == media::PIXEL_FORMAT_YV12A))) {
    // Since connecting sources and sinks do not check the format, we need to
    // just ignore formats that we can not handle.
    NOTREACHED();
    return;
  }
  scoped_refptr<media::VideoFrame> frame = input_frame;
  // Drop alpha channel since we do not support it yet.
  if (frame->format() == media::PIXEL_FORMAT_YV12A)
    frame = media::WrapAsI420VideoFrame(input_frame);

  const int orig_width = frame->natural_size().width();
  const int orig_height = frame->natural_size().height();
  int adapted_width;
  int adapted_height;
  // The VideoAdapter is only used for cpu-adaptation downscaling, no
  // aspect changes. So we ignore these crop-related outputs.
  int crop_width;
  int crop_height;
  int crop_x;
  int crop_y;
  int64_t translated_camera_time_us;

  if (!AdaptFrame(orig_width, orig_height,
                  frame->timestamp().InMicroseconds(),
                  rtc::TimeMicros(),
                  &adapted_width, &adapted_height,
                  &crop_width, &crop_height, &crop_x, &crop_y,
                  &translated_camera_time_us)) {
    return;
  }

  // Return |frame| directly if it is texture backed, because there is no
  // cropping support for texture yet. See http://crbug/503653.
  // Return |frame| directly if it is GpuMemoryBuffer backed, as we want to
  // keep the frame on native buffers.
  if (frame->HasTextures() ||
      frame->storage_type() ==
      media::VideoFrame::STORAGE_GPU_MEMORY_BUFFERS) {
    OnFrame(cricket::WebRtcVideoFrame(
                new rtc::RefCountedObject<WebRtcVideoFrameAdapter>(frame),
                webrtc::kVideoRotation_0, translated_camera_time_us),
            orig_width, orig_height);
    return;
  }

  // Create a centered cropped visible rect that preservers aspect ratio for
  // cropped natural size.
  gfx::Rect visible_rect = frame->visible_rect();
  visible_rect.ClampToCenteredSize(gfx::Size(
      visible_rect.width() * adapted_width / orig_width,
      visible_rect.height() * adapted_height / orig_height));

  const gfx::Size adapted_size(adapted_width, adapted_height);
  scoped_refptr<media::VideoFrame> video_frame =
      media::VideoFrame::WrapVideoFrame(frame, frame->format(),
                                        visible_rect, adapted_size);
  if (!video_frame)
    return;

  video_frame->AddDestructionObserver(base::Bind(&ReleaseOriginalFrame, frame));

  // If no scaling is needed, return a wrapped version of |frame| directly.
  if (video_frame->natural_size() == video_frame->visible_rect().size()) {
    OnFrame(cricket::WebRtcVideoFrame(
                new rtc::RefCountedObject<WebRtcVideoFrameAdapter>(video_frame),
                webrtc::kVideoRotation_0, translated_camera_time_us),
            orig_width, orig_height);
    return;
  }

  // We need to scale the frame before we hand it over to webrtc.
  scoped_refptr<media::VideoFrame> scaled_frame =
      scaled_frame_pool_.CreateFrame(media::PIXEL_FORMAT_I420, adapted_size,
                                     gfx::Rect(adapted_size), adapted_size,
                                     frame->timestamp());
  libyuv::I420Scale(video_frame->visible_data(media::VideoFrame::kYPlane),
                    video_frame->stride(media::VideoFrame::kYPlane),
                    video_frame->visible_data(media::VideoFrame::kUPlane),
                    video_frame->stride(media::VideoFrame::kUPlane),
                    video_frame->visible_data(media::VideoFrame::kVPlane),
                    video_frame->stride(media::VideoFrame::kVPlane),
                    video_frame->visible_rect().width(),
                    video_frame->visible_rect().height(),
                    scaled_frame->data(media::VideoFrame::kYPlane),
                    scaled_frame->stride(media::VideoFrame::kYPlane),
                    scaled_frame->data(media::VideoFrame::kUPlane),
                    scaled_frame->stride(media::VideoFrame::kUPlane),
                    scaled_frame->data(media::VideoFrame::kVPlane),
                    scaled_frame->stride(media::VideoFrame::kVPlane),
                    adapted_width, adapted_height, libyuv::kFilterBilinear);

  OnFrame(cricket::WebRtcVideoFrame(
              new rtc::RefCountedObject<WebRtcVideoFrameAdapter>(scaled_frame),
              webrtc::kVideoRotation_0, translated_camera_time_us),
          orig_width, orig_height);
}

}  // namespace content
