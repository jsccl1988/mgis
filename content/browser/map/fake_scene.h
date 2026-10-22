// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#ifndef CONTENT_BROWSER_MAP_FAKE_SCENE_H_
#define CONTENT_BROWSER_MAP_FAKE_SCENE_H_

#include <string>
#include <vector>

#include "content/browser/map/scene.h"

namespace content {

class TraceLog;

// Placeholder scene child for the single "scene" view.
// A real renderer is not linked here. Failed attaches leave no child HWND.
// SCENE2D picks primitive:basemap:m0. SCENE3D picks primitive:groundwater:g0.
// COLUMBUS_VIEW misses. SetSceneMode keeps the registered primitive ids.
class FakeScene : public Scene {
 public:
  FakeScene();
  ~FakeScene() override;

  // Creates a STATIC child only for kViewIdScene and a non-NULL parent.
  // Other ids append "backend.unavailable <view_id>" and return false.
  bool Attach(void* parent_hwnd,
              const std::string& view_id,
              int width,
              int height,
              TraceLog* trace);

  // Stores |mode| and appends "scene.mode scene2d|scene3d|columbus".
  // Does not clear the registered primitive ids.
  void SetSceneMode(SceneMode mode, TraceLog* trace);
  SceneMode scene_mode() const override;
  std::string Pick(int x, int y) const override;

 private:
  FakeScene(const FakeScene&) = delete;
  FakeScene& operator=(const FakeScene&) = delete;

  bool HasPrimitive(const std::string& id) const;

  void* child_;
  SceneMode mode_;
  std::vector<std::string> primitive_ids_;
};

}  // namespace content

#endif  // CONTENT_BROWSER_MAP_FAKE_SCENE_H_
