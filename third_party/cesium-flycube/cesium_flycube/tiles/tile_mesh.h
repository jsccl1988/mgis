// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#pragma once

#include "cesium_flycube/tiles/feature_table.h"
#include "cesium_flycube/tiles/raster_image.h"

#include <CesiumGltf/Model.h>

#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include <cstdint>
#include <vector>

namespace cesium_flycube {

// CPU mesh extracted from a tile's glTF, positions in ECEF meters.
// Overlay UVs come from `_CESIUMOVERLAY_n` when present, else `TEXCOORD_0`.
struct TileMesh {
  std::vector<glm::dvec3> ecef_positions;
  std::vector<glm::vec2> texcoords;
  std::vector<glm::vec3> normals;
  std::vector<std::uint32_t> indices;
  std::vector<RasterDrape> rasters;
  FeatureBatch features;
};

// Reads POSITION, overlay/base UVs, NORMAL, indices, and feature ids.
TileMesh ExtractTileMesh(const CesiumGltf::Model& model,
                         const glm::dmat4& tile_to_ecef);

// Feature id for vertex `index`, or -1 when missing / out of range.
std::int64_t FeatureIdAt(const TileMesh& mesh, std::size_t index);

}  // namespace cesium_flycube
