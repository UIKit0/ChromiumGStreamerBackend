// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/views/location_bar/location_bar_bubble_delegate_view.h"

#include "chrome/browser/chrome_notification_types.h"
#include "chrome/browser/ui/browser.h"
#include "chrome/browser/ui/browser_finder.h"
#include "chrome/browser/ui/exclusive_access/fullscreen_controller.h"
#include "chrome/browser/ui/layout_constants.h"
#include "content/public/browser/notification_source.h"
#include "content/public/browser/render_view_host.h"
#include "ui/base/material_design/material_design_controller.h"
#include "ui/gfx/geometry/rect.h"

LocationBarBubbleDelegateView::WebContentMouseHandler::WebContentMouseHandler(
    LocationBarBubbleDelegateView* bubble,
    content::WebContents* web_contents)
    : bubble_(bubble), web_contents_(web_contents) {
  DCHECK(bubble_);
  DCHECK(web_contents_);
  event_monitor_ = views::EventMonitor::CreateWindowMonitor(
      this, web_contents_->GetTopLevelNativeWindow());
}

LocationBarBubbleDelegateView::WebContentMouseHandler::
    ~WebContentMouseHandler() {}

void LocationBarBubbleDelegateView::WebContentMouseHandler::OnKeyEvent(
    ui::KeyEvent* event) {
  if ((event->key_code() == ui::VKEY_ESCAPE ||
       web_contents_->GetRenderViewHost()->IsFocusedElementEditable()) &&
      event->type() == ui::ET_KEY_PRESSED)
    bubble_->CloseBubble();
}

void LocationBarBubbleDelegateView::WebContentMouseHandler::OnMouseEvent(
    ui::MouseEvent* event) {
  if (event->type() == ui::ET_MOUSE_PRESSED)
    bubble_->CloseBubble();
}

void LocationBarBubbleDelegateView::WebContentMouseHandler::OnTouchEvent(
    ui::TouchEvent* event) {
  if (event->type() == ui::ET_TOUCH_PRESSED)
    bubble_->CloseBubble();
}

LocationBarBubbleDelegateView::LocationBarBubbleDelegateView(
    views::View* anchor_view,
    content::WebContents* web_contents)
    : BubbleDialogDelegateView(anchor_view,
                               anchor_view ? views::BubbleBorder::TOP_RIGHT
                                           : views::BubbleBorder::NONE) {
  // Add observer to close the bubble if the fullscreen state changes.
  if (web_contents) {
    Browser* browser = chrome::FindBrowserWithWebContents(web_contents);
    registrar_.Add(
        this, chrome::NOTIFICATION_FULLSCREEN_CHANGED,
        content::Source<FullscreenController>(
            browser->exclusive_access_manager()->fullscreen_controller()));
  }
  // Compensate for built-in vertical padding in the anchor view's image.
  set_anchor_view_insets(gfx::Insets(
      GetLayoutConstant(LOCATION_BAR_BUBBLE_ANCHOR_VERTICAL_INSET), 0));
}

LocationBarBubbleDelegateView::~LocationBarBubbleDelegateView() {}

void LocationBarBubbleDelegateView::ShowForReason(DisplayReason reason) {
  if (reason == USER_GESTURE) {
    // In the USER_GESTURE case, the icon will be in an active state so the
    // bubble doesn't need an arrow.
    if (ui::MaterialDesignController::IsModeMaterial())
      SetArrowPaintType(views::BubbleBorder::PAINT_TRANSPARENT);
    GetWidget()->Show();
  } else {
    GetWidget()->ShowInactive();
  }
}

int LocationBarBubbleDelegateView::GetDialogButtons() const {
  return ui::DIALOG_BUTTON_NONE;
}

void LocationBarBubbleDelegateView::Observe(
    int type,
    const content::NotificationSource& source,
    const content::NotificationDetails& details) {
  DCHECK_EQ(chrome::NOTIFICATION_FULLSCREEN_CHANGED, type);
  GetWidget()->SetVisibilityAnimationTransition(views::Widget::ANIMATE_NONE);
  CloseBubble();
}

void LocationBarBubbleDelegateView::CloseBubble() {
  GetWidget()->Close();
}

void LocationBarBubbleDelegateView::AdjustForFullscreen(
    const gfx::Rect& screen_bounds) {
  if (GetAnchorView())
    return;

  const int kBubblePaddingFromScreenEdge = 20;
  int horizontal_offset = width() / 2 + kBubblePaddingFromScreenEdge;
  const int x_pos = base::i18n::IsRTL()
                        ? (screen_bounds.x() + horizontal_offset)
                        : (screen_bounds.right() - horizontal_offset);
  SetAnchorRect(gfx::Rect(x_pos, screen_bounds.y(), 0, 0));
}
