// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#ifndef CONTENT_BROWSER_VIEWPORT_MAP_FAKE_SCENE_H_
#define CONTENT_BROWSER_VIEWPORT_MAP_FAKE_SCENE_H_

#include <string>
#include <vector>

#include "content/browser/viewport/map/scene.h"
#include "content/public/browser/types.h"

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
              TraceLog* trace) override;

  // Non-NULL after a successful Attach. Owned by this scene.
  void* child_hwnd() const override { return child_; }
  void* child_hwnd_for(SceneMode /*mode*/) const override { return child_; }

  // Stores |mode| and appends "scene.mode scene2d|scene3d|columbus".
  // Does not clear the registered primitive ids.
  void SetSceneMode(SceneMode mode, TraceLog* trace) override;
  SceneMode scene_mode() const override;
  std::string Pick(int x, int y) const override;
  bool HasLayer(const std::string& id) const override;
  bool HasEntity(const std::string& id) const override;

  // Arms the orbit sample used by expect_camera in SCENE3D.
  void ArmTrackball() override;
  // A move while the trackball is armed sets the orbit delta to 1.
  void NotePointer(const InputEvent& event) override;

  // SCENE2D compares lon/lat within 1e-6 degrees and height within 1 meter.
  // SCENE3D succeeds when the orbit delta is greater than 0.
  bool ExpectCamera(bool has_target,
                    double lon,
                    double lat,
                    double height,
                    std::string* error) const override;

  // |label| is scene2d, scene3d, or columbus. Other text returns false.
  // Clears an in-progress edit draft and does not drop registered primitives.
  bool SetModeLabel(const std::string& label, TraceLog* trace);

  // view.pan is only 2D or Columbus. selection.point and edit.append.point
  // are 2D or 3D. The view id must be kViewIdScene.
  bool ActivateTool(const std::string& tool_id,
                    const std::string& view_id) override;

  // A selection press writes the mode's primitive id into |mark|.
  // An edit press records one draft vertex. |mark| may be empty on success.
  bool OnPointer(const InputEvent& event, std::string* mark) override;

  // After edit.append.point recorded a vertex, writes draft.basemap or
  // draft.groundwater into |mark|.
  bool Commit(std::string* mark) override;

 private:
  FakeScene(const FakeScene&) = delete;
  FakeScene& operator=(const FakeScene&) = delete;

  bool HasPrimitive(const std::string& id) const;

  void* child_;
  SceneMode mode_;
  std::vector<std::string> primitive_ids_;
  std::string tool_id_;
  int draft_points_;
  double lon_;
  double lat_;
  double height_;
  double orbit_delta_;
  bool trackball_armed_;
};

}  // namespace content

#endif  // CONTENT_BROWSER_VIEWPORT_MAP_FAKE_SCENE_H_
