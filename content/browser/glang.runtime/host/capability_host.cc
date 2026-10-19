// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "content/browser/glang.runtime/host/capability_host.h"

namespace content {

CapabilityHost::CapabilityHost()
    : mark_slot_(nullptr), mark_context_(nullptr) {}

CapabilityHost::~CapabilityHost() {}

void CapabilityHost::set_mark_slot(MarkSlot slot, void* context) {
  mark_slot_ = slot;
  mark_context_ = context;
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
