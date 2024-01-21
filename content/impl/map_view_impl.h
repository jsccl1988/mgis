// Copyright (c) 2023 The MGIS Authors.
// All rights reserved.

#ifndef CONTENT_CONTROL_IMPL_MAP_VIEW_IMPL_H
#define CONTENT_CONTROL_IMPL_MAP_VIEW_IMPL_H

#include <vector>

#include "content/control/window/map_window.h"
#include "content/public/map_view.h"

namespace content {
class MapViewImpl : public MapView {
 public:
  MapViewImpl() {}

  // Property
  virtual void GetCreateParams(MapView::CreateParams* create_params) {
    *create_params = create_params_;
  }
  virtual void GetPreferences(MapView::Preferences* preferences) {
    *preferences = preferences_;
  }

  // Create/Destroy
  virtual void Create(const MapView::CreateParams& create_params,
                      const MapView::Preferences& preferences) {
    create_params_ = create_params;
    preferences_ = preferences;
    window_.Create(create_params.parent_hwnd, CWindow::rcDefault,
                   TEXT("SmartGIS"), WS_OVERLAPPEDWINDOW | WS_VISIBLE);
  }
  virtual void Destroy() {}

 private:
  MapView::CreateParams create_params_;
  MapView::Preferences preferences_;
  MapWindow window_;

 protected:
  friend class base::RefCountedThreadSafe<MapViewImpl>;
  virtual ~MapViewImpl() {}
};
}  // namespace content
#endif  // CONTENT_CONTROL_IMPL_MAP_VIEW_IMPL_H
