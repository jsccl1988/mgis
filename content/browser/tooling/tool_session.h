// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#ifndef CONTENT_BROWSER_TOOLING_TOOL_SESSION_H_
#define CONTENT_BROWSER_TOOLING_TOOL_SESSION_H_

#include <string>

#include "content/public/browser/types.h"

namespace tool {
class Workspace;
}

namespace content {

class EventBus;
class Scene;
class TraceLog;

// Per-view command and input session. Each known view id has its own
// tool::Workspace. This header does not include tool/ private headers.
// dispatch_input translates to tool::InputEvent, then publishes GestureApplied.
class ToolSession {
 public:
  ToolSession();
  ~ToolSession();

  // Unowned. NULL skips that side effect.
  void set_event_bus(EventBus* event_bus);
  void set_trace_log(TraceLog* trace_log);
  // Unowned. activate() on kViewIdScene follows this scene's mode.
  // NULL rejects that view. "scene" stays outside is_known_view_id.
  void set_scene(Scene* scene);

  // Records |command_id| for |view_id|. Returns false, and leaves the last
  // command unchanged, when either string is empty or |view_id| is unknown.
  bool execute(const std::string& command_id, const std::string& view_id);

  // Routes |event| and publishes GestureApplied when |view_id| is known.
  // Returns whether the router consumed the sample.
  bool dispatch_input(const InputEvent& event);

  // Records |interaction_id| for |view_id|. Returns false, and leaves the last
  // activation unchanged, when either string is empty or |view_id| is unknown.
  // kViewIdScene is not a known view id. It succeeds only when set_scene() is
  // set and SceneMode allows the tool: pan and zoom in SCENE2D or
  // COLUMBUS_VIEW, view3d.trackball only in SCENE3D.
  bool activate(const std::string& interaction_id, const std::string& view_id);

  const std::string& last_command_id() const;
  const std::string& last_command_view_id() const;
  bool has_last_input() const;
  const InputEvent& last_input() const;
  const std::string& last_interaction_id() const;
  const std::string& last_interaction_view_id() const;

  // Unowned workspace for a known view id, or NULL. PluginHost installs
  // interaction factories into each workspace registry through this.
  tool::Workspace* WorkspaceFor(const std::string& view_id);

 private:
  ToolSession(const ToolSession&) = delete;
  ToolSession& operator=(const ToolSession&) = delete;

  std::string last_command_id_;
  std::string last_command_view_id_;
  bool has_last_input_;
  InputEvent last_input_;
  struct Routers;
  Routers* routers_;
  EventBus* event_bus_;
  TraceLog* trace_log_;
  Scene* scene_;

  std::string last_interaction_id_;
  std::string last_interaction_view_id_;
};

}  // namespace content

#endif  // CONTENT_BROWSER_TOOLING_TOOL_SESSION_H_
