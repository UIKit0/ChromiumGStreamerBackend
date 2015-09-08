// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_BUBBLE_BUBBLE_CONTROLLER_H_
#define COMPONENTS_BUBBLE_BUBBLE_CONTROLLER_H_

#include "base/memory/scoped_ptr.h"
#include "base/memory/weak_ptr.h"
#include "base/threading/thread_checker.h"
#include "components/bubble/bubble_close_reason.h"

class BubbleDelegate;
class BubbleManager;
class BubbleUi;

// BubbleController is responsible for the lifetime of the delegate and its UI.
class BubbleController : public base::SupportsWeakPtr<BubbleController> {
 public:
  explicit BubbleController(BubbleManager* manager,
                            scoped_ptr<BubbleDelegate> delegate);
  virtual ~BubbleController();

  // Calls CloseBubble on the associated BubbleManager.
  bool CloseBubble(BubbleCloseReason reason);

  // Calls UpdateBubbleUi on the associated BubbleManager.
  // Returns true if the UI was updated.
  bool UpdateBubbleUi();

 private:
  friend class BubbleManager;

  // Creates and shows the UI for the delegate.
  void Show();

  // Notifies the bubble UI that it should update its anchor location.
  // Important when there's a UI change (ex: fullscreen transition).
  void UpdateAnchorPosition();

  // Cleans up the delegate and its UI if it closed.
  // Returns true if the bubble was closed.
  bool ShouldClose(BubbleCloseReason reason);

  BubbleManager* manager_;
  scoped_ptr<BubbleDelegate> delegate_;
  scoped_ptr<BubbleUi> bubble_ui_;

  // Verify that functions that affect the UI are done on the same thread.
  base::ThreadChecker thread_checker_;

  DISALLOW_COPY_AND_ASSIGN(BubbleController);
};

#endif  // COMPONENTS_BUBBLE_BUBBLE_CONTROLLER_H_
