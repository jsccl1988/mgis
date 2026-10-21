// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "content/browser/map/fake_scene.h"

#include <windows.h>

#include "content/public/trace_log.h"
#include "content/public/types.h"

namespace content {

namespace {

const char* SceneModeLabel(SceneMode mode) {
  if (mode == SceneMode::kScene3D)
    return "scene3d";
  if (mode == SceneMode::kColumbusView)
    return "columbus";
  return "scene2d";
}

}  // namespace

FakeScene::FakeScene() : child_(NULL), mode_(SceneMode::kScene2D) {}

FakeScene::~FakeScene() {
  if (child_)
    DestroyWindow(reinterpret_cast<HWND>(child_));
}

bool FakeScene::Attach(void* parent_hwnd,
                       const std::string& view_id,
                       int width,
                       int height,
                       TraceLog* trace) {
  if (parent_hwnd == NULL || view_id != kViewIdScene) {
    if (trace) {
      trace->Append(kTraceConsole,
                    std::string("backend.unavailable ") + view_id);
    }
    return false;
  }

  HWND child = CreateWindowExW(0, L"STATIC", L"",
                               WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, 0, 0,
                               width, height,
                               reinterpret_cast<HWND>(parent_hwnd), NULL,
                               GetModuleHandleW(NULL), NULL);
  if (!child)
    return false;

  if (child_)
    DestroyWindow(reinterpret_cast<HWND>(child_));
  child_ = child;
  mode_ = SceneMode::kScene2D;
  return true;
}

void FakeScene::SetSceneMode(SceneMode mode, TraceLog* trace) {
  mode_ = mode;
  if (trace) {
    trace->Append(kTraceConsole,
                  std::string("scene.mode ") + SceneModeLabel(mode));
  }
}

SceneMode FakeScene::scene_mode() const {
  return mode_;
}

}  // namespace content
