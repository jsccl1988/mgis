// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "plugin/contribute.h"

#include <stddef.h>
#include <string.h>

namespace plugin {

bool Contribute(const char* command_id) {
  // In-process sample command. No DLL is loaded.
  if (command_id == NULL || command_id[0] == '\0')
    return false;
  return strcmp(command_id, "plugin.sample.run") == 0;
}

}  // namespace plugin
