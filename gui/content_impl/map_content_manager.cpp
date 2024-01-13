// Copyright (c) 2023 The MGIS Authors.
// All rights reserved.

#include "gui/content_impl/map_content_manager.h"

#include <map>

#include "base/path/base_paths.h"
#include "content/public/map_box/map_box.h"
#include "content/public/map_content.h"
#include "gui/content_impl/map_content_client_impl.h"
#include "gui/main_runner.h"

#define AUTO_GENERATE_DUMP

namespace gui {
// static
base::Lock MapContentManager::g_factory_lock;
MapContentManager* MapContentManager::Instance() {
  static MapContentManager* s_instance = nullptr;
  if (s_instance == nullptr) {
    base::AutoLock lock(&g_factory_lock);
    if (s_instance == nullptr) {
      s_instance = new MapContentManager();
    }
  }

  return s_instance;
}

MapContentManager::MapContentManager() {}
MapContentManager::~MapContentManager() {}

bool MapContentManager::Init(
    MainRunner* main_runner, content::MapContent* map_content,
    const base::WeakPtr<content::TaskRunner>& task_runner) {
  if (!main_runner || !map_content) {
    return false;
  }

  map_content_ = map_content;
  main_runner_ = main_runner;

  map_content_client_ = new MapContentClientImpl();
  content::MapBox* map_box = nullptr;
  if (map_content_->Create(map_content_client_->AsWeakPtr(), task_runner,
                           map_content_preferences_, &map_box)) {
    map_box->Create(map_box_preferences_);
    map_box_ = map_box;
    return true;
  }
  return true;
}

bool MapContentManager::GetVersion(std::string* version) {
  if (map_content_) {
    map_content_->GetVersion(version);
    return true;
  }

  return false;
}

bool MapContentManager::GetMapBox(content::MapBox** map_box) {
  if (!map_box) {
    return false;
  }

  *map_box = map_box_.get();

  return true;
}

bool MapContentManager::GetMapContentClient(
    content::MapContentClient** map_content_client) {
  if (!map_content_client) {
    return false;
  }

  *map_content_client = map_content_client_.get();

  return true;
}

bool MapContentManager::QuitMessageLoop() {
  if (main_runner_) {
    main_runner_->Stop();
  }

  if (map_box_) {
    map_box_->Destroy();
  }

  if (map_content_) {
    map_content_->Destroy();
  }

  map_content_client_ = nullptr;
  map_box_ = nullptr;
  map_content_ = nullptr;

  main_runner_ = nullptr;

  return true;
}
}  // namespace gui
