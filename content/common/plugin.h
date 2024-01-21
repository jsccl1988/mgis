// Copyright (c) 2024 The MGIS Authors.
// All rights reserved.

#ifndef CONTENT_COMMON_PLUGIN_H
#define CONTENT_COMMON_PLUGIN_H

#include <string>
#include <vector>

#include "base/memory/singleton.h"
#include "base/synchronization/lock.h"
#include "content/content.h"
#include "content/content_export.h"
#include "content/common/library.h"


namespace content {
class Plugin : public Library {
 public:
  Plugin(const base::NameChar* name, const base::PathChar* path);
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
class PluginManager {
 public:
  virtual ~PluginManager(void);

 public:
  static PluginManager* GetSingletonPtr(void);
  static void DestoryInstance(void);

 public:
  void LoadAllPlugin(const base::PathChar* filePath);
  void UnLoadAllPlugin(void);

 protected:
  void StartAllPlugin(void);
  void StopAllPlugin(void);
  Plugin* LoadPlugin(const base::NameChar* name, const base::PathChar* path);
  void UnloadPlugin(const base::NameChar* name);

 protected:
  Plugins plugins_;

 private:
  PluginManager(void);

 private:
  static PluginManager* singleton_;
};
}  // namespace content
#endif  // CONTENT_CORE_PLUGIN_H