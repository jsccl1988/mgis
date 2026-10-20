// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#pragma once

#include "cesium_flycube/tiles/tile_mesh.h"

#include <cstddef>
#include <string>
#include <vector>

namespace Cesium3DTilesSelection {
class ViewUpdateResult;
}

namespace CesiumUtility {
class CreditSystem;
}

namespace cesium_flycube {

struct TilesetFrame {
  std::size_t tiles_to_render = 0;
  std::size_t meshes_ready = 0;
  std::size_t vertex_count = 0;
  std::size_t index_count = 0;
  std::vector<std::string> credits;
  std::vector<TileMesh> meshes;
};

// Copies the tiles selected this frame into a value the caller can keep.
TilesetFrame CollectTilesetFrame(
    const Cesium3DTilesSelection::ViewUpdateResult& update,
    CesiumUtility::CreditSystem& credits);

}  // namespace cesium_flycube
