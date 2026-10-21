// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "content/public/gis_contents.h"

#include "content/browser/map/fake_scene.h"
#include "content/public/tool_session.h"
#include "content/public/types.h"

namespace content {

GisContents::GisContents()
    : document_(nullptr), scene_(new FakeScene()), tools_(new ToolSession()) {
  tools_->set_scene(scene_);
}

GisContents::~GisContents() {
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
  return scene_->Attach(parent_hwnd, view_id, width, height, &trace_log_);
}

void GisContents::SetSceneMode(SceneMode mode) {
  scene_->SetSceneMode(mode, &trace_log_);
}

SceneMode GisContents::scene_mode() const {
  return scene_->scene_mode();
}

std::string GisContents::Pick(int x, int y) const {
  return scene_->Pick(x, y);
}

ToolSession* GisContents::tool_session() {
  return tools_;
}

}  // namespace content
