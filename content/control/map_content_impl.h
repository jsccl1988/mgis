// Copyright (c) 2023 The MGIS Authors.
// All rights reserved.

#ifndef CONTENT_CONTROL_MAP_CONTENT_IMPL_H
#define CONTENT_CONTROL_MAP_CONTENT_IMPL_H

#include "content/public/map_content.h"

namespace content {
class MapContentImpl : public MapContent {
 public:
  MapContentImpl() {}
  virtual ~MapContentImpl() {}

 public:
  virtual bool GetVersion(std::string* version)  {}
  virtual bool Create(const base::WeakPtr<MapContentClient>& map_content_client,
                      const base::WeakPtr<TaskRunner>& task_runner,
                      const MapContent::Preferences& preferences,
                      MapBox** map_box)  {}
  virtual bool Destroy()  {}
};

static const char* CreateMapContentFnName = "CreateMapContent";
typedef int32_t (*CreateMapContentFnPtr)(content::MapContent** map_content);
}  // namespace content

#endif  // CONTENT_CONTROL_MAP_CONTENT_IMPL_H
