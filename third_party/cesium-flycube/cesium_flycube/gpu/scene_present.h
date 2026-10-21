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
  // White triangle edges. The city showcase turns this off.
  bool draw_edges = true;
};

struct PresentMesh {
  FlyCubeMesh mesh;
  float color[4];
  // Optional RGBA per vertex. Empty means `color` for every vertex.
  std::vector<float> colors;
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

// One PNG with two 16:9 panels and the same DX12 window. The panels do not
// share a projection; each camera keeps its own vertical field of view.
PresentResult PresentSplitScene(const char* title,
                                 const PresentCamera& left,
                                 const PresentCamera& right,
                                 const std::vector<PresentMesh>& meshes,
                                 const std::string& png_path);

// How long a browse window stays up, and where an automatic flight ends.
struct BrowseOptions {
  // 0 keeps the window open until the user closes it.
  double seconds = 0;
  bool auto_fly = true;
  PresentCamera fly_target;
};

struct BrowseResult {
  bool ok = false;
  bool flew = false;
  std::uint32_t frames = 0;
  std::uint32_t drawn_pixels = 0;
  float eye[3] = {};
  std::string message;
};

// CPU projection only. Used to prove a camera move without a window.
std::uint32_t CountDrawnPixels(const PresentCamera& camera,
                               const std::vector<PresentMesh>& meshes);

// Interactive orbit (left drag, right pan, wheel) plus an optional flight.
// The same mesh is reprojected every frame so the motion is visible.
BrowseResult PresentBrowse(const char* title,
                           const PresentCamera& camera,
                           const std::vector<PresentMesh>& meshes,
                           const BrowseOptions& options);

}  // namespace cesium_flycube
