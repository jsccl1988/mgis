// Copyright (c) 2023 The MGIS Authors.
// All rights reserved.

#ifndef GUI_CONTENT_IMPL_MAP_CONTENT_FACTORY_H
#define GUI_CONTENT_IMPL_MAP_CONTENT_FACTORY_H

#include <map>

#include "base/synchronization/lock.h"
#include "base/memory/ref_counted.h"
#include "content/public/map_box/map_box.h"
#include "content/public/map_box/map_view.h"
#include "content/public/map_content.h"

namespace gui {
class MapContentFactory {
 public:
  MapContentFactory();
  ~MapContentFactory();

 public:
  bool CreateMapContent(content::MapContent** map_content);
};
}  // namespace gui
#endif  //  GUI_CONTENT_IMPL_MAP_CONTENT_FACTORY_H
