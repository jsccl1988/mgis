// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#ifndef CONTENT_PUBLIC_GIS_CONTENTS_H_
#define CONTENT_PUBLIC_GIS_CONTENTS_H_

#include <string>

#include "content/public/browser/scene_mode.h"
#include "content/public/event_bus.h"
#include "content/public/trace_log.h"
#include "content/public/types.h"

namespace content {

class CapabilityHost;
class FakeScene;
class GisDocument;
class ToolSession;

// Viewport and document host for one desktop session.
// This is not a browser profile or a web contents object. The shell keeps
// aura windows; this object keeps the document pointer, the session bus,
// and one scene child.
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
  void SetSceneMode(SceneMode mode);
  SceneMode scene_mode() const;

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

  GisDocument* document_;
  EventBus event_bus_;
  TraceLog trace_log_;
  // Owned. The scene child lives in the browser layer.
  FakeScene* scene_;
  ToolSession* tools_;
  CapabilityHost* host_;
  void* attach_parent_;
};

}  // namespace content

#endif  // CONTENT_PUBLIC_GIS_CONTENTS_H_
