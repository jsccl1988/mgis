// Copyright (c) 2023 The MGIS Authors.
// All rights reserved.

#ifndef CONTENT_PUBLIC_MAP_CONTENT_H
#define CONTENT_PUBLIC_MAP_CONTENT_H

#include "base/basictypes.h"
#include "base/memory/ref_counted.h"
#include "base/memory/weak_ptr.h"
#include "content/content_export.h"
#include "content/public/app/task_runner.h"

namespace content {
class ProcessRunner;
class MapContentClient;
class MapView;
class MapContent : public base::RefCountedThreadSafe<MapContent> {
 public:
  struct Preferences {};

 public:
  virtual bool GetVersion(std::string* version) = 0;
  virtual bool Create(const base::WeakPtr<MapContentClient>& map_content_client,
                      const base::WeakPtr<TaskRunner>& task_runner,
                      const MapContent::Preferences& preferences,
                      ProcessRunner** process_runner, MapView** map_view) = 0;
  virtual bool Destroy() = 0;

  virtual bool GetMapView(MapView** map_view) = 0;

 protected:
  friend class base::RefCountedThreadSafe<MapContent>;
  virtual ~MapContent() {}
};

static const std::string kCreateMapContentFnName = "CreateMapContent";
}  // namespace content

extern "C" {
int32_t CONTENT_EXPORT CreateMapContent(content::MapContent** map_content);
int32_t CONTENT_EXPORT DestroyMapContent(content::MapContent** map_content);

using CreateMapContentFn = int32_t (*)(content::MapContent** map_content);
using DestroyMapContentFn = int32_t (*)(content::MapContent** map_content);
}

#endif  // CONTENT_PUBLIC_MAP_CONTENT_H
