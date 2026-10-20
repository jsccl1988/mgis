// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "cesium_flycube/geo/globe_anchor.h"

namespace cesium_flycube {

void GlobeAnchor::SetDegrees(double longitude_degrees,
                             double latitude_degrees,
                             double height_meters) {
  longitude_degrees_ = longitude_degrees;
  latitude_degrees_ = latitude_degrees;
  height_meters_ = height_meters;
}

glm::dvec3 GlobeAnchor::LocalPosition(const Georeference& georeference) const {
  return georeference.CartographicToLocal(
      CesiumGeospatial::Cartographic::fromDegrees(
          longitude_degrees_, latitude_degrees_, height_meters_));
}

}  // namespace cesium_flycube
