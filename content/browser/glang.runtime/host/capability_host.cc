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
      pointer_context_(NULL),
      attach_slot_(NULL),
      attach_context_(NULL),
      scene_mode_slot_(NULL),
      scene_mode_context_(NULL),
      expect_camera_slot_(NULL),
      expect_camera_context_(NULL) {}

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

void CapabilityHost::set_attach_slot(AttachSlot slot, void* context) {
  attach_slot_ = slot;
  attach_context_ = context;
}

void CapabilityHost::set_scene_mode_slot(SceneModeSlot slot, void* context) {
  scene_mode_slot_ = slot;
  scene_mode_context_ = context;
}

void CapabilityHost::set_expect_camera_slot(ExpectCameraSlot slot,
                                           void* context) {
  expect_camera_slot_ = slot;
  expect_camera_context_ = context;
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

bool CapabilityHost::has_attach_slot() const {
  return attach_slot_ != NULL;
}

bool CapabilityHost::has_scene_mode_slot() const {
  return scene_mode_slot_ != NULL;
}

bool CapabilityHost::has_expect_camera_slot() const {
  return expect_camera_slot_ != NULL;
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

bool CapabilityHost::RunAttach(const std::string& view_id) {
  if (!attach_slot_)
    return false;
  return attach_slot_(attach_context_, view_id);
}

bool CapabilityHost::RunSceneMode(const std::string& mode) {
  if (!scene_mode_slot_)
    return false;
  return scene_mode_slot_(scene_mode_context_, mode);
}

bool CapabilityHost::RunExpectCamera(const std::string& spec) {
  if (!expect_camera_slot_)
    return false;
  return expect_camera_slot_(expect_camera_context_, spec);
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
