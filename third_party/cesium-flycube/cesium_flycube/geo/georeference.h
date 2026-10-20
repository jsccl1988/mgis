// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#pragma once

#include <CesiumGeospatial/Cartographic.h>
#include <CesiumGeospatial/Ellipsoid.h>
#include <CesiumGeospatial/LocalHorizontalCoordinateSystem.h>

#include <glm/vec3.hpp>

namespace cesium_flycube {

// Maps ECEF on WGS84 into a right-handed FlyCube world: X east, Y north, Z up.
// This is the CesiumGeoreference role from Cesium for Unreal, with meters and
// a right-handed frame instead of Unreal's left-handed centimeters.
class Georeference {
 public:
  static constexpr double kMinimumScale = 1e-6;

  Georeference();

  void SetOriginDegrees(double longitude_degrees,
                        double latitude_degrees,
                        double height_meters);
  void SetScaleToMeters(double scale_to_meters);

  const CesiumGeospatial::Ellipsoid& ellipsoid() const;
  const CesiumGeospatial::Cartographic& origin() const;
  double scale_to_meters() const { return scale_to_meters_; }

  glm::dvec3 EcefToLocal(const glm::dvec3& ecef) const;
  glm::dvec3 LocalToEcef(const glm::dvec3& local) const;
  glm::dvec3 LocalDirectionToEcef(const glm::dvec3& local_direction) const;
  glm::dvec3 CartographicToLocal(const CesiumGeospatial::Cartographic& llh) const;

 private:
  void Rebuild();

  CesiumGeospatial::Cartographic origin_;
  double scale_to_meters_ = 1.0;
  CesiumGeospatial::LocalHorizontalCoordinateSystem frame_;
};

}  // namespace cesium_flycube
