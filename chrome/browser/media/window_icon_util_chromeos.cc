// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/media/window_icon_util.h"

#include "content/public/browser/desktop_media_id.h"
#include "ui/aura/client/aura_constants.h"
#include "ui/aura/window.h"

gfx::ImageSkia GetWindowIcon(content::DesktopMediaID id) {
  DCHECK_EQ(content::DesktopMediaID::TYPE_WINDOW, id.type);
  aura::Window* window = content::DesktopMediaID::GetAuraWindowById(id);
  if (!window)
    return gfx::ImageSkia();

  const gfx::ImageSkia* icon_image_ptr =
      window->GetProperty(aura::client::kWindowIconKey);

  return icon_image_ptr ? *icon_image_ptr : gfx::ImageSkia();
}
