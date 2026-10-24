// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "plugin/contribute.h"

#include <string>

#include "content/public/browser/plugin_host.h"

namespace plugin {

bool Contribute(content::PluginHost* host, const char* command_id) {
  if (!host || command_id == NULL || command_id[0] == '\0')
    return false;
  std::string error;
  if (!host->has_command_id(command_id))
    return false;
  return host->run_processing(command_id, &error);
}

}  // namespace plugin
