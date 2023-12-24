// Copyright (c) 2023 The MGIS Authors.
// All rights reserved.

#include "gui/content/map_content_manager.h"

#include <map>

#include "browser/map_box.h"
#include "content.h"

#include "gui/base/path/base_paths.h"
#include "gui/main_runner.h"
#include "gui/content/delegate/map_content_client_impl.h"

#define AUTO_GENERATE_DUMP

namespace gui {
// static
base::Lock MapContentManager::g_factory_lock;
MapContentManager* MapContentManager::Instance() {
  static MapContentManager* s_instance = NULL;
  if (s_instance == NULL) {
    base::AutoLock lock(g_factory_lock);
    if (s_instance == NULL) {
      s_instance = new MapContentManager();
    }
  }

  return s_instance;
}

MapContentManager::MapContentManager() {}

MapContentManager::~MapContentManager() {}

bool MapContentManager::Init(MainRunner * main_runner
  , content::MapContent * content
  , const base::WeakPtr<content::TaskRunner>& task_runner) {
  if (!main_runner || !content) {
    return false;
  }

  string16 path;
  PathProvider(DIR_APP_DATA_HBDEMO, &path);

  map_content_ = content;
  main_runner_ = main_runner;

  map_content_setting_.customer_id = "gui";
  map_content_setting_.humming_dir = path;
  map_content_setting_.cache_dir = path + L"\\user_cache";
  map_content_setting_.force_cold_start = true;

#ifdef AUTO_GENERATE_DUMP
  map_content_setting_.crashcatcher_setting.auto_dump_dir_ = path + L"\\dump";
  map_content_setting_.crashcatcher_setting.generate_dump_ = true;
#else
  map_content_setting_.crashcatcher_setting.generate_dump_ = false;
#endif  // AUTO_GENERATE_DUMP

  map_content_setting_.crashcatcher_setting.on_crashed_callback_func_ =
    &CrashHandler;

  // create web render
  map_box_preferences_.enable_chrome = true;
  map_box_preferences_.enable_trident = true;
  map_box_preferences_.chrome_delay_time = 0;
  map_box_preferences_.trident_delay_time = 0;

  map_box_preferences_.enable_direct_write = false;
  map_box_preferences_.enable_high_dpi = true;
  map_box_preferences_.enable_gpu = true;
  map_box_preferences_.enable_override_download = false;
  map_box_preferences_.enable_override_url_fetch = false;
  map_box_preferences_.enable_geolocation = true;
  map_box_preferences_.enable_override_geolocation = false;
  map_box_preferences_.default_session_path = path + L"\\session\\default";

  map_box_preferences_.user_agent = L"Hummingbird/1.2";

  map_content_client_ = new MapContentClientImpl;
  content::MapBox* map_box = NULL;
  if (map_content_->Create(map_content_client_->AsWeakPtr()
    , task_runner
    , map_content_setting_
    , &map_box, NULL)) {
    map_box->Create(map_box_preferences_);
    map_box_ = map_box;
    return true;
  }
  return true;
}

bool MapContentManager::GetVersion(string16 * version) {
  if (map_content_) {
    std::string ver;
    map_content_->GetVersion(&ver);
    *version = ver.ToString16();
    return true;
  }
  return false;
}

bool MapContentManager::GetMapBox(content::MapBox ** map_box) {
  if (!map_box)
    return false;

  *map_box = map_box_.get();

  return 0;
}

bool MapContentManager::GetMapContentClient(
  content::MapContentClient ** map_content_client) {
  if (!map_content_client)
    return false;

  *map_content_client = map_content_client_.get();

  return 0;
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

  map_content_client_ = NULL;
  map_box_ = NULL;
  map_content_ = NULL;

  main_runner_ = NULL;

  return true;
}

}  // namespace gui
