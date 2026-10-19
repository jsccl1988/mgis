// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "tool/interaction.h"

namespace tool {
namespace {

bool IsView3dTool(const std::string& id) {
  return id == "view3d.trackball" || id == "view3d.fps";
}

bool IsVolumeView(const std::string& view_id) {
  return view_id == "view0" || view_id == "view1";
}

}  // namespace

bool InteractionRegistry::add(const std::string& id,
                              std::unique_ptr<InteractionFactory> factory) {
  if (id.empty() || !factory)
    return false;
  if (factories_.find(id) != factories_.end())
    return false;
  factories_[id] = std::move(factory);
  return true;
}

std::unique_ptr<Interaction> InteractionRegistry::create(
    const std::string& id) const {
  std::map<std::string, std::unique_ptr<InteractionFactory> >::const_iterator
      it = factories_.find(id);
  if (it == factories_.end() || !it->second)
    return std::unique_ptr<Interaction>();
  return it->second->Create();
}

InteractionStack::InteractionStack(InteractionRegistry* registry,
                                   const std::string& view_id)
    : registry_(registry), view_id_(view_id) {}

bool InteractionStack::Allows(const std::string& id) const {
  if (IsView3dTool(id))
    return IsVolumeView(view_id_);
  return true;
}

std::unique_ptr<Interaction> InteractionStack::CreateIfAllowed(
    const std::string& id) const {
  if (id.empty() || !registry_ || !Allows(id))
    return std::unique_ptr<Interaction>();
  return registry_->create(id);
}

bool InteractionStack::activate(const std::string& id) {
  std::unique_ptr<Interaction> created = CreateIfAllowed(id);
  if (!created)
    return false;
  stack_.clear();
  stack_.push_back(std::move(created));
  return true;
}

bool InteractionStack::push(const std::string& id) {
  std::unique_ptr<Interaction> created = CreateIfAllowed(id);
  if (!created)
    return false;
  stack_.push_back(std::move(created));
  return true;
}

bool InteractionStack::pop() {
  if (stack_.empty())
    return false;
  stack_.pop_back();
  return true;
}

Interaction* InteractionStack::current() const {
  if (stack_.empty())
    return nullptr;
  return stack_.back().get();
}

InputRouter::InputRouter() : stack_(nullptr) {}

void InputRouter::add_always_on(Interaction* interaction) {
  if (!interaction)
    return;
  always_on_.push_back(interaction);
}

void InputRouter::set_stack(InteractionStack* stack) {
  stack_ = stack;
}

bool InputRouter::dispatch(const InputEvent& event) {
  for (size_t i = 0; i < always_on_.size(); ++i) {
    Interaction* always_on = always_on_[i];
    if (always_on && always_on->on_input(event))
      return true;
  }
  Interaction* exclusive = stack_ ? stack_->current() : nullptr;
  if (exclusive && exclusive->on_input(event))
    return true;
  return false;
}

bool WheelZoom::on_input(const InputEvent& event) {
  return event.kind == kWheel;
}

bool HoverCursor::on_input(const InputEvent& event) {
  if (event.kind == kMouse && event.action == kMouseMove)
    return false;
  return false;
}

}  // namespace tool
