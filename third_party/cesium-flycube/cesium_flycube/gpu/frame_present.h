// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#pragma once

#include "cesium_flycube/geo/georeference.h"
#include "cesium_flycube/gpu/scene_present.h"
#include "cesium_flycube/tiles/tile_mesh.h"

#include <vector>

namespace cesium_flycube {

// Converts a tileset mesh into a PresentMesh. Samples RasterDrape into
// per-vertex colors so CPU raster and FlyCube DX12 share the same look.
PresentMesh TileMeshToPresent(const TileMesh& tile, const Georeference& geo);

std::vector<PresentMesh> FrameToPresent(const std::vector<TileMesh>& tiles,
                                        const Georeference& geo);

}  // namespace cesium_flycube
