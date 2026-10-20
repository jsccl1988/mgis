// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#pragma once

#include <CesiumGltf/Model.h>

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

#include <cstdint>
#include <vector>

namespace cesium_flycube {

// CPU mesh extracted from a tile's glTF, positions in ECEF meters.
struct TileMesh {
  std::vector<glm::dvec3> ecef_positions;
  std::vector<std::uint32_t> indices;
};

// Reads POSITION and indices from every primitive and applies `tile_to_ecef`.
TileMesh ExtractTileMesh(const CesiumGltf::Model& model,
                         const glm::dmat4& tile_to_ecef);

}  // namespace cesium_flycube
