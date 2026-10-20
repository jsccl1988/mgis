// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#pragma once

#include "cesium_flycube/gpu/mesh_pack.h"

#include <cstdint>
#include <string>
#include <vector>

namespace cesium_flycube {

struct PresentCamera {
  float eye[3];
  float target[3];
  float up[3];
  float fov_y_radians = 0.9f;
};

struct PresentMesh {
  FlyCubeMesh mesh;
  float color[4];
};

struct PresentResult {
  bool ok = false;
  std::string png_path;
  std::uint32_t drawn_pixels = 0;
  std::string message;
};

// Opens a FlyCube DX12 window and writes the same projected frame to a PNG.
PresentResult PresentScene(const char* title,
                           const PresentCamera& camera,
                           const std::vector<PresentMesh>& meshes,
                           const std::string& png_path);

}  // namespace cesium_flycube
