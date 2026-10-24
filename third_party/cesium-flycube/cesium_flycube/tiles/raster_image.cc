// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "cesium_flycube/tiles/raster_image.h"

namespace cesium_flycube {
namespace {

int ClampPixel(double value, int limit) {
  if (value < 0.0) {
    return 0;
  }
  if (value >= static_cast<double>(limit)) {
    return limit - 1;
  }
  return static_cast<int>(value);
}

}  // namespace

bool SampleRaster(const RasterDrape& drape,
                  const glm::vec2& texcoord,
                  std::uint8_t rgba[4]) {
  if (rgba == nullptr || !drape.image || drape.image->compressed ||
      drape.image->bytes_per_channel != 1 || drape.image->width <= 0 ||
      drape.image->height <= 0 || drape.image->channels <= 0) {
    return false;
  }
  const RasterImage& image = *drape.image;
  const glm::dvec2 mapped =
      drape.translation +
      drape.scale * glm::dvec2(texcoord.x, texcoord.y);
  const int x = ClampPixel(mapped.x * static_cast<double>(image.width),
                           image.width);
  const int y = ClampPixel(mapped.y * static_cast<double>(image.height),
                           image.height);
  const size_t channels = static_cast<size_t>(image.channels);
  const size_t offset =
      (static_cast<size_t>(y) * static_cast<size_t>(image.width) +
       static_cast<size_t>(x)) *
      channels;
  if (offset + channels > image.pixels.size()) {
    return false;
  }
  rgba[0] = static_cast<std::uint8_t>(image.pixels[offset]);
  rgba[1] = channels > 1 ? static_cast<std::uint8_t>(image.pixels[offset + 1])
                         : rgba[0];
  rgba[2] = channels > 2 ? static_cast<std::uint8_t>(image.pixels[offset + 2])
                         : rgba[0];
  rgba[3] = channels > 3 ? static_cast<std::uint8_t>(image.pixels[offset + 3])
                         : static_cast<std::uint8_t>(255);
  return true;
}

}  // namespace cesium_flycube
