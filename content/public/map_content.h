// Copyright (c) 2023 The MGIS Authors.
// All rights reserved.

#ifndef CONTENT_PUBLIC_MAP_CONTENT_H
#define CONTENT_PUBLIC_MAP_CONTENT_H

#include "base/memory/ref_counted.h"
#include "base/memory/weak_ptr.h"
#include "content/public/app/task_runner.h"

namespace content {
class MapContentClient;
class MapBox;
class MapContent : public base::RefCountedThreadSafe<MapContent> {
 public:
  struct Preferences {};

 public:
  virtual bool GetVersion(std::string* version) = 0;
  virtual bool Create(const base::WeakPtr<MapContentClient>& map_content_client,
                      const base::WeakPtr<TaskRunner>& task_runner,
                      const MapContent::Preferences& preferences,
                      MapBox** map_box) = 0;
  virtual bool Destroy() = 0;

 protected:
  friend class base::RefCountedThreadSafe<MapContent>;
  virtual ~MapContent() {}
};

static const char* CreateMapContentFnName = "CreateMapContent";
typedef int32_t (*CreateMapContentFnPtr)(content::MapContent** map_content);
}  // namespace content

#endif  // CONTENT_PUBLIC_MAP_CONTENT_H
