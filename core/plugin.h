#ifndef GIS_CORE_PLUGIN_H
#define GIS_CORE_PLUGIN_H

#include <string>

#include "core/core.h"
#include "core/core_export.h"
#include "core/dynamic_library.h"


namespace core {
class GIS_CORE_EXPORT Plugin : public DynamicLibrary {
 public:
  Plugin(const char* name, const char* path);
  virtual ~Plugin(void);

 public:
  inline int GetPluginVersion(void) { return get_plugin_version_(); }

  inline void StartPlugin(void) { return start_plugin_(); }
  inline void StopPlugin(void) { return stop_plugin_(); }

  bool Load(void);
  void UnLoad(void);

 protected:
  using fnGetPluginVersion = int();
  using fnStartPlugin = void();
  using fnStopPlugin = void();

 protected:
  fnGetPluginVersion* get_plugin_version_;
  fnStartPlugin* start_plugin_;
  fnStopPlugin* stop_plugin_;
};

typedef std::vector<Plugin*> vPluginPtrs;
class GIS_CORE_EXPORT PluginManager {
 public:
  virtual ~PluginManager(void);

 public:
  static PluginManager* GetSingletonPtr(void);
  static void DestoryInstance(void);

 public:
  void LoadAllPlugin(const char* filePath);
  void UnLoadAllPlugin(void);

 protected:
  void StartAllPlugin(void);
  void StopAllPlugin(void);
  Plugin* LoadPlugin(const char* name, const char* path);
  void UnLoadPlugin(const char* name);

 protected:
  vPluginPtrs plugins_;

 private:
  PluginManager(void);

 private:
  static PluginManager* singleton_;
};
}  // namespace core
#endif  // GIS_CORE_PLUGIN_H