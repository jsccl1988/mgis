// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#ifndef PLUGIN_LOADER_H_
#define PLUGIN_LOADER_H_

#include <string>
#include <vector>

#include "plugin/registry.h"

namespace content {
class PluginHost;
class TraceLog;
}

namespace plugin {

// Loads plugin DLLs named in Manifest::dll and calls mview_plugin_contribute.
// Does not live inside PluginHost; the shell owns one Loader beside GisContents.
class Loader {
 public:
  Loader();
  ~Loader();

  // Scans |search_dirs| for each manifest DLL after the manifest directory.
  // Skips empty dll, missing files, version mismatches, and contribute failures.
  // Returns the number of DLLs that contributed successfully.
  int LoadAll(const Registry& registry,
              content::PluginHost* host,
              const std::vector<std::string>& search_dirs,
              content::TraceLog* trace);

  // Loads one manifest. |dll_path_utf8| is the full path to the DLL.
  bool LoadOne(const Manifest& manifest,
               const std::string& dll_path_utf8,
               content::PluginHost* host,
               content::TraceLog* trace);

 private:
  Loader(const Loader&);
  Loader& operator=(const Loader&);

  std::vector<void*> modules_;
};

}  // namespace plugin

#endif  // PLUGIN_LOADER_H_
