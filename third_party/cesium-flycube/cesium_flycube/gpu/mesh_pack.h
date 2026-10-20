// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace cesium_flycube {

// One mesh in FlyCube world space (X east, Y north, Z up), meters.
struct FlyCubeMesh {
  std::vector<float> xyz;
  std::vector<std::uint32_t> indices;
};

// Concatenated vertex and index buffers, still on the CPU.
struct PackedMesh {
  std::vector<float> xyz;
  std::vector<std::uint32_t> indices;
};

PackedMesh PackMeshes(const std::vector<FlyCubeMesh>& meshes);

}  // namespace cesium_flycube
