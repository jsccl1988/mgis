// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#ifndef CONTENT_BROWSER_MAP_FAKE_SCENE_H_
#define CONTENT_BROWSER_MAP_FAKE_SCENE_H_

#include <string>

#include "content/public/browser/scene_mode.h"

namespace content {

class TraceLog;

// Placeholder scene child for the single "scene" view.
// A real renderer is not linked here. Failed attaches leave no child HWND.
class FakeScene {
 public:
  FakeScene();
  ~FakeScene();

  // Creates a STATIC child only for kViewIdScene and a non-NULL parent.
  // Other ids append "backend.unavailable <view_id>" and return false.
  bool Attach(void* parent_hwnd,
              const std::string& view_id,
              int width,
              int height,
              TraceLog* trace);

  // Stores |mode| and appends "scene.mode scene2d|scene3d|columbus".
  void SetSceneMode(SceneMode mode, TraceLog* trace);
  SceneMode scene_mode() const;

 private:
  FakeScene(const FakeScene&) = delete;
  FakeScene& operator=(const FakeScene&) = delete;

  void* child_;
  SceneMode mode_;
};

}  // namespace content

#endif  // CONTENT_BROWSER_MAP_FAKE_SCENE_H_
