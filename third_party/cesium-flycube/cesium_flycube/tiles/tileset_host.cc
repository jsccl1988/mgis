// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "cesium_flycube/tiles/tileset_host.h"

#include "cesium_flycube/tiles/tileset_services.h"

#include <Cesium3DTilesSelection/EllipsoidTilesetLoader.h>
#include <Cesium3DTilesSelection/ViewUpdateResult.h>
#include <Cesium3DTilesSelection/Tileset.h>
#include <Cesium3DTilesSelection/TilesetOptions.h>
#include <Cesium3DTilesSelection/TilesetViewGroup.h>
#include <CesiumGeospatial/Cartographic.h>

#include <utility>

namespace cesium_flycube {

struct TilesetHost::State {
  TilesetServices services;
  Cesium3DTilesSelection::TilesetViewGroup view_group;
  std::unique_ptr<Cesium3DTilesSelection::Tileset> tileset;
};

TilesetHost::TilesetHost() : state_(std::make_unique<State>()) {}

TilesetHost::~TilesetHost() {
  if (state_) {
    state_->tileset.reset();
  }
}

void TilesetHost::LoadEllipsoid() {
  Cesium3DTilesSelection::TilesetOptions options;
  options.maximumScreenSpaceError = 16.0;
  state_->tileset = Cesium3DTilesSelection::EllipsoidTilesetLoader::createTileset(
      state_->services.Externals(), options);
}

void TilesetHost::LoadUrl(const std::string& url) {
  state_->tileset = std::make_unique<Cesium3DTilesSelection::Tileset>(
      state_->services.Externals(), url, Cesium3DTilesSelection::TilesetOptions{});
}

void TilesetHost::LoadIon(int64_t asset_id, const std::string& access_token) {
  state_->tileset = std::make_unique<Cesium3DTilesSelection::Tileset>(
      state_->services.Externals(),
      asset_id,
      access_token,
      Cesium3DTilesSelection::TilesetOptions{});
}

TilesetFrame TilesetHost::Tick(
    const std::vector<Cesium3DTilesSelection::ViewState>& views,
    float delta_seconds) {
  if (!state_->tileset) {
    return {};
  }
  state_->services.Pump();
  const Cesium3DTilesSelection::ViewUpdateResult& update =
      state_->tileset->updateViewGroup(state_->view_group, views, delta_seconds);
  state_->services.Pump();
  state_->tileset->loadTiles();
  return CollectTilesetFrame(update, state_->services.credits());
}

CesiumAsync::Future<double> TilesetHost::SampleHeightDegrees(
    double longitude_degrees,
    double latitude_degrees) {
  const std::vector<CesiumGeospatial::Cartographic> positions = {
      CesiumGeospatial::Cartographic::fromDegrees(
          longitude_degrees, latitude_degrees, 0.0)};
  return std::move(state_->tileset->sampleHeightMostDetailed(positions))
      .thenImmediately([](Cesium3DTilesSelection::SampleHeightResult&& result) {
        if (result.positions.empty()) {
          return 0.0;
        }
        return result.positions.front().height;
      });
}

}  // namespace cesium_flycube
