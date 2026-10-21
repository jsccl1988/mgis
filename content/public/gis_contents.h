// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#ifndef CONTENT_PUBLIC_GIS_CONTENTS_H_
#define CONTENT_PUBLIC_GIS_CONTENTS_H_

#include <string>

#include "content/public/browser/scene_mode.h"
#include "content/public/event_bus.h"
#include "content/public/trace_log.h"

namespace content {

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

  // Primitive mark at |x|, |y|. Empty when the current mode misses.
  std::string Pick(int x, int y) const;

  // Session owned by this contents. Browser tests call it. The shell does not.
  ToolSession* tool_session();

 private:
  GisContents(const GisContents&) = delete;
  GisContents& operator=(const GisContents&) = delete;

  GisDocument* document_;
  EventBus event_bus_;
  TraceLog trace_log_;
  // Owned. The scene child lives in the browser layer.
  FakeScene* scene_;
  // Owned. Reads |scene_| when activate() targets kViewIdScene.
  ToolSession* tools_;
};

}  // namespace content

#endif  // CONTENT_PUBLIC_GIS_CONTENTS_H_
