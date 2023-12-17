#ifndef FRAMEWORK_EXPORT_PLUGIN_H
#define FRAMEWORK_EXPORT_PLUGIN_H

#include <string>

#include "framework/framework.h"
#include "framework/core_export.h"
#include "framework/dynamic_library.h"

namespace framework {
class FRAMEWORK_EXPORT_EXPORT Plugin : public DynamicLibrary {
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

typedef std::vector<Plugin*> Plugins;
class FRAMEWORK_EXPORT_EXPORT PluginManager {
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
  Plugins plugins_;

 private:
  PluginManager(void);

 private:
  static PluginManager* singleton_;
};
}  // namespace framework
#endif  // FRAMEWORK_EXPORT_PLUGIN_H