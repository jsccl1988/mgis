// Copyright (c) 2023 The MGIS Authors.
// All rights reserved.

#ifndef GUI_CONTENT_IMPL_MAP_CONTENT_MANAGER_H_
#define GUI_CONTENT_IMPL_MAP_CONTENT_MANAGER_H_

#include <map>

#include "base/memory/ref_counted.h"
#include "base/synchronization/lock.h"
#include "content/public/map_box/map_box.h"
#include "content/public/map_box/map_view.h"
#include "content/public/map_content.h"

namespace gui {
class MainRunner;
class MapContentManager {
 public:
  static MapContentManager* Instance();

 public:
  bool Init(MainRunner* main_runner, content::MapContent* map_content,
            const base::WeakPtr<content::TaskRunner>& task_runner);

  bool GetVersion(std::string* version);
  bool GetMapBox(content::MapBox** map_box);
  bool GetMapContentClient(content::MapContentClient** map_content_client);

  bool QuitMessageLoop();

 private:
  MapContentManager();
  ~MapContentManager();

 private:
  base::ScopedRefPtr<content::MapContent> map_content_;
  base::ScopedRefPtr<content::MapBox> map_box_;
  base::ScopedRefPtr<content::MapContentClient> map_content_client_;

  base::ScopedRefPtr<MainRunner> main_runner_;

  content::MapContent::Preferences map_content_preferences_;
  content::MapBox::Preferences map_box_preferences_;

  static base::Lock g_factory_lock;
};
}  // namespace gui
#endif  //  GUI_CONTENT_IMPL_MAP_CONTENT_MANAGER_H_
