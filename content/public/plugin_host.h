// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#ifndef CONTENT_PUBLIC_PLUGIN_HOST_H_
#define CONTENT_PUBLIC_PLUGIN_HOST_H_

#include <string>
#include <vector>

namespace content {

// In-process plugin surface. Plugins contribute ids and run algorithms here.
// This type has no pipe client, no MapLibre map, and no UE types.
class PluginHost {
 public:
  PluginHost();
  ~PluginHost();

  // Records a command id. Returns false when |command_id| is empty or already
  // contributed. Does not install a tool implementation.
  bool contribute_command_id(const std::string& command_id);

  // Records an interaction-factory id. Returns false when |factory_id| is
  // empty or already contributed. The factory object lives in tool/ later.
  bool contribute_interaction_factory_id(const std::string& factory_id);

  // Algorithm entry. P0 records |algorithm_id| and does not run a processor.
  // |error| may be NULL. Returns false when |algorithm_id| is empty.
  bool run_processing(const std::string& algorithm_id, std::string* error);

  bool has_command_id(const std::string& command_id) const;
  bool has_interaction_factory_id(const std::string& factory_id) const;
  const std::string& last_algorithm_id() const;

 private:
  PluginHost(const PluginHost&) = delete;
  PluginHost& operator=(const PluginHost&) = delete;

  std::vector<std::string> command_ids_;
  std::vector<std::string> interaction_factory_ids_;
  std::string last_algorithm_id_;
};

}  // namespace content

#endif  // CONTENT_PUBLIC_PLUGIN_HOST_H_
