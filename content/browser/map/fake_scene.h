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
// SCENE2D picks primitive:basemap:m0 and SCENE3D picks primitive:groundwater:g0.
// The camera starts at longitude 0, latitude 0, height 0.
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

  // |label| is scene2d, scene3d, or columbus. Other text returns false.
  bool SetModeLabel(const std::string& label, TraceLog* trace);

  // view.pan is only 2D or Columbus. selection.point and edit.append.point
  // are 2D or 3D. The view id must be kViewIdScene.
  bool ActivateTool(const std::string& tool_id, const std::string& view_id);

  // A selection press writes the mode's primitive id into |mark|.
  // An edit press records one draft vertex. |mark| may be empty on success.
  bool OnPointer(const InputEvent& event, std::string* mark);

  // After edit.append.point recorded a vertex, writes draft.basemap or
  // draft.groundwater into |mark|.
  bool Commit(std::string* mark);

  // SCENE2D and Columbus compare |spec| "scene <lon> <lat> <height>".
  // Longitude and latitude use 1e-6 degrees. Height uses 1 meter.
  bool ExpectCamera(const std::string& spec) const;

  std::string Pick() const;

 private:
  FakeScene(const FakeScene&) = delete;
  FakeScene& operator=(const FakeScene&) = delete;

  void* child_;
  SceneMode mode_;
  std::string tool_id_;
  int draft_points_;
  double lon_;
  double lat_;
  double height_;
};

}  // namespace content

#endif  // CONTENT_BROWSER_MAP_FAKE_SCENE_H_
