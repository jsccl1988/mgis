// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "cesium_flycube/tiles/prepare_resources.h"

#include "cesium_flycube/tiles/tile_mesh.h"

#include <Cesium3DTilesSelection/Tile.h>
#include <Cesium3DTilesSelection/TileLoadResult.h>
#include <CesiumAsync/AsyncSystem.h>
#include <CesiumGltf/Model.h>

#include <memory>
#include <utility>
#include <variant>

namespace cesium_flycube {

CesiumAsync::Future<Cesium3DTilesSelection::TileLoadResultAndRenderResources>
PrepareRendererResources::prepareInLoadThread(
    const CesiumAsync::AsyncSystem& async_system,
    Cesium3DTilesSelection::TileLoadResult&& tile_load_result,
    const glm::dmat4& transform,
    const std::any& /*renderer_options*/) {
  Cesium3DTilesSelection::TileLoadResultAndRenderResources prepared;
  auto* model = std::get_if<CesiumGltf::Model>(&tile_load_result.contentKind);
  if (model != nullptr) {
    auto mesh = std::make_unique<TileMesh>(ExtractTileMesh(*model, transform));
    if (!mesh->ecef_positions.empty()) {
      prepared.pRenderResources = mesh.release();
    }
  }
  prepared.result = std::move(tile_load_result);
  return async_system.createResolvedFuture(std::move(prepared));
}

void* PrepareRendererResources::prepareInMainThread(
    Cesium3DTilesSelection::Tile& /*tile*/,
    void* load_thread_result) {
  return load_thread_result;
}

void PrepareRendererResources::free(Cesium3DTilesSelection::Tile& /*tile*/,
                                    void* load_thread_result,
                                    void* main_thread_result) noexcept {
  void* resources = main_thread_result != nullptr ? main_thread_result
                                                  : load_thread_result;
  delete static_cast<TileMesh*>(resources);
}

void* PrepareRendererResources::prepareRasterInLoadThread(
    CesiumImage::ImageAsset& /*image*/,
    const std::any& /*renderer_options*/) {
  return nullptr;
}

void* PrepareRendererResources::prepareRasterInMainThread(
    CesiumRasterOverlays::RasterOverlayTile& /*raster_tile*/,
    void* /*load_thread_result*/) {
  return nullptr;
}

void PrepareRendererResources::freeRaster(
    const CesiumRasterOverlays::RasterOverlayTile& /*raster_tile*/,
    void* /*load_thread_result*/,
    void* /*main_thread_result*/) noexcept {}

void PrepareRendererResources::attachRasterInMainThread(
    const Cesium3DTilesSelection::Tile& /*tile*/,
    int32_t /*overlay_texture_coordinate_id*/,
    const CesiumRasterOverlays::RasterOverlayTile& /*raster_tile*/,
    void* /*main_thread_renderer_resources*/,
    const glm::dvec2& /*translation*/,
    const glm::dvec2& /*scale*/) {}

void PrepareRendererResources::detachRasterInMainThread(
    const Cesium3DTilesSelection::Tile& /*tile*/,
    int32_t /*overlay_texture_coordinate_id*/,
    const CesiumRasterOverlays::RasterOverlayTile& /*raster_tile*/,
    void* /*main_thread_renderer_resources*/) noexcept {}

}  // namespace cesium_flycube
