// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#pragma once

#include "cesium_flycube/geo/georeference.h"
#include "cesium_flycube/view/camera_manager.h"

#include <Cesium3DTilesSelection/ViewState.h>

#include <vector>

namespace cesium_flycube {

// Turns AdditionalCameras into the cesium-native frustums a tileset selects on.
class ViewStateBuilder {
 public:
  static std::vector<Cesium3DTilesSelection::ViewState>
  Build(const Georeference& georeference, const CameraManager& cameras);
};

}  // namespace cesium_flycube
