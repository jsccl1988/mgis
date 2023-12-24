// Copyright (c) 2023 The MGIS Authors.
// All rights reserved.


#include "gui/content_impl/delegate/content_client_impl.h"

#include "gui/browser.h"
#include "gui/browser_frame.h"

#include "gui/content_impl/delegate/map_box_delegate_impl.h"

namespace gui {
void MapContentClientImpl::QueryMapBoxDelegate(
  content_impl::MapBoxDelegate ** delegate) {
  if (!map_box_delegate_) {
    map_box_delegate_ = new MapBoxDelegateImpl();
  }

  *delegate = map_box_delegate_.get();
}

void MapContentClientImpl::QueryMapViewDelegate(uint32 id
  , content_impl::MapViewDelegate ** delegate) {
  // BrowserFrame * browser_frame = Browser::Instance()->GetBrowserFrame(id);
  // if (browser_frame) {
  //   *delegate = browser_frame->map_view_delegate();
  // }
}
}  // namespace gui
