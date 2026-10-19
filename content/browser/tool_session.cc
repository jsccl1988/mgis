// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "content/public/tool_session.h"

namespace content {

ToolSession::ToolSession() : has_last_input_(false) {}

ToolSession::~ToolSession() {}

bool ToolSession::execute(const std::string& command_id,
                          const std::string& view_id) {
  if (command_id.empty() || !is_known_view_id(view_id))
    return false;
  last_command_id_ = command_id;
  last_command_view_id_ = view_id;
  return true;
}

bool ToolSession::dispatch_input(const InputEvent& event) {
  last_input_ = event;
  has_last_input_ = true;
  return true;
}

bool ToolSession::activate(const std::string& interaction_id,
                           const std::string& view_id) {
  if (interaction_id.empty() || !is_known_view_id(view_id))
    return false;
  last_interaction_id_ = interaction_id;
  last_interaction_view_id_ = view_id;
  return true;
}

const std::string& ToolSession::last_command_id() const {
  return last_command_id_;
}

const std::string& ToolSession::last_command_view_id() const {
  return last_command_view_id_;
}

bool ToolSession::has_last_input() const {
  return has_last_input_;
}

const InputEvent& ToolSession::last_input() const {
  return last_input_;
}

const std::string& ToolSession::last_interaction_id() const {
  return last_interaction_id_;
}

const std::string& ToolSession::last_interaction_view_id() const {
  return last_interaction_view_id_;
}

}  // namespace content
