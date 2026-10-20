// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#pragma once

#include "cesium_flycube/view/camera.h"

#include <vector>

namespace cesium_flycube {

// Collects the views a tileset uses for LOD. Mirrors ACesiumCameraManager:
// player, editor, and scene-capture collection stay off; callers fill
// AdditionalCameras. AddCamera is not provided.
class CameraManager {
 public:
  bool use_player_cameras = false;
  bool use_editor_cameras = false;
  bool use_scene_captures = false;

  std::vector<Camera>& additional_cameras() { return additional_cameras_; }
  const std::vector<Camera>& additional_cameras() const {
    return additional_cameras_;
  }

 private:
  std::vector<Camera> additional_cameras_;
};

}  // namespace cesium_flycube
