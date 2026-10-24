// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "cesium_flycube/geo/origin_shift.h"

#include <glm/geometric.hpp>

namespace cesium_flycube {

bool OriginShift::Apply(Georeference* georeference,
                        const GlobeAnchor& anchor) const {
  if (georeference == nullptr || mode != OriginShiftMode::kChangeGeoreference) {
    return false;
  }
  const glm::dvec3 local = anchor.LocalPosition(*georeference);
  if (distance_meters > 0.0 && glm::length(local) <= distance_meters) {
    return false;
  }
  georeference->SetOriginDegrees(anchor.longitude_degrees(),
                                 anchor.latitude_degrees(),
                                 anchor.height_meters());
  return true;
}

}  // namespace cesium_flycube
