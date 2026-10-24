// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#pragma once

#include <Cesium3DTilesSelection/IPrepareRendererResources.h>

namespace cesium_flycube {

// Cesium for Unreal's render-resource hook, reduced to a CPU mesh FlyCube can
// upload. Raster overlays become RasterDrape entries on the TileMesh.
class PrepareRendererResources
    : public Cesium3DTilesSelection::IPrepareRendererResources {
 public:
  CesiumAsync::Future<Cesium3DTilesSelection::TileLoadResultAndRenderResources>
  prepareInLoadThread(const CesiumAsync::AsyncSystem& async_system,
                      Cesium3DTilesSelection::TileLoadResult&& tile_load_result,
                      const glm::dmat4& transform,
                      const std::any& renderer_options) override;

  void* prepareInMainThread(Cesium3DTilesSelection::Tile& tile,
                            void* load_thread_result) override;

  void free(Cesium3DTilesSelection::Tile& tile,
            void* load_thread_result,
            void* main_thread_result) noexcept override;

  void* prepareRasterInLoadThread(CesiumImage::ImageAsset& image,
                                  const std::any& renderer_options) override;

  void* prepareRasterInMainThread(
      CesiumRasterOverlays::RasterOverlayTile& raster_tile,
      void* load_thread_result) override;

  void freeRaster(const CesiumRasterOverlays::RasterOverlayTile& raster_tile,
                  void* load_thread_result,
                  void* main_thread_result) noexcept override;

  void attachRasterInMainThread(
      const Cesium3DTilesSelection::Tile& tile,
      int32_t overlay_texture_coordinate_id,
      const CesiumRasterOverlays::RasterOverlayTile& raster_tile,
      void* main_thread_renderer_resources,
      const glm::dvec2& translation,
      const glm::dvec2& scale) override;

  void detachRasterInMainThread(
      const Cesium3DTilesSelection::Tile& tile,
      int32_t overlay_texture_coordinate_id,
      const CesiumRasterOverlays::RasterOverlayTile& raster_tile,
      void* main_thread_renderer_resources) noexcept override;
};

}  // namespace cesium_flycube
