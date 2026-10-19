// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

// Prints the WGS84 equatorial radius from the standalone cesium-native pin.
// The UE product uses the copy shipped inside Cesium for Unreal instead.

#include <CesiumGeospatial/Ellipsoid.h>

#include <iostream>

int main() {
  const CesiumGeospatial::Ellipsoid& wgs84 = CesiumGeospatial::Ellipsoid::WGS84;
  std::cout << wgs84.getMaximumRadius() << "\n";
  return 0;
}
