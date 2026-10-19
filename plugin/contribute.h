// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#ifndef PLUGIN_CONTRIBUTE_H_
#define PLUGIN_CONTRIBUTE_H_

namespace plugin {

// Narrow command entry used until a PluginHost is linked into this process.
// |command_id| is a UTF-8 command id. Returns true when that command ran.
bool Contribute(const char* command_id);

}  // namespace plugin

#endif  // PLUGIN_CONTRIBUTE_H_
