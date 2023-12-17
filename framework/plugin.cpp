#include "framework/plugin.h"

#include <filesystem>

#include "base/logging.h"

namespace framework {
namespace fs = std::filesystem;
Plugin::Plugin(const char* name, const char* path)
    : DynamicLibrary(name, path) {
  get_plugin_version_ = nullptr;
  start_plugin_ = nullptr;
  stop_plugin_ = nullptr;
}

Plugin::~Plugin(void) {}

bool Plugin::Load() {
  LOG(INFO) << __FUNCTION__ << "loading plugin" << name_;

  int path_length = path_.length() + MAX_PATH;
  char* path = new char[path_length];
  sprintf_s(path, path_length, "%s%s", path_.c_str(), name_.c_str());
  // dll_ = ::LoadLibrary(path);
  if (!dll_) {
    std::string err = "   Loading ";
    err += name_;
    err += " error!";
    LOG(ERROR) << __FUNCTION__ << err;
    ::MessageBox(nullptr, err.c_str(), "Gis- error", MB_OK | MB_ICONERROR);

    return false;
  }

  SAFE_DELETE_A(path);

  get_plugin_version_ = reinterpret_cast<fnGetPluginVersion*>(
      ::GetProcAddress(dll_, "GetPluginVersion"));
  start_plugin_ =
      reinterpret_cast<fnStartPlugin*>(::GetProcAddress(dll_, "StartPlugin"));
  stop_plugin_ =
      reinterpret_cast<fnStopPlugin*>(::GetProcAddress(dll_, "StopPlugin"));

  if (!get_plugin_version_ || !start_plugin_ || !stop_plugin_) {
    ::FreeLibrary(dll_);
    std::string err = "   ";
    err += name_;
    err += " is an invalid plugin,so not load it!";
    LOG(ERROR) << __FUNCTION__ << err;
    ::MessageBox(nullptr, err.c_str(), "Gis - error", MB_OK | MB_ICONERROR);
    return false;
  }

  return true;
}

void Plugin::UnLoad() {
  if (dll_ != nullptr) {
    LOG(INFO) << __FUNCTION__ << "unloading plugin" << name_;
    FreeLibrary(dll_);
    dll_ = nullptr;
  }
}

PluginManager* PluginManager::singleton_ = nullptr;
PluginManager::PluginManager(void) {}
PluginManager::~PluginManager(void) { UnLoadAllPlugin(); }
PluginManager* PluginManager::GetSingletonPtr(void) {
  if (singleton_ == nullptr) {
    singleton_ = new PluginManager();
  }
  return singleton_;
}

void PluginManager::DestoryInstance(void) { SAFE_DELETE(singleton_); }

void PluginManager::LoadAllPlugin(const char* dir) {
  // get all plugins in file
  for (auto dir_iter : fs::directory_iterator(dir)) {
    LoadPlugin(dir_iter.path(), dir_iter.path());
    ++i;
  }

  StartAllPlugin();
}

void PluginManager::UnLoadAllPlugin() {
  StopAllPlugin();

  // Unload & delete resources in turn
  for (Plugins::iterator it = plugins_.begin(); it != plugins_.end(); ++it) {
    (*it)->UnLoad();
    SAFE_DELETE(*it);
  }

  // Empty the list
  plugins_.clear();
}

Plugin* PluginManager::LoadPlugin(const char* name, const char* path) {
  Plugin* pPlugin = nullptr;

  Plugins::iterator i = plugins_.begin();
  while (i != plugins_.end()) {
    if (strcmp((*i)->GetName(), name) == 0) {
      pPlugin = *i;
      return pPlugin;
    }
    ++i;
  }

  pPlugin = new Plugin(name, path);
  if (pPlugin->Load())
    plugins_.push_back(pPlugin);
  else
    SAFE_DELETE(pPlugin);

  return pPlugin;
}

void PluginManager::UnLoadPlugin(const char* name) {
  Plugin* pPlugin = nullptr;

  Plugins::iterator i = plugins_.begin();
  while (i != plugins_.end()) {
    if (strcmp((*i)->GetName(), name) == 0) {
      pPlugin = *i;
      pPlugin->UnLoad();
      SAFE_DELETE(pPlugin);
      plugins_.erase(i);
      break;
    }
    ++i;
  }
}

void PluginManager::StartAllPlugin() {
  for (Plugins::iterator it = plugins_.begin(); it != plugins_.end(); ++it) {
    (*it)->StartPlugin();
  }
}

void PluginManager::StopAllPlugin() {
  for (Plugins::iterator it = plugins_.begin(); it != plugins_.end(); ++it) {
    (*it)->StopPlugin();
  }
}
}  // namespace framework