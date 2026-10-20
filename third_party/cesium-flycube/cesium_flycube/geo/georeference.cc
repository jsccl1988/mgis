// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "cesium_flycube/geo/georeference.h"

namespace cesium_flycube {

Georeference::Georeference()
    : origin_(CesiumGeospatial::Cartographic::fromDegrees(0.0, 0.0, 0.0)),
      frame_(origin_,
             CesiumGeospatial::LocalDirection::East,
             CesiumGeospatial::LocalDirection::North,
             CesiumGeospatial::LocalDirection::Up,
             1.0) {}

void Georeference::SetOriginDegrees(double longitude_degrees,
                                    double latitude_degrees,
                                    double height_meters) {
  origin_ = CesiumGeospatial::Cartographic::fromDegrees(
      longitude_degrees, latitude_degrees, height_meters);
  Rebuild();
}

void Georeference::SetScaleToMeters(double scale_to_meters) {
  scale_to_meters_ =
      scale_to_meters < kMinimumScale ? kMinimumScale : scale_to_meters;
  Rebuild();
}

const CesiumGeospatial::Ellipsoid& Georeference::ellipsoid() const {
  return CesiumGeospatial::Ellipsoid::WGS84;
}

const CesiumGeospatial::Cartographic& Georeference::origin() const {
  return origin_;
}

glm::dvec3 Georeference::EcefToLocal(const glm::dvec3& ecef) const {
  return frame_.ecefPositionToLocal(ecef);
}

glm::dvec3 Georeference::LocalToEcef(const glm::dvec3& local) const {
  return frame_.localPositionToEcef(local);
}

glm::dvec3 Georeference::LocalDirectionToEcef(
    const glm::dvec3& local_direction) const {
  return frame_.localDirectionToEcef(local_direction);
}

glm::dvec3 Georeference::CartographicToLocal(
    const CesiumGeospatial::Cartographic& llh) const {
  return EcefToLocal(ellipsoid().cartographicToCartesian(llh));
}

void Georeference::Rebuild() {
  frame_ = CesiumGeospatial::LocalHorizontalCoordinateSystem(
      origin_,
      CesiumGeospatial::LocalDirection::East,
      CesiumGeospatial::LocalDirection::North,
      CesiumGeospatial::LocalDirection::Up,
      scale_to_meters_,
      ellipsoid());
}

}  // namespace cesium_flycube
