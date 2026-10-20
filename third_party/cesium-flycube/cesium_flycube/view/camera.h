// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

namespace cesium_flycube {

// One explicit view. Cesium for Unreal's CameraComponent source is not used;
// FlyCube has no actor, so parameters are written directly (Manual).
struct Camera {
  glm::dvec2 viewport_size{1280.0, 720.0};
  glm::dvec3 location{0.0, 0.0, 1500.0};
  glm::dvec3 direction{0.0, 0.0, -1.0};
  glm::dvec3 up{0.0, 1.0, 0.0};
  double horizontal_fov_degrees = 60.0;
};

}  // namespace cesium_flycube
