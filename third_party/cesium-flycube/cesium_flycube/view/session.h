// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#pragma once

#include "cesium_flycube/geo/georeference.h"
#include "cesium_flycube/view/camera_manager.h"
#include "cesium_flycube/view/origin_policy.h"

#include <Cesium3DTilesSelection/ViewState.h>

#include <string>
#include <vector>

namespace cesium_flycube {

// One georeference plus the cameras that observe it. Separation policy and
// view-state conversion live in OriginPolicy and ViewStateBuilder.
class Session {
 public:
  static constexpr double kMaxCameraSeparationMeters =
      OriginPolicy::kMaxCameraSeparationMeters;

  Georeference& georeference() { return georeference_; }
  const Georeference& georeference() const { return georeference_; }

  CameraManager& cameras() { return cameras_; }
  const CameraManager& cameras() const { return cameras_; }

  bool CamerasShareOrigin(std::string* reason) const;
  std::vector<Cesium3DTilesSelection::ViewState> BuildViewStates() const;

 private:
  Georeference georeference_;
  CameraManager cameras_;
};

}  // namespace cesium_flycube
