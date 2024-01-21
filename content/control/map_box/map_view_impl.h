// Copyright (c) 2023 The MGIS Authors.
// All rights reserved.

#ifndef CONTENT_CONTROL_MAP_BOX_MAP_VIEW_IMPL_H
#define CONTENT_CONTROL_MAP_BOX_MAP_VIEW_IMPL_H

#include <vector>

#include "content/control/map_box/map_window.h"
#include "content/public/map_box/map_view.h"

namespace content {
class CONTENT_EXPORT MapViewImpl : MapView {
 public:
  MapViewImpl() {}
  virtual ~MapViewImpl() {}

  // Property
  virtual void GetCreateParams(MapView::CreateParams* params) {}
  virtual void GetPreferences(MapView::Preferences* preferences) {}

  // Create/Destroy
  virtual void Create(const MapView::CreateParams& params,
                      const MapView::Preferences& preferences) {
    window_.Create(params.parent_hwnd, CWindow::rcDefault, TEXT("SmartGIS"),
                   WS_OVERLAPPEDWINDOW | WS_VISIBLE);
  }
  virtual void Destroy() {}

  // View Control
  virtual void Resize(const Rect& rect) {}

 private:
  MapWindow window_;
};
}  // namespace content
#endif  // CONTENT_CONTROL_MAP_BOX_MAP_VIEW_IMPL_H
