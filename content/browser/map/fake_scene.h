// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#ifndef CONTENT_BROWSER_MAP_FAKE_SCENE_H_
#define CONTENT_BROWSER_MAP_FAKE_SCENE_H_

#include <string>

#include "content/public/browser/scene_mode.h"
#include "content/public/types.h"

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

  // Arms the orbit sample used by expect_camera in SCENE3D.
  void ArmTrackball();
  // A move while the trackball is armed sets the orbit delta to 1.
  void NotePointer(const InputEvent& event);

  // SCENE2D compares lon/lat within 1e-6 degrees and height within 1 meter.
  // SCENE3D succeeds when the orbit delta is greater than 0.
  bool ExpectCamera(bool has_target,
                    double lon,
                    double lat,
                    double height,
                    std::string* error) const;

 private:
  FakeScene(const FakeScene&) = delete;
  FakeScene& operator=(const FakeScene&) = delete;

  void* child_;
  SceneMode mode_;
  double lon_;
  double lat_;
  double height_;
  double orbit_delta_;
  bool trackball_armed_;
};

}  // namespace content

#endif  // CONTENT_BROWSER_MAP_FAKE_SCENE_H_
