// Copyright (c) 2023 The MGIS Authors.
// All rights reserved.

#include "gui/content_impl/map_content_client_impl.h"

#include "gui/content_impl/delegate/map_box_delegate_impl.h"
#include "gui/content_impl/delegate/map_view_delegate_impl.h"

namespace gui {
void MapContentClientImpl::QueryMapBoxDelegate(
    content::MapBoxDelegate** delegate) {
  if (!map_box_delegate_) {
    map_box_delegate_ = new MapBoxDelegateImpl();
  }

  *delegate = map_box_delegate_.get();
}

void MapContentClientImpl::QueryMapViewDelegate(
    content::MapViewDelegate** delegate) {
  if (!map_view_delegate_) {
    map_view_delegate_ = new MapViewDelegateImpl();
  }

  *delegate = map_view_delegate_.get();
}
}  // namespace gui
