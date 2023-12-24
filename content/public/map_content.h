// Copyright (c) 2023 The MGIS Authors.
// All rights reserved.

#ifndef CONTENT_PUBLIC_MAP_CONTENT_H
#define CONTENT_PUBLIC_MAP_CONTENT_H

#include "base/memory/ref_counted.h"
#include "base/memory/weak_ptr.h"
#include "content/app/task_runner.h"

namespace content {
class MapContentClient;
class MapBox;
class MapContent : public base::RefCountedThreadSafe<MapContent> {
 public:
  struct Options {};

 public:
  virtual bool GetVersion(base::stringx* version) = 0;
  virtual bool Create(const base::WeakPtr<MapContentClient>& web_render_client,
                      const base::WeakPtr<TaskRunner>& task_runner,
                      const MapContent::Options& options,
                      MapBox** web_browser) = 0;
  virtual bool Destroy() = 0;

 protected:
  friend class base::RefCountedThreadSafe<MapContent>;
  virtual ~MapContent() {}
};

static const char* CreateMapContentFnName = "CreateMapContent";
typedef int32 (*CreateMapContentFnPtr)(content::MapContent** map_content);
}  // namespace content

#endif  // CONTENT_PUBLIC_MAP_CONTENT_H
