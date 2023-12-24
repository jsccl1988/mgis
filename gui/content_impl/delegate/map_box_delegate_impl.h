// Copyright (c) 2023 The MGIS Authors.
// All rights reserved.

#ifndef GUI_CONTENT_IMPL_DELEGATE_MAP_BOX_DELEGATE_IMPL_H
#define GUI_CONTENT_IMPL_DELEGATE_MAP_BOX_DELEGATE_IMPL_H

#include "content/public/delegate/map_box_delegate.h"

namespace gui {
class MapBoxDelegateImpl :public content::MapBoxDelegate {
 public:
  MapBoxDelegateImpl() {}
  virtual ~MapBoxDelegateImpl() {}

 public:
  void PreMainMessageLoopRun() override;
};
}  // namespace gui
#endif  // GUI_CONTENT_IMPL_DELEGATE_MAP_BOX_DELEGATE_IMPL_H
