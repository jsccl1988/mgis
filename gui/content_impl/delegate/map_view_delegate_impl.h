// Copyright (c) 2023 The MGIS Authors.
// All rights reserved.

#ifndef DEMO_WEB_RENDER_DELEGATE_MAP_VIEW_DELEGATE_IMPL_H_
#define DEMO_WEB_RENDER_DELEGATE_MAP_VIEW_DELEGATE_IMPL_H_

#include <vector>

#include "content/public/delegate/map_view_delegate.h"
#include "content/public/map_box/map_view.h"

namespace gui {
class MapViewDelegateImpl : public content::MapViewDelegate {
 public:
  MapViewDelegateImpl();
  virtual ~MapViewDelegateImpl();

 public:
  void OnStatusChange(const std::string& status) override;
};
}  // namespace gui
#endif  // DEMO_WEB_RENDER_DELEGATE_MAP_VIEW_DELEGATE_IMPL_H_
