// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#pragma once

#include "cesium_flycube/geo/georeference.h"
#include "cesium_flycube/geo/globe_anchor.h"

namespace cesium_flycube {

// Matches ECesiumOriginShiftMode. FlyCube has no Unreal sub-levels, so
// kSwitchSubLevelsOnly leaves the origin where it is.
enum class OriginShiftMode {
  kDisabled,
  kSwitchSubLevelsOnly,
  kChangeGeoreference,
};

// Moves the georeference origin toward an anchor so local coordinates stay
// small. Distance 0 shifts on every call when the mode is kChangeGeoreference.
class OriginShift {
 public:
  OriginShiftMode mode = OriginShiftMode::kDisabled;
  double distance_meters = 0.0;

  bool Apply(Georeference* georeference, const GlobeAnchor& anchor) const;
};

}  // namespace cesium_flycube
