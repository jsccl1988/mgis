// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#pragma once

#include <glm/vec2.hpp>

#include <cstdint>
#include <memory>
#include <vector>

namespace cesium_flycube {

// Uncompressed pixels copied from a CesiumImage::ImageAsset.
struct RasterImage {
  std::int32_t width = 0;
  std::int32_t height = 0;
  std::int32_t channels = 4;
  std::int32_t bytes_per_channel = 1;
  bool compressed = false;
  std::vector<std::byte> pixels;
};

// One imagery tile draped on a geometry tile. `image` stays alive while the
// raster overlay tile's renderer resources do.
struct RasterDrape {
  std::int32_t texture_coordinate_id = 0;
  glm::dvec2 translation{0.0, 0.0};
  glm::dvec2 scale{1.0, 1.0};
  std::shared_ptr<RasterImage> image;
};

// Samples the drape at a mesh texture coordinate in 0..1.
bool SampleRaster(const RasterDrape& drape,
                  const glm::vec2& texcoord,
                  std::uint8_t rgba[4]);

}  // namespace cesium_flycube
