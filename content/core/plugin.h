#ifndef CONTENT_CORE_PLUGIN_H
#define CONTENT_CORE_PLUGIN_H

#include <string>
#include <vector>

#include "base/memory/singleton.h"
#include "base/synchronization/lock.h"
#include "content/content.h"
#include "content/content_export.h"
#include "content/core/dynamic_library.h"


namespace content {
class CONTENT_EXPORT Plugin : public DynamicLibrary {
 public:
  Plugin(const char* name, const char* path);
  virtual ~Plugin(void);

 public:
  bool Load(void) override;
  bool Unload(void) override;

  inline int GetVersion(void) { return get_version_(); }
  inline void Start(void) { return start_(); }
  inline void Stop(void) { return stop_(); }

 protected:
  using fnGetVersion = int();
  using fnStart = void();
  using fnStop = void();

 protected:
  fnGetVersion* get_version_;
  fnStart* start_;
  fnStop* stop_;
};

using Plugins = std::vector<Plugin*>;
class CONTENT_EXPORT PluginManager {
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
  void UnloadPlugin(const char* name);

 protected:
  Plugins plugins_;

 private:
  PluginManager(void);

 private:
  static PluginManager* singleton_;
};
}  // namespace content
#endif  // CONTENT_CORE_PLUGIN_H