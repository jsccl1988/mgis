// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#pragma once

#include "cesium_flycube/tiles/frame.h"

#include <Cesium3DTilesSelection/ViewState.h>
#include <CesiumAsync/Future.h>

#include <memory>
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

  void LoadEllipsoid();
  void LoadUrl(const std::string& url);
  void LoadIon(int64_t asset_id, const std::string& access_token);

  TilesetFrame Tick(const std::vector<Cesium3DTilesSelection::ViewState>& views,
                    float delta_seconds);

  CesiumAsync::Future<double> SampleHeightDegrees(double longitude_degrees,
                                                  double latitude_degrees);

 private:
  struct State;
  std::unique_ptr<State> state_;
};

}  // namespace cesium_flycube
