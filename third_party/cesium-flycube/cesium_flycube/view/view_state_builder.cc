// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "cesium_flycube/view/view_state_builder.h"

#include <CesiumUtility/Math.h>

#include <cmath>

namespace cesium_flycube {

std::vector<Cesium3DTilesSelection::ViewState> ViewStateBuilder::Build(
    const Georeference& georeference,
    const CameraManager& cameras) {
  std::vector<Cesium3DTilesSelection::ViewState> views;
  views.reserve(cameras.additional_cameras().size());
  for (const Camera& camera : cameras.additional_cameras()) {
    const double width = camera.viewport_size.x > 1.0 ? camera.viewport_size.x : 1.0;
    const double height =
        camera.viewport_size.y > 1.0 ? camera.viewport_size.y : 1.0;
    const double horizontal = CesiumUtility::Math::degreesToRadians(
        camera.horizontal_fov_degrees);
    const double vertical =
        2.0 * std::atan(std::tan(horizontal * 0.5) * (height / width));
    views.emplace_back(
        georeference.LocalToEcef(camera.location),
        georeference.LocalDirectionToEcef(camera.direction),
        georeference.LocalDirectionToEcef(camera.up),
        glm::dvec2(width, height),
        horizontal,
        vertical,
        georeference.ellipsoid());
  }
  return views;
}

}  // namespace cesium_flycube
