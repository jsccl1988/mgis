// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "cesium_flycube/tiles/raster_overlay.h"

namespace cesium_flycube {
namespace {

template <typename T, typename... Args>
CesiumUtility::IntrusivePointer<const CesiumRasterOverlays::RasterOverlay>
Make(Args&&... args) {
  return CesiumUtility::IntrusivePointer<const CesiumRasterOverlays::RasterOverlay>(
      new T(std::forward<Args>(args)...));
}

}  // namespace

CesiumUtility::IntrusivePointer<const CesiumRasterOverlays::RasterOverlay>
MakeDebugColorizeTilesOverlay(const std::string& name) {
  return Make<CesiumRasterOverlays::DebugColorizeTilesRasterOverlay>(name);
}

CesiumUtility::IntrusivePointer<const CesiumRasterOverlays::RasterOverlay>
MakeIonRasterOverlay(const std::string& name,
                     std::int64_t asset_id,
                     const std::string& access_token) {
  return Make<CesiumRasterOverlays::IonRasterOverlay>(
      name, asset_id, access_token);
}

CesiumUtility::IntrusivePointer<const CesiumRasterOverlays::RasterOverlay>
MakeUrlTemplateOverlay(const std::string& name, const std::string& url) {
  return Make<CesiumRasterOverlays::UrlTemplateRasterOverlay>(name, url);
}

CesiumUtility::IntrusivePointer<const CesiumRasterOverlays::RasterOverlay>
MakeTileMapServiceOverlay(const std::string& name, const std::string& url) {
  return Make<CesiumRasterOverlays::TileMapServiceRasterOverlay>(name, url);
}

CesiumUtility::IntrusivePointer<const CesiumRasterOverlays::RasterOverlay>
MakeWebMapServiceOverlay(const std::string& name,
                         const std::string& url,
                         const std::string& layers) {
  CesiumRasterOverlays::WebMapServiceRasterOverlayOptions options;
  options.layers = layers;
  return Make<CesiumRasterOverlays::WebMapServiceRasterOverlay>(
      name, url, std::vector<CesiumAsync::IAssetAccessor::THeader>{}, options);
}

CesiumUtility::IntrusivePointer<const CesiumRasterOverlays::RasterOverlay>
MakeWebMapTileServiceOverlay(const std::string& name,
                             const std::string& url,
                             const std::string& layer) {
  CesiumRasterOverlays::WebMapTileServiceRasterOverlayOptions options;
  options.layer = layer;
  return Make<CesiumRasterOverlays::WebMapTileServiceRasterOverlay>(
      name, url, std::vector<CesiumAsync::IAssetAccessor::THeader>{}, options);
}

CesiumUtility::IntrusivePointer<const CesiumRasterOverlays::RasterOverlay>
MakeBingMapsOverlay(const std::string& name,
                    const std::string& key,
                    const std::string& style) {
  return Make<CesiumRasterOverlays::BingMapsRasterOverlay>(
      name, "https://dev.virtualearth.net", key, style);
}

CesiumUtility::IntrusivePointer<const CesiumRasterOverlays::RasterOverlay>
MakeAzureMapsOverlay(
    const std::string& name,
    const CesiumRasterOverlays::AzureMapsSessionParameters& session) {
  return Make<CesiumRasterOverlays::AzureMapsRasterOverlay>(name, session);
}

CesiumUtility::IntrusivePointer<const CesiumRasterOverlays::RasterOverlay>
MakeGoogleMapTilesOverlay(
    const std::string& name,
    const CesiumRasterOverlays::GoogleMapTilesNewSessionParameters& session) {
  return Make<CesiumRasterOverlays::GoogleMapTilesRasterOverlay>(name, session);
}

CesiumUtility::IntrusivePointer<const CesiumRasterOverlays::RasterOverlay>
MakePolygonOverlay(
    const std::string& name,
    const std::vector<CesiumGeospatial::CartographicPolygon>& polygons,
    bool invert_selection,
    const CesiumGeospatial::Projection& projection) {
  return Make<CesiumRasterOverlays::RasterizedPolygonsOverlay>(
      name,
      polygons,
      invert_selection,
      CesiumGeospatial::Ellipsoid::WGS84,
      projection);
}

CesiumUtility::IntrusivePointer<const CesiumRasterOverlays::RasterOverlay>
MakeVectorTilesOverlay(const std::string& name, const std::string& url) {
  return Make<CesiumVectorOverlays::VectorTilesRasterOverlay>(name, url);
}

CesiumUtility::IntrusivePointer<const CesiumRasterOverlays::RasterOverlay>
MakeVectorTilesIonOverlay(const std::string& name,
                          std::int64_t asset_id,
                          const std::string& access_token) {
  return Make<CesiumVectorOverlays::VectorTilesRasterOverlay>(
      name, asset_id, access_token);
}

}  // namespace cesium_flycube
