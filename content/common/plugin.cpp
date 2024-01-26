// Copyright (c) 2024 The MGIS Authors.
// All rights reserved.

#include "content/common/plugin.h"

#include <filesystem>
#include <memory>

#include "base/logging.h"

namespace content {
Plugin::Plugin(const base::NameChar* name, const base::PathChar* path)
    : Library(name, path),
      get_version_(nullptr),
      start_(nullptr),
      stop_(nullptr) {}

Plugin::~Plugin(void) {}

bool Plugin::Load() {
  if (!Library::Load()) {
    return false;
  }

  get_version_ =
      reinterpret_cast<fnGetVersion*>(::GetProcAddress(dll_, "GetVersion"));
  start_ = reinterpret_cast<fnStart*>(::GetProcAddress(dll_, "Star"));
  stop_ = reinterpret_cast<fnStop*>(::GetProcAddress(dll_, "Stop"));

  if (!get_version_ || !start_ || !stop_) {
    Unload();
    return false;
  }

  return true;
}

bool Plugin::Unload() { return Library::Unload(); }

void PluginManager::LoadAllPlugin(const base::PathChar* dir) {
  // using fs = std::filesystem;
  // // get all plugins in file
  // for (auto dir_iter : fs::directory_iterator(dir)) {
  //   LoadPlugin(dir_iter.path(), dir_iter.path());
  // }

  StartAllPlugin();
}

void PluginManager::UnLoadAllPlugin() {
  StopAllPlugin();

  for (Plugins::iterator it = plugins_.begin(); it != plugins_.end(); ++it) {
    (*it)->Unload();
    SAFE_DELETE(*it);
  }

  plugins_.clear();
}

Plugin* PluginManager::LoadPlugin(const base::NameChar* name,
                                  const base::PathChar* path) {
  Plugin* plugin = nullptr;
  Plugins::iterator i = plugins_.begin();
  while (i != plugins_.end()) {
    if (wcscmp((*i)->GetName(), name) == 0) {
      plugin = *i;
      return plugin;
    }
    ++i;
  }

  plugin = new Plugin(name, path);
  if (plugin->Load()) {
    plugins_.push_back(plugin);
  } else {
    SAFE_DELETE(plugin);
  }

  return plugin;
}

void PluginManager::UnloadPlugin(const base::NameChar* name) {
  Plugin* plugin = nullptr;

  Plugins::iterator i = plugins_.begin();
  while (i != plugins_.end()) {
    if (wcscmp((*i)->GetName(), name) == 0) {
      plugin = *i;
      plugin->Unload();
      SAFE_DELETE(plugin);
      plugins_.erase(i);
      break;
    }
    ++i;
  }
}

void PluginManager::StartAllPlugin() {
  for (Plugins::iterator it = plugins_.begin(); it != plugins_.end(); ++it) {
    (*it)->Start();
  }
}

void PluginManager::StopAllPlugin() {
  for (Plugins::iterator it = plugins_.begin(); it != plugins_.end(); ++it) {
    (*it)->Stop();
  }
}
}  // namespace content