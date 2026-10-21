// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "content/public/gis_contents.h"

#include "content/browser/map/fake_scene.h"
#include "content/public/script_runtime.h"
#include "content/public/tool_session.h"
#include "content/public/types.h"

namespace content {

GisContents::GisContents()
    : document_(nullptr),
      scene_(new FakeScene()),
      tools_(new ToolSession()),
      host_(new CapabilityHost()),
      attach_parent_(NULL) {
  tools_->set_event_bus(&event_bus_);
  tools_->set_trace_log(&trace_log_);
  host_->set_command_slot(&GisContents::OnCommand, this);
  host_->set_activate_slot(&GisContents::OnActivate, this);
  host_->set_pointer_slot(&GisContents::OnPointer, this);
  host_->set_scene_mode_slot(&GisContents::OnSceneMode, this);
  host_->set_camera_slot(&GisContents::OnExpectCamera, this);
  host_->set_attach_slot(&GisContents::OnAttach, this);
}

GisContents::~GisContents() {
  delete host_;
  delete tools_;
  delete scene_;
}

GisDocument* GisContents::document() const {
  return document_;
}

void GisContents::set_document(GisDocument* document) {
  document_ = document;
}

EventBus* GisContents::event_bus() {
  return &event_bus_;
}

TraceLog* GisContents::trace_log() {
  return &trace_log_;
}

bool GisContents::has_view(const std::string& view_id) const {
  return is_known_view_id(view_id);
}

bool GisContents::AttachView(void* parent_hwnd,
                             const std::string& view_id,
                             int width,
                             int height) {
  const bool ok =
      scene_->Attach(parent_hwnd, view_id, width, height, &trace_log_);
  if (ok)
    attach_parent_ = parent_hwnd;
  return ok;
}

void GisContents::SetSceneMode(SceneMode mode) {
  scene_->SetSceneMode(mode, &trace_log_);
}

SceneMode GisContents::scene_mode() const {
  return scene_->scene_mode();
}

CapabilityHost& GisContents::capability_host() {
  return *host_;
}

bool GisContents::OnCommand(void* context,
                            const std::string& command_id,
                            const std::string& view_id) {
  GisContents* self = static_cast<GisContents*>(context);
  if (!self || !self->tools_)
    return false;
  return self->tools_->execute(command_id, view_id);
}

bool GisContents::OnActivate(void* context,
                             const std::string& command_id,
                             const std::string& view_id) {
  GisContents* self = static_cast<GisContents*>(context);
  if (!self || !self->tools_)
    return false;
  if (!self->tools_->activate(command_id, view_id))
    return false;
  if (command_id == "view3d.trackball" && view_id == kViewIdScene)
    self->scene_->ArmTrackball();
  return true;
}

bool GisContents::OnPointer(void* context, const InputEvent& event) {
  GisContents* self = static_cast<GisContents*>(context);
  if (!self || !self->tools_)
    return false;
  self->scene_->NotePointer(event);
  return self->tools_->dispatch_input(event);
}

bool GisContents::OnSceneMode(void* context, const std::string& mode) {
  GisContents* self = static_cast<GisContents*>(context);
  if (!self)
    return false;
  SceneMode parsed = SceneMode::kScene2D;
  if (mode == "scene3d")
    parsed = SceneMode::kScene3D;
  else if (mode == "columbus")
    parsed = SceneMode::kColumbusView;
  else if (mode != "scene2d")
    return false;
  self->SetSceneMode(parsed);
  return true;
}

bool GisContents::OnExpectCamera(void* context,
                                 const std::string& view_id,
                                 bool has_target,
                                 double lon,
                                 double lat,
                                 double height,
                                 std::string* error) {
  GisContents* self = static_cast<GisContents*>(context);
  if (!self || view_id != kViewIdScene) {
    if (error)
      *error = "expect_camera accepts only scene";
    return false;
  }
  return self->scene_->ExpectCamera(has_target, lon, lat, height, error);
}

bool GisContents::OnAttach(void* context,
                           const std::string& view_id,
                           std::string* error) {
  GisContents* self = static_cast<GisContents*>(context);
  if (!self || view_id != kViewIdScene) {
    if (error)
      *error = "attach accepts only scene";
    return false;
  }
  if (!self->attach_parent_) {
    if (error)
      *error = "parent hwnd missing";
    return false;
  }
  if (!self->AttachView(self->attach_parent_, view_id, 640, 480)) {
    if (error)
      *error = "attach failed";
    return false;
  }
  return true;
}

}  // namespace content
