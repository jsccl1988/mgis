// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "cesium_flycube/gpu/frame_present.h"

#include "cesium_flycube/tiles/raster_image.h"

#include <algorithm>

namespace cesium_flycube {

PresentMesh TileMeshToPresent(const TileMesh& tile, const Georeference& geo) {
  PresentMesh mesh;
  mesh.color[0] = 0.35f;
  mesh.color[1] = 0.45f;
  mesh.color[2] = 0.55f;
  mesh.color[3] = 1.0f;
  mesh.mesh.indices = tile.indices;
  mesh.mesh.xyz.reserve(tile.ecef_positions.size() * 3);
  mesh.colors.reserve(tile.ecef_positions.size() * 4);

  std::uint8_t solid[4] = {};
  const bool have_solid =
      !tile.rasters.empty() && tile.texcoords.empty() &&
      SampleRaster(tile.rasters.front(), glm::vec2(0.5f, 0.5f), solid);
  if (have_solid) {
    mesh.color[0] = static_cast<float>(solid[0]) / 255.0f;
    mesh.color[1] = static_cast<float>(solid[1]) / 255.0f;
    mesh.color[2] = static_cast<float>(solid[2]) / 255.0f;
  }

  for (size_t i = 0; i < tile.ecef_positions.size(); ++i) {
    const glm::dvec3 local = geo.EcefToLocal(tile.ecef_positions[i]);
    mesh.mesh.xyz.push_back(static_cast<float>(local.x));
    mesh.mesh.xyz.push_back(static_cast<float>(local.y));
    mesh.mesh.xyz.push_back(static_cast<float>(local.z));

    float rgba[4] = {mesh.color[0], mesh.color[1], mesh.color[2], 1.0f};
    if (have_solid) {
      rgba[0] = mesh.color[0];
      rgba[1] = mesh.color[1];
      rgba[2] = mesh.color[2];
    } else if (!tile.rasters.empty() && i < tile.texcoords.size()) {
      std::uint8_t sample[4] = {};
      if (SampleRaster(tile.rasters.front(), tile.texcoords[i], sample)) {
        rgba[0] = static_cast<float>(sample[0]) / 255.0f;
        rgba[1] = static_cast<float>(sample[1]) / 255.0f;
        rgba[2] = static_cast<float>(sample[2]) / 255.0f;
      }
    } else if (!tile.normals.empty() && i < tile.normals.size()) {
      const glm::vec3& n = tile.normals[i];
      const float shade =
          0.55f + 0.45f * std::max(0.0f, n.z);
      rgba[0] *= shade;
      rgba[1] *= shade;
      rgba[2] *= shade;
    }
    mesh.colors.insert(mesh.colors.end(),
                       {rgba[0], rgba[1], rgba[2], rgba[3]});
  }
  return mesh;
}

std::vector<PresentMesh> FrameToPresent(const std::vector<TileMesh>& tiles,
                                        const Georeference& geo) {
  std::vector<PresentMesh> out;
  out.reserve(tiles.size());
  for (const TileMesh& tile : tiles) {
    out.push_back(TileMeshToPresent(tile, geo));
  }
  return out;
}

}  // namespace cesium_flycube
