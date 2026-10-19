// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "content/public/gis_contents.h"

#include "content/public/types.h"

namespace content {

GisContents::GisContents() : document_(nullptr) {}

GisContents::~GisContents() {}

GisDocument* GisContents::document() const {
  return document_;
}

void GisContents::set_document(GisDocument* document) {
  document_ = document;
}

EventBus* GisContents::event_bus() {
  return &event_bus_;
}

bool GisContents::has_view(const std::string& view_id) const {
  return is_known_view_id(view_id);
}

}  // namespace content
