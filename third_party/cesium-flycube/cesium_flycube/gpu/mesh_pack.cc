// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "cesium_flycube/gpu/mesh_pack.h"

namespace cesium_flycube {

PackedMesh PackMeshes(const std::vector<FlyCubeMesh>& meshes) {
  PackedMesh packed;
  for (const FlyCubeMesh& mesh : meshes) {
    const auto base = static_cast<std::uint32_t>(packed.xyz.size() / 3);
    packed.xyz.insert(packed.xyz.end(), mesh.xyz.begin(), mesh.xyz.end());
    for (std::uint32_t index : mesh.indices) {
      packed.indices.push_back(base + index);
    }
  }
  return packed;
}

}  // namespace cesium_flycube
