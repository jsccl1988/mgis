// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "content/browser/glang.runtime/host/capability_host.h"

namespace content {

CapabilityHost::CapabilityHost()
    : mark_slot_(NULL),
      mark_context_(NULL),
      command_slot_(NULL),
      command_context_(NULL),
      activate_slot_(NULL),
      activate_context_(NULL),
      pointer_slot_(NULL),
      pointer_context_(NULL) {}

CapabilityHost::~CapabilityHost() {}

void CapabilityHost::set_mark_slot(MarkSlot slot, void* context) {
  mark_slot_ = slot;
  mark_context_ = context;
}

void CapabilityHost::set_command_slot(CommandSlot slot, void* context) {
  command_slot_ = slot;
  command_context_ = context;
}

void CapabilityHost::set_activate_slot(CommandSlot slot, void* context) {
  activate_slot_ = slot;
  activate_context_ = context;
}

void CapabilityHost::set_pointer_slot(PointerSlot slot, void* context) {
  pointer_slot_ = slot;
  pointer_context_ = context;
}

bool CapabilityHost::has_command_slot() const {
  return command_slot_ != NULL;
}

bool CapabilityHost::has_activate_slot() const {
  return activate_slot_ != NULL;
}

bool CapabilityHost::has_pointer_slot() const {
  return pointer_slot_ != NULL;
}

bool CapabilityHost::RunCommand(const std::string& command_id,
                                const std::string& view_id) {
  if (!command_slot_)
    return false;
  return command_slot_(command_context_, command_id, view_id);
}

bool CapabilityHost::RunActivate(const std::string& command_id,
                                 const std::string& view_id) {
  if (!activate_slot_)
    return false;
  return activate_slot_(activate_context_, command_id, view_id);
}

bool CapabilityHost::RunPointer(const InputEvent& event) {
  if (!pointer_slot_)
    return false;
  return pointer_slot_(pointer_context_, event);
}

void CapabilityHost::mark(const std::string& name) {
  if (name.empty())
    return;
  marks_.push_back(name);
  if (mark_slot_)
    mark_slot_(mark_context_, name);
}

bool CapabilityHost::has_mark(const std::string& name) const {
  for (size_t i = 0; i < marks_.size(); ++i) {
    if (marks_[i] == name)
      return true;
  }
  return false;
}

const std::vector<std::string>& CapabilityHost::marks() const {
  return marks_;
}

}  // namespace content
