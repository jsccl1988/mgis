// Copyright (c) 2023 The MGIS Authors.
// All rights reserved.

#ifndef GUI_CONTENT_IMPL_MANAGER_MAP_CONTENT_MANAGER_H_
#define GUI_CONTENT_IMPL_MANAGER_MAP_CONTENT_MANAGER_H_

#include <map>

#include "base/memory/ref_counted.h"
#include "content/public/app/process_runner.h"
#include "content/public/map_content.h"
#include "content/public/map_view.h"
#include "gui/base/lock.h"

namespace gui {
class MainRunner;
class MapContentManager {
 public:
  static MapContentManager* Instance();

 public:
  bool Init(const base::WeakPtr<content::TaskRunner>& task_runner);
  bool Run();
  bool Destroy();

  inline bool GetMapContent(content::MapContent** map_content) {
    if (!map_content) {
      return false;
    }

    *map_content = map_content_.get();

    return true;
  }
  inline bool GetMapContentClient(
      content::MapContentClient** map_content_client) {
    if (!map_content_client) {
      return false;
    }

    *map_content_client = map_content_client_.get();

    return true;
  }

 private:
  MapContentManager();
  ~MapContentManager();

 private:
  base::ScopedRefPtr<content::MapContent> map_content_;
  base::ScopedRefPtr<content::MapContentClient> map_content_client_;
  base::ScopedRefPtr<content::ProcessRunner> process_runner_;

  content::MapContent::Preferences map_content_preferences_;
  content::MapView::Preferences map_view_preferences_;

  static gui::Lock g_lock;
};
}  // namespace gui
#endif  //  GUI_CONTENT_IMPL_MANAGER_MAP_CONTENT_MANAGER_H_
