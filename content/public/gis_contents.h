// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#ifndef CONTENT_PUBLIC_GIS_CONTENTS_H_
#define CONTENT_PUBLIC_GIS_CONTENTS_H_

#include <string>

#include "content/public/event_bus.h"
#include "content/public/trace_log.h"

namespace content {

class GisDocument;

// Viewport and document host for one desktop session.
// This is not a browser profile or a web contents object. The shell keeps
// aura windows; this object keeps the document pointer and the session bus.
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

 private:
  GisContents(const GisContents&) = delete;
  GisContents& operator=(const GisContents&) = delete;

  GisDocument* document_;
  EventBus event_bus_;
  TraceLog trace_log_;
};

}  // namespace content

#endif  // CONTENT_PUBLIC_GIS_CONTENTS_H_
