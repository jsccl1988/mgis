// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#pragma once

#include "cesium_flycube/tiles/frame.h"
#include "cesium_flycube/tiles/tileset_settings.h"

#include <Cesium3DTilesSelection/ViewState.h>
#include <CesiumAsync/Future.h>
#include <CesiumRasterOverlays/RasterOverlay.h>
#include <CesiumUtility/IntrusivePointer.h>

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace cesium_flycube {

// One Cesium3DTileset. Sources match ETilesetSource in Cesium for Unreal:
// ellipsoid, URL, and Cesium ion. Views come from AdditionalCameras.
class TilesetHost {
 public:
  TilesetHost();
  ~TilesetHost();

  TilesetHost(const TilesetHost&) = delete;
  TilesetHost& operator=(const TilesetHost&) = delete;

  TilesetSettings& settings() { return settings_; }
  const TilesetSettings& settings() const { return settings_; }

  void LoadEllipsoid();
  void LoadUrl(const std::string& url);
  void LoadIon(int64_t asset_id, const std::string& access_token);

  void AddRasterOverlay(
      const CesiumUtility::IntrusivePointer<
          const CesiumRasterOverlays::RasterOverlay>& overlay);

  TilesetFrame Tick(const std::vector<Cesium3DTilesSelection::ViewState>& views,
                    float delta_seconds);

  float LoadProgress() const;
  const std::string& last_load_error() const { return last_load_error_; }

  CesiumAsync::Future<double> SampleHeightDegrees(double longitude_degrees,
                                                  double latitude_degrees);

 private:
  struct State;
  TilesetSettings settings_;
  std::string last_load_error_;
  std::unique_ptr<State> state_;
};

}  // namespace cesium_flycube
