// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#pragma once

#include "cesium_flycube/geo/georeference.h"
#include "cesium_flycube/view/camera_manager.h"

#include <string>

namespace cesium_flycube {

// Rejects AdditionalCameras that cannot share one georeference origin.
class OriginPolicy {
 public:
  static constexpr double kMaxCameraSeparationMeters = 100000.0;

  static bool CamerasShareOrigin(const Georeference& georeference,
                                 const CameraManager& cameras,
                                 std::string* reason);
};

}  // namespace cesium_flycube
