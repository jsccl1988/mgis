// Copyright (c) 2023 The MGIS Authors.
// All rights reserved.

#ifndef GUI_CONTENT_IMPL_MAP_CONTENT_DELEGATE_IMPL_H
#define GUI_CONTENT_IMPL_MAP_CONTENT_DELEGATE_IMPL_H

#include "content/public/map_content_client.h"

namespace gui {
class MapContentClientImpl : public content::MapContentClient {
 public:
  MapContentClientImpl() {}
  virtual ~MapContentClientImpl() {}

 public:
  void QueryMapBoxDelegate(content::MapBoxDelegate** delegate) override;
  void QueryMapViewDelegate(content::MapViewDelegate** delegate) override;

 private:
  base::ScopedRefPtr<content::MapBoxDelegate> map_box_delegate_;
  base::ScopedRefPtr<content::MapViewDelegate> map_view_delegate_;
};
}  // namespace gui

#endif  // GUI_CONTENT_IMPL_MAP_CONTENT_DELEGATE_IMPL_H
