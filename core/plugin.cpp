#include "core/plugin.h"

#include "base/logging.h"

namespace core {
Plugin::Plugin(const char* name, const char* path) : DynLib(name, path) {
  get_plugin_version_ = nullptr;
  start_plugin_ = nullptr;
  stop_plugin_ = nullptr;
}

Plugin::~Plugin(void) {}

bool Plugin::Load() {
  LOG(INFO) << __FUNCTION__ << "loading plugin" << name_;

  int nPathLen = strlen(path_) + MAX_PATH;
  char* pChPath = new char[nPathLen];
  sprintf_s(pChPath, nPathLen, "%s%s", path_, name_);
  dll_ = ::LoadLibrary(pChPath);
  if (!dll_) {
    std::string strErr = "   Loading ";
    strErr += name_;
    strErr += " error!";
    LOG(ERROR) << __FUNCTION__ << strErr;
    ::MessageBox(nullptr, strErr.c_str(), "Gis- error", MB_OK | MB_ICONERROR);

    return false;
  }

  SAFE_DELETE_A(pChPath);

  get_plugin_version_ = reinterpret_cast<fnGetPluginVersion*>(
      ::GetProcAddress(dll_, "GetPluginVersion"));
  start_plugin_ =
      reinterpret_cast<fnStartPlugin*>(::GetProcAddress(dll_, "StartPlugin"));
  stop_plugin_ =
      reinterpret_cast<fnStopPlugin*>(::GetProcAddress(dll_, "StopPlugin"));

  if (!get_plugin_version_ || !start_plugin_ || !stop_plugin_) {
    ::FreeLibrary(dll_);
    std::string strErr = "   ";
    strErr += name_;
    strErr += " is an invalid plugin,so not load it!";
    LOG(ERROR) << __FUNCTION__ << strErr;
    ::MessageBox(nullptr, strErr.c_str(), "Gis - error", MB_OK | MB_ICONERROR);
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

extern const string g_strPluginLibLog;
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

void PluginManager::LoadAllPlugin(const char* filePath) {
  // get all plugins in file
  vFileInfos vFileInfos;
  FileSystem fileSys;
  fileSys.SetCurrentDir(filePath);
  fileSys.SearchCurrentDir("*.am", false);
  vFileInfos = fileSys.GetFileInfos();

  // load plugins
  vFileInfos::iterator i = vFileInfos.begin();

  while (i != vFileInfos.end()) {
    LoadPlugin((*i).szName, (*i).szPath);
    ++i;
  }

  StartAllPlugin();
}

void PluginManager::UnLoadAllPlugin() {
  StopAllPlugin();

  // Unload & delete resources in turn
  for (vPluginPtrs::iterator it = plugins_.begin(); it != plugins_.end();
       ++it) {
    (*it)->UnLoad();
    SAFE_DELETE(*it);
  }

  // Empty the list
  plugins_.clear();
}

Plugin* PluginManager::LoadPlugin(const char* name, const char* path) {
  Plugin* pPlugin = nullptr;

  vPluginPtrs::iterator i = plugins_.begin();
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

  vPluginPtrs::iterator i = plugins_.begin();
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
  for (vPluginPtrs::iterator it = plugins_.begin(); it != plugins_.end();
       ++it) {
    (*it)->StartPlugin();
  }
}

void PluginManager::StopAllPlugin() {
  for (vPluginPtrs::iterator it = plugins_.begin(); it != plugins_.end();
       ++it) {
    (*it)->StopPlugin();
  }
}
}  // namespace core