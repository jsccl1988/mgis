// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#pragma once

#include "cesium_flycube/geo/georeference.h"

#include <glm/vec3.hpp>

namespace cesium_flycube {

// A point fixed to the ellipsoid, in the same role as
// UCesiumGlobeAnchorComponent.
class GlobeAnchor {
 public:
  void SetDegrees(double longitude_degrees,
                  double latitude_degrees,
                  double height_meters);

  double longitude_degrees() const { return longitude_degrees_; }
  double latitude_degrees() const { return latitude_degrees_; }
  double height_meters() const { return height_meters_; }

  glm::dvec3 LocalPosition(const Georeference& georeference) const;

 private:
  double longitude_degrees_ = 0.0;
  double latitude_degrees_ = 0.0;
  double height_meters_ = 0.0;
};

}  // namespace cesium_flycube
