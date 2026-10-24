// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#ifndef PLUGIN_CONTRIBUTE_H_
#define PLUGIN_CONTRIBUTE_H_

namespace content {
class PluginHost;
}

namespace plugin {

// Runs |command_id| through |host|. Returns true when the host accepts it.
bool Contribute(content::PluginHost* host, const char* command_id);

}  // namespace plugin

#endif  // PLUGIN_CONTRIBUTE_H_
