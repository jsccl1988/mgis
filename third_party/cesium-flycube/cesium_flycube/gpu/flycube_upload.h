// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#pragma once

#include "cesium_flycube/gpu/mesh_pack.h"

#include <cstdint>
#include <string>
#include <vector>

namespace cesium_flycube {

struct FlyCubeUploadResult {
  bool ok = false;
  std::string adapter_name;
  std::uint32_t vertex_count = 0;
  std::uint32_t index_count = 0;
  std::string message;
};

// Creates a DX12 FlyCube device and copies packed meshes into upload buffers.
FlyCubeUploadResult UploadToFlyCube(const std::vector<FlyCubeMesh>& meshes);

}  // namespace cesium_flycube
