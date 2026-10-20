// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#pragma once

#include <Cesium3DTilesSelection/TilesetExternals.h>
#include <CesiumAsync/AsyncSystem.h>
#include <CesiumUtility/CreditSystem.h>

#include <memory>

namespace cesium_flycube {

// I/O behind a tileset: worker threads, HTTP, credits, and renderer prep.
// Destroy the tileset before destroying this object.
class TilesetServices {
 public:
  TilesetServices();
  ~TilesetServices();

  TilesetServices(const TilesetServices&) = delete;
  TilesetServices& operator=(const TilesetServices&) = delete;

  Cesium3DTilesSelection::TilesetExternals Externals() const;
  void Pump();
  CesiumUtility::CreditSystem& credits();

 private:
  struct State;
  std::unique_ptr<State> state_;
};

}  // namespace cesium_flycube
