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
  // Shade each vertex from normalize(world position). Removes lat-band faceting
  // on Earth-centered globes.
  bool shade_from_origin = false;
  // Vertex colors already include lighting (Shanghai-centered globe meshes).
  bool skip_shade = false;
};

struct PresentMesh {
  FlyCubeMesh mesh;
  float color[4];
  // Optional RGBA per vertex. Empty means `color` for every vertex.
  std::vector<float> colors;
  // Draw under later meshes: fill empty pixels only, do not write depth
  // (atmosphere limb under a globe).
  bool underlay = false;
};

struct PresentResult {
  bool ok = false;
  std::string png_path;
  std::uint32_t drawn_pixels = 0;
  // CPU raster cost for the still (useful for capture HUD / logs).
  float frame_ms = 0;
  float fps = 0;
  std::string message;
};

// Writes the projected frame to a PNG. When `open_window` is true, also opens a
// short FlyCube DX12 window with the same mesh. `stamp_fps` draws an FPS HUD
// into the PNG (CPU raster timing of that frame).
PresentResult PresentScene(const char* title,
                           const PresentCamera& camera,
                           const std::vector<PresentMesh>& meshes,
                           const std::string& png_path,
                           bool open_window = true,
                           bool stamp_fps = false);

// One PNG with two 16:9 panels. The panels do not share a projection; each
// camera keeps its own vertical field of view. Does not open a FlyCube window
// (DX12 present AVs in some sessions; use PresentBrowse for interactive).
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
  // Draw FPS in the window title and as an on-screen HUD.
  bool show_fps = true;
  // Prefer FlyCube DX12 present; falls back to GDI if device init fails.
  bool prefer_gpu = true;
};

struct BrowseResult {
  bool ok = false;
  bool flew = false;
  std::uint32_t frames = 0;
  std::uint32_t drawn_pixels = 0;
  float eye[3] = {};
  // Instantaneous FPS of the last frame and average over the session.
  float fps = 0;
  float avg_fps = 0;
  // "dx12" or "gdi".
  std::string backend;
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

// CPU triangle soup for product HWND present (vista::WorldPass). Built once from
// draped PresentMeshes; Project fills interleaved xyzrgb for DX12 upload.
struct ProductMeshCache {
  std::vector<float> world_xyz;
  std::vector<float> rgb;
  std::vector<std::uint32_t> indices;
  bool empty() const { return indices.empty(); }
};

// |style.skip_shade| should match browse (true for draped vertex colors).
bool BuildProductMeshCache(const PresentCamera& style,
                           const std::vector<PresentMesh>& meshes,
                           ProductMeshCache* cache);
void ProjectProductMeshCache(const PresentCamera& camera,
                             const ProductMeshCache& cache,
                             std::vector<float>* gpu_vertices);

}  // namespace cesium_flycube
