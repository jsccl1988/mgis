// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#pragma once

#include <Cesium3DTilesSelection/TilesetOptions.h>

#include <cstdint>

namespace cesium_flycube {

// The TilesetOptions fields ACesium3DTileset forwards into cesium-native.
// Unreal occlusion queries and materials stay out of this struct.
struct TilesetSettings {
  double maximum_screen_space_error = 16.0;
  std::uint32_t maximum_simultaneous_tile_loads = 20;
  bool preload_ancestors = true;
  bool preload_siblings = true;
  std::uint32_t loading_descendant_limit = 20;
  bool forbid_holes = false;
  bool enable_frustum_culling = true;
  bool enable_occlusion_culling = true;
  bool delay_refinement_for_occlusion = true;
  bool enable_fog_culling = true;
  bool enforce_culled_screen_space_error = true;
  double culled_screen_space_error = 64.0;
  std::int64_t maximum_cached_bytes = 512LL * 1024 * 1024;
  bool render_tiles_under_camera = true;
  bool enable_lod_transition_period = false;
  float lod_transition_length = 1.0f;
  bool show_credits_on_screen = false;
};

void ApplyTilesetSettings(const TilesetSettings& settings,
                          Cesium3DTilesSelection::TilesetOptions* options);

}  // namespace cesium_flycube
