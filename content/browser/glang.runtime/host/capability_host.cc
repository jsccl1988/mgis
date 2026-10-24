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
      scene_mode_slot_(NULL),
      scene_mode_context_(NULL),
      camera_slot_(NULL),
      camera_context_(NULL),
      attach_slot_(NULL),
      attach_context_(NULL),
      layer_slot_(NULL),
      layer_context_(NULL),
      entity_slot_(NULL),
      entity_context_(NULL) {}

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

void CapabilityHost::set_scene_mode_slot(SceneModeSlot slot, void* context) {
  scene_mode_slot_ = slot;
  scene_mode_context_ = context;
}

void CapabilityHost::set_camera_slot(ExpectCameraSlot slot, void* context) {
  camera_slot_ = slot;
  camera_context_ = context;
}

void CapabilityHost::set_attach_slot(AttachSlot slot, void* context) {
  attach_slot_ = slot;
  attach_context_ = context;
}

void CapabilityHost::set_layer_slot(ExpectIdSlot slot, void* context) {
  layer_slot_ = slot;
  layer_context_ = context;
}

void CapabilityHost::set_entity_slot(ExpectIdSlot slot, void* context) {
  entity_slot_ = slot;
  entity_context_ = context;
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

bool CapabilityHost::has_scene_mode_slot() const {
  return scene_mode_slot_ != NULL;
}

bool CapabilityHost::has_camera_slot() const {
  return camera_slot_ != NULL;
}

bool CapabilityHost::has_attach_slot() const {
  return attach_slot_ != NULL;
}

bool CapabilityHost::has_layer_slot() const {
  return layer_slot_ != NULL;
}

bool CapabilityHost::has_entity_slot() const {
  return entity_slot_ != NULL;
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

bool CapabilityHost::RunSceneMode(const std::string& mode) {
  if (!scene_mode_slot_)
    return false;
  return scene_mode_slot_(scene_mode_context_, mode);
}

bool CapabilityHost::RunExpectCamera(const std::string& view_id,
                                     bool has_target,
                                     double lon,
                                     double lat,
                                     double height,
                                     std::string* error) {
  if (!camera_slot_)
    return false;
  return camera_slot_(camera_context_, view_id, has_target, lon, lat, height,
                      error);
}

bool CapabilityHost::RunAttach(const std::string& view_id, std::string* error) {
  if (!attach_slot_)
    return false;
  return attach_slot_(attach_context_, view_id, error);
}

bool CapabilityHost::RunExpectLayer(const std::string& id, std::string* error) {
  if (!layer_slot_)
    return false;
  return layer_slot_(layer_context_, id, error);
}

bool CapabilityHost::RunExpectEntity(const std::string& id,
                                     std::string* error) {
  if (!entity_slot_)
    return false;
  return entity_slot_(entity_context_, id, error);
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
