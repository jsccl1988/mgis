// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#pragma once

#include "cesium_flycube/geo/georeference.h"
#include "cesium_flycube/geo/globe_anchor.h"
#include "cesium_flycube/geo/origin_shift.h"
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

  OriginShift& origin_shift() { return origin_shift_; }
  const OriginShift& origin_shift() const { return origin_shift_; }

  bool CamerasShareOrigin(std::string* reason) const;
  std::vector<Cesium3DTilesSelection::ViewState> BuildViewStates() const;

  // Applies origin_shift_ toward `anchor`. Returns true when the origin moved.
  bool ShiftOrigin(const GlobeAnchor& anchor);

 private:
  Georeference georeference_;
  CameraManager cameras_;
  OriginShift origin_shift_;
};

}  // namespace cesium_flycube
