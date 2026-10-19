// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "content/public/plugin_host.h"

namespace content {
namespace {

bool ContainsId(const std::vector<std::string>& ids, const std::string& id) {
  for (size_t i = 0; i < ids.size(); ++i) {
    if (ids[i] == id)
      return true;
  }
  return false;
}

void SetError(std::string* error, const std::string& message) {
  if (error)
    *error = message;
}

void ClearError(std::string* error) {
  if (error)
    error->clear();
}

}  // namespace

PluginHost::PluginHost() {}

PluginHost::~PluginHost() {}

bool PluginHost::contribute_command_id(const std::string& command_id) {
  if (command_id.empty() || ContainsId(command_ids_, command_id))
    return false;
  command_ids_.push_back(command_id);
  return true;
}

bool PluginHost::contribute_interaction_factory_id(
    const std::string& factory_id) {
  if (factory_id.empty() ||
      ContainsId(interaction_factory_ids_, factory_id))
    return false;
  interaction_factory_ids_.push_back(factory_id);
  return true;
}

bool PluginHost::run_processing(const std::string& algorithm_id,
                                std::string* error) {
  if (algorithm_id.empty()) {
    SetError(error, "algorithm id is empty");
    return false;
  }
  last_algorithm_id_ = algorithm_id;
  ClearError(error);
  return true;
}

bool PluginHost::has_command_id(const std::string& command_id) const {
  return ContainsId(command_ids_, command_id);
}

bool PluginHost::has_interaction_factory_id(
    const std::string& factory_id) const {
  return ContainsId(interaction_factory_ids_, factory_id);
}

const std::string& PluginHost::last_algorithm_id() const {
  return last_algorithm_id_;
}

}  // namespace content
