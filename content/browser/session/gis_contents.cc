// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "content/public/browser/gis_contents.h"

#include "content/browser/viewport/map/scene.h"
#include "content/browser/viewport/map/scene_factory.h"
#include "content/browser/glang.runtime/script_runtime.h"
#include "content/browser/tooling/tool_session.h"
#include "content/public/browser/plugin_host.h"
#include "content/public/browser/types.h"

namespace content {

GisContents::GisContents()
    : document_(nullptr),
      scene_(CreateScene()),
      tools_(new ToolSession()),
      plugins_(new PluginHost()),
      host_(new CapabilityHost()),
      attach_parent_(NULL) {
  tools_->set_scene(scene_);
  tools_->set_event_bus(&event_bus_);
  tools_->set_trace_log(&trace_log_);
  plugins_->set_tool_session(tools_);
  host_->set_command_slot(&GisContents::OnCommand, this);
  host_->set_activate_slot(&GisContents::OnActivate, this);
  host_->set_pointer_slot(&GisContents::OnPointer, this);
  host_->set_scene_mode_slot(&GisContents::OnSceneMode, this);
  host_->set_camera_slot(&GisContents::OnExpectCamera, this);
  host_->set_attach_slot(&GisContents::OnAttach, this);
  host_->set_layer_slot(&GisContents::OnExpectLayer, this);
  host_->set_entity_slot(&GisContents::OnExpectEntity, this);
}

GisContents::~GisContents() {
  delete host_;
  host_ = NULL;
  delete plugins_;
  plugins_ = NULL;
  delete tools_;
  tools_ = NULL;
  delete scene_;
  scene_ = NULL;
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
  if (ok) {
    attach_parent_ = parent_hwnd;
    scene_->SetInputForwarder(&GisContents::OnPointer, this);
    // Default 2D navigation so the first Map tab drag works without a toolbar
    // click. 3D tab switches ActivateSceneTool(view3d.trackball) in the shell.
    ActivateSceneTool("view.pan");
  }
  return ok;
}

void* GisContents::scene_child_hwnd() const {
  return scene_ ? scene_->child_hwnd() : NULL;
}

void* GisContents::scene_child_hwnd_for(SceneMode mode) const {
  return scene_ ? scene_->child_hwnd_for(mode) : NULL;
}

void GisContents::SetSceneMode(SceneMode mode) {
  scene_->SetSceneMode(mode, &trace_log_);
}

SceneMode GisContents::scene_mode() const {
  return scene_->scene_mode();
}

void GisContents::ResizeScene(int width, int height) {
  if (scene_)
    scene_->Resize(width, height);
}

void GisContents::PresentScene() {
  if (scene_)
    scene_->Present();
}

std::string GisContents::Pick(int x, int y) const {
  return scene_->Pick(x, y);
}

ToolSession* GisContents::tool_session() {
  return tools_;
}

bool GisContents::ActivateSceneTool(const std::string& interaction_id) {
  if (!tools_ || !scene_ || interaction_id.empty())
    return false;
  if (!tools_->activate(interaction_id, kViewIdScene))
    return false;
  if (!scene_->ActivateTool(interaction_id, kViewIdScene))
    return false;
  if (interaction_id == "view3d.trackball")
    scene_->ArmTrackball();
  return true;
}

bool GisContents::HandlePointer(const InputEvent& event) {
  return OnPointer(this, event);
}

PluginHost* GisContents::plugin_host() {
  return plugins_;
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
  if (!self || !self->tools_ || !self->scene_)
    return false;
  if (!self->tools_->activate(command_id, view_id))
    return false;
  // Scene tools live on Scene; Workspace does not own kViewIdScene
  // interactions. Keep both sides in sync for suite / agent lines.
  if (view_id == kViewIdScene) {
    if (!self->scene_->ActivateTool(command_id, view_id))
      return false;
    if (command_id == "view3d.trackball")
      self->scene_->ArmTrackball();
  }
  return true;
}

bool GisContents::OnPointer(void* context, const InputEvent& event) {
  GisContents* self = static_cast<GisContents*>(context);
  if (!self || !self->tools_ || !self->scene_)
    return false;
  if (event.view_id == kViewIdScene) {
    self->scene_->NotePointer(event);
    std::string mark;
    if (!self->scene_->OnPointer(event, &mark))
      return false;
    // Still publish GestureApplied for observers; scene may not be consumed
    // by tool::Workspace.
    (void)self->tools_->dispatch_input(event);
    if (!mark.empty())
      self->host_->mark(mark);
    return true;
  }
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

bool GisContents::OnExpectLayer(void* context,
                                const std::string& id,
                                std::string* error) {
  GisContents* self = static_cast<GisContents*>(context);
  if (!self || !self->scene_) {
    if (error)
      *error = "expect_layer: scene missing";
    return false;
  }
  if (!self->scene_->HasLayer(id)) {
    if (error)
      *error = "expect_layer: missing " + id;
    return false;
  }
  return true;
}

bool GisContents::OnExpectEntity(void* context,
                                 const std::string& id,
                                 std::string* error) {
  GisContents* self = static_cast<GisContents*>(context);
  if (!self || !self->scene_) {
    if (error)
      *error = "expect_entity: scene missing";
    return false;
  }
  if (!self->scene_->HasEntity(id)) {
    if (error)
      *error = "expect_entity: missing " + id;
    return false;
  }
  return true;
}

}  // namespace content
