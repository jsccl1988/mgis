// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#ifndef CONTENT_PUBLIC_BROWSER_GIS_CONTENTS_H_
#define CONTENT_PUBLIC_BROWSER_GIS_CONTENTS_H_

#include <string>

#include "content/public/browser/scene_mode.h"
#include "content/public/browser/event_bus.h"
#include "content/public/browser/trace_log.h"
#include "content/public/browser/types.h"

namespace content {

class CapabilityHost;
class GisDocument;
class PluginHost;
class Scene;
class ToolSession;

// Viewport and document host for one desktop session.
// This is not a browser profile or a web contents object. The shell keeps
// aura windows; this object keeps the document pointer, the session bus,
// and one scene child (or per-mode presenter children).
class GisContents {
 public:
  GisContents();
  ~GisContents();

  // Not owned. NULL until set_document().
  GisDocument* document() const;
  void set_document(GisDocument* document);

  // Owned by this contents. Never NULL. Lifetime matches the contents.
  EventBus* event_bus();
  TraceLog* trace_log();

  // True for the three product view ids (map, view0, view1).
  bool has_view(const std::string& view_id) const;

  // Attaches a child only when |view_id| is kViewIdScene.
  bool AttachView(void* parent_hwnd,
                  const std::string& view_id,
                  int width,
                  int height);

  // Active-mode scene child HWND after AttachView succeeds; NULL otherwise.
  void* scene_child_hwnd() const;

  // Child for |mode|. FakeScene returns the same HWND for every mode.
  void* scene_child_hwnd_for(SceneMode mode) const;

  void SetSceneMode(SceneMode mode);
  SceneMode scene_mode() const;

  void ResizeScene(int width, int height);
  void PresentScene();

  // Primitive mark at |x|, |y|. Empty when the current mode misses.
  std::string Pick(int x, int y) const;

  // Session owned by this contents. Browser tests call it. The shell does not.
  ToolSession* tool_session();

  // Shell map-tool entry: activates |interaction_id| on kViewIdScene without
  // requiring the shell to include content/browser tooling headers.
  bool ActivateSceneTool(const std::string& interaction_id);

  // Product HWND / suite path: one pointer sample into Scene + ToolSession.
  bool HandlePointer(const InputEvent& event);

  // In-process plugin contribution surface. Owned by this contents. Never NULL.
  PluginHost* plugin_host();

  // Suite host for browser tests. The shell does not call this.
  CapabilityHost& capability_host();

 private:
  GisContents(const GisContents&) = delete;
  GisContents& operator=(const GisContents&) = delete;

  static bool OnCommand(void* context,
                        const std::string& command_id,
                        const std::string& view_id);
  static bool OnActivate(void* context,
                         const std::string& command_id,
                         const std::string& view_id);
  static bool OnPointer(void* context, const InputEvent& event);
  static bool OnSceneMode(void* context, const std::string& mode);
  static bool OnExpectCamera(void* context,
                             const std::string& view_id,
                             bool has_target,
                             double lon,
                             double lat,
                             double height,
                             std::string* error);
  static bool OnAttach(void* context,
                       const std::string& view_id,
                       std::string* error);
  static bool OnExpectLayer(void* context,
                            const std::string& id,
                            std::string* error);
  static bool OnExpectEntity(void* context,
                             const std::string& id,
                             std::string* error);

  GisDocument* document_;
  EventBus event_bus_;
  TraceLog trace_log_;
  // Owned. Created via CreateScene() (FakeScene unless product factory set).
  Scene* scene_;
  // Owned. Reads |scene_| when activate() targets kViewIdScene.
  ToolSession* tools_;
  // Owned. Bound to |tools_| so contributed factories reach Workspace.
  PluginHost* plugins_;
  CapabilityHost* host_;
  void* attach_parent_;
};

}  // namespace content

#endif  // CONTENT_PUBLIC_BROWSER_GIS_CONTENTS_H_
