// Copyright (c) 2023 The MGIS Authors.
// All rights reserved.

#include "gui/content_impl/manager/map_content_manager.h"

#include <map>

#include "base/path/base_paths.h"
#include "content/public/map_content.h"
#include "content/public/map_view.h"
#include "gui/content_impl/client/map_content_client_impl.h"
#include "gui/content_impl/factory/map_content_factory.h"
#include "gui/main_runner.h"

#define AUTO_GENERATE_DUMP

namespace gui {
// static
gui::Lock MapContentManager::g_lock;
MapContentManager* MapContentManager::Instance() {
  static MapContentManager* s_instance = nullptr;
  if (s_instance == nullptr) {
    gui::AutoLock lock(&g_lock);
    if (s_instance == nullptr) {
      s_instance = new MapContentManager();
    }
  }

  return s_instance;
}

MapContentManager::MapContentManager() {}
MapContentManager::~MapContentManager() {}

bool MapContentManager::Init(
    const base::WeakPtr<content::TaskRunner>& task_runner) {
  if (!task_runner) {
    return false;
  }

  content::MapContent* map_content = nullptr;
  MapContentFactory factory;
  if (factory.CreateMapContent(&map_content) && map_content) {
    map_content_ = map_content;
    map_content_client_ = new MapContentClientImpl();

    content::MapView* map_view = nullptr;
    content::ProcessRunner* process_runner = nullptr;
    if (map_content_->Create(map_content_client_->AsWeakPtr(), task_runner,
                             map_content_preferences_, &process_runner,
                             &map_view)) {
      process_runner_ = process_runner;
      if (process_runner_) {
        process_runner_->Init();

        content::MapView::CreateParams create_params;
        create_params.parent_hwnd = nullptr;
        map_view->Create(create_params, map_view_preferences_);
        return true;
      }
    }
  }

  return false;
}

bool MapContentManager::Run() {
  if (process_runner_) {
    return process_runner_->Run();
  }

  return false;
}

bool MapContentManager::Destroy() {
  if (process_runner_) {
    return process_runner_->Stop();
  }

  if (map_content_) {
    map_content_->Destroy();
  }

  map_content_client_ = nullptr;
  map_content_ = nullptr;
  process_runner_ = nullptr;

  return true;
}
}  // namespace gui
