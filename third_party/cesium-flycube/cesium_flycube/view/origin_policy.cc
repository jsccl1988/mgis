// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "cesium_flycube/view/origin_policy.h"

#include <glm/geometric.hpp>

namespace cesium_flycube {

bool OriginPolicy::CamerasShareOrigin(const Georeference& georeference,
                                      const CameraManager& cameras,
                                      std::string* reason) {
  const std::vector<Camera>& views = cameras.additional_cameras();
  if (views.size() < 2) {
    return true;
  }
  const glm::dvec3 first = georeference.LocalToEcef(views.front().location);
  for (size_t i = 1; i < views.size(); ++i) {
    const glm::dvec3 other = georeference.LocalToEcef(views[i].location);
    if (glm::length(other - first) > kMaxCameraSeparationMeters) {
      if (reason != nullptr) {
        *reason = "origin_conflict";
      }
      return false;
    }
  }
  return true;
}

}  // namespace cesium_flycube
