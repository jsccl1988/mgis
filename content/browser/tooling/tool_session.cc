// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "content/browser/tooling/tool_session.h"

#include <sstream>

#include "content/browser/viewport/map/scene.h"
#include "content/public/browser/event_bus.h"
#include "content/public/browser/trace_log.h"
#include "tool/workspace.h"

namespace content {

struct ToolSession::Routers {
  Routers()
      : map(kViewIdMap),
        view0(kViewIdView0),
        view1(kViewIdView1),
        scene(kViewIdScene) {}

  tool::Workspace map;
  tool::Workspace view0;
  tool::Workspace view1;
  tool::Workspace scene;
};

namespace {

tool::InputEvent ToToolEvent(const InputEvent& event) {
  tool::InputEvent out;
  out.view_id = event.view_id;
  out.x_px = event.x;
  out.y_px = event.y;
  out.modifiers = event.modifiers;
  if (event.kind == kInputEventWheel) {
    out.kind = tool::kWheel;
    out.wheel_dy = event.wheel_delta;
    return out;
  }
  if (event.kind == kInputEventKey) {
    out.kind = tool::kKey;
    return out;
  }
  out.kind = tool::kMouse;
  if (event.mouse_action == kMouseDown)
    out.action = tool::kMousePress;
  else if (event.mouse_action == kMouseUp)
    out.action = tool::kMouseRelease;
  else
    out.action = tool::kMouseMove;
  return out;
}

const char* KindName(InputEventKind kind) {
  if (kind == kInputEventWheel)
    return "wheel";
  if (kind == kInputEventKey)
    return "key";
  return "mouse";
}

bool SceneToolAllowed(SceneMode mode, const std::string& interaction_id) {
  const bool planar = interaction_id == "view.pan" ||
                      interaction_id == "view.zoom_in" ||
                      interaction_id == "view.zoom_out";
  if (planar)
    return mode == SceneMode::kScene2D || mode == SceneMode::kColumbusView;
  if (interaction_id == "view3d.trackball")
    return mode == SceneMode::kScene3D;
  if (interaction_id == "selection.point" ||
      interaction_id == "edit.append.point") {
    return mode == SceneMode::kScene2D || mode == SceneMode::kScene3D;
  }
  return false;
}

}  // namespace

tool::Workspace* ToolSession::WorkspaceFor(const std::string& view_id) {
  if (!routers_)
    return NULL;
  if (view_id == kViewIdMap)
    return &routers_->map;
  if (view_id == kViewIdView0)
    return &routers_->view0;
  if (view_id == kViewIdView1)
    return &routers_->view1;
  if (view_id == kViewIdScene)
    return &routers_->scene;
  return NULL;
}

ToolSession::ToolSession()
    : has_last_input_(false),
      routers_(new Routers()),
      event_bus_(NULL),
      trace_log_(NULL),
      scene_(NULL) {}

ToolSession::~ToolSession() {
  delete routers_;
  routers_ = NULL;
}

void ToolSession::set_event_bus(EventBus* event_bus) {
  event_bus_ = event_bus;
}

void ToolSession::set_trace_log(TraceLog* trace_log) {
  trace_log_ = trace_log;
}

void ToolSession::set_scene(Scene* scene) {
  scene_ = scene;
}

bool ToolSession::execute(const std::string& command_id,
                          const std::string& view_id) {
  tool::Workspace* workspace = WorkspaceFor(view_id);
  if (command_id.empty() || !workspace)
    return false;
  if (!workspace->execute(command_id))
    return false;
  last_command_id_ = command_id;
  last_command_view_id_ = view_id;
  if (trace_log_)
    trace_log_->Append(kTraceConsole, "command " + command_id + " " + view_id);
  return true;
}

bool ToolSession::dispatch_input(const InputEvent& event) {
  tool::Workspace* workspace = WorkspaceFor(event.view_id);
  if (!workspace)
    return false;
  const bool perf = event.kind != kInputEventMouse ||
                    event.mouse_action != kMouseMove;
  TraceScope scope(perf ? trace_log_ : NULL, "dispatch_input");
  const bool consumed = workspace->dispatch(ToToolEvent(event));
  last_input_ = event;
  has_last_input_ = true;

  GestureApplied applied;
  applied.view_id = event.view_id;
  applied.kind = event.kind;
  applied.mouse_action = event.mouse_action;
  applied.x = event.x;
  applied.y = event.y;
  applied.wheel_delta = event.wheel_delta;
  applied.consumed = consumed;
  if (event_bus_)
    event_bus_->publish(applied);
  if (trace_log_) {
    std::ostringstream line;
    line << "gesture " << event.view_id << " " << KindName(event.kind) << " "
         << event.x << "," << event.y
         << (consumed ? " consumed" : " open");
    trace_log_->Append(kTraceConsole, line.str());
  }
  return consumed;
}

bool ToolSession::activate(const std::string& interaction_id,
                           const std::string& view_id) {
  if (view_id == kViewIdScene) {
    if (interaction_id.empty() || !scene_ ||
        !SceneToolAllowed(scene_->scene_mode(), interaction_id)) {
      return false;
    }
    last_command_id_ = interaction_id;
    last_command_view_id_ = view_id;
    last_interaction_id_ = interaction_id;
    last_interaction_view_id_ = view_id;
    if (trace_log_) {
      trace_log_->Append(kTraceConsole,
                         "command " + interaction_id + " " + view_id);
    }
    return true;
  }
  if (!execute(interaction_id, view_id))
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
