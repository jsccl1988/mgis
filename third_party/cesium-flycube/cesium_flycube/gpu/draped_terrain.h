// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#pragma once

#include "cesium_flycube/gpu/scene_present.h"
#include "cesium_flycube/tiles/tileset_host.h"
#include "cesium_flycube/view/session.h"

#include <cstddef>
#include <string>
#include <vector>

namespace cesium_flycube {

// Options for a Shanghai-style draped ellipsoid capture used by examples.
struct DrapedTerrainOptions {
  double longitude_degrees = 121.4737;
  double latitude_degrees = 31.2304;
  double height_meters = 0.0;
  // Prefer Esri World Imagery URL template; falls back to debug colorize.
  bool prefer_imagery = false;
  // Non-empty overrides Esri when prefer_imagery is true (self-hosted TMS).
  std::string imagery_url_template;
  int max_ticks = 120;
  float tick_delta_seconds = 0.05f;
  // Stop when any mesh has a raster drape (debug colorize / imagery).
  bool require_drape = true;
};

struct DrapedTerrainResult {
  bool ok = false;
  bool draped = false;
  std::size_t vertex_count = 0;
  std::string message;
  std::vector<PresentMesh> pictures;
};

// Sets the session origin, loads an ellipsoid tileset with an overlay, ticks
// until meshes (and optionally drapes) are ready, then fills PresentMeshes via
// FrameToPresent. `tileset` must outlive use of the returned pictures' CPU
// data only — PresentMesh owns its own vertex colors after this returns.
DrapedTerrainResult CaptureDrapedEllipsoid(Session* session,
                                           TilesetHost* tileset,
                                           const DrapedTerrainOptions& options);

}  // namespace cesium_flycube
