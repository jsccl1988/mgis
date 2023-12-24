// Copyright (c) 2023 The MGIS Authors.
// All rights reserved.


#ifndef GUI_CONTENT_IMPL_MAP_CONTENT_FACTORY_H
#define GUI_CONTENT_IMPL_MAP_CONTENT_FACTORY_H

#include <map>

#include "base/lock.h"
#include "base/memory/ref_counted.h"

#include "content.h"

#include "browser/map_box.h"
#include "browser/map_view.h"

namespace gui {
class MapContent;
class MapContentFactory {
 public:
  MapContentFactory();
  ~MapContentFactory();

 public:
  bool CreateMapContent(content::MapContent ** we_brender);
};
}  // namespace gui
#endif  //  GUI_CONTENT_IMPL_MAP_CONTENT_FACTORY_H
