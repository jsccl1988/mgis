// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#pragma once

#include <CesiumAsync/IAssetAccessor.h>
#include <CesiumGeospatial/CartographicPolygon.h>
#include <CesiumGeospatial/Ellipsoid.h>
#include <CesiumGeospatial/Projection.h>
#include <CesiumRasterOverlays/AzureMapsRasterOverlay.h>
#include <CesiumRasterOverlays/BingMapsRasterOverlay.h>
#include <CesiumRasterOverlays/DebugColorizeTilesRasterOverlay.h>
#include <CesiumRasterOverlays/GoogleMapTilesRasterOverlay.h>
#include <CesiumRasterOverlays/IonRasterOverlay.h>
#include <CesiumRasterOverlays/RasterOverlay.h>
#include <CesiumRasterOverlays/RasterizedPolygonsOverlay.h>
#include <CesiumRasterOverlays/TileMapServiceRasterOverlay.h>
#include <CesiumRasterOverlays/UrlTemplateRasterOverlay.h>
#include <CesiumRasterOverlays/WebMapServiceRasterOverlay.h>
#include <CesiumRasterOverlays/WebMapTileServiceRasterOverlay.h>
#include <CesiumUtility/IntrusivePointer.h>
#include <CesiumVectorOverlays/VectorTilesRasterOverlay.h>

#include <string>
#include <vector>

namespace cesium_flycube {

// Factories for the raster overlays Cesium for Unreal exposes. Each one is a
// cesium-native overlay the tileset can add. FlyCube does not create Unreal
// materials for them.

CesiumUtility::IntrusivePointer<const CesiumRasterOverlays::RasterOverlay>
MakeDebugColorizeTilesOverlay(const std::string& name);

CesiumUtility::IntrusivePointer<const CesiumRasterOverlays::RasterOverlay>
MakeIonRasterOverlay(const std::string& name,
                     std::int64_t asset_id,
                     const std::string& access_token);

CesiumUtility::IntrusivePointer<const CesiumRasterOverlays::RasterOverlay>
MakeUrlTemplateOverlay(const std::string& name, const std::string& url);

CesiumUtility::IntrusivePointer<const CesiumRasterOverlays::RasterOverlay>
MakeTileMapServiceOverlay(const std::string& name, const std::string& url);

CesiumUtility::IntrusivePointer<const CesiumRasterOverlays::RasterOverlay>
MakeWebMapServiceOverlay(const std::string& name,
                         const std::string& url,
                         const std::string& layers);

CesiumUtility::IntrusivePointer<const CesiumRasterOverlays::RasterOverlay>
MakeWebMapTileServiceOverlay(const std::string& name,
                             const std::string& url,
                             const std::string& layer);

CesiumUtility::IntrusivePointer<const CesiumRasterOverlays::RasterOverlay>
MakeBingMapsOverlay(const std::string& name,
                    const std::string& key,
                    const std::string& style =
                        CesiumRasterOverlays::BingMapsStyle::AERIAL);

CesiumUtility::IntrusivePointer<const CesiumRasterOverlays::RasterOverlay>
MakeAzureMapsOverlay(
    const std::string& name,
    const CesiumRasterOverlays::AzureMapsSessionParameters& session);

CesiumUtility::IntrusivePointer<const CesiumRasterOverlays::RasterOverlay>
MakeGoogleMapTilesOverlay(
    const std::string& name,
    const CesiumRasterOverlays::GoogleMapTilesNewSessionParameters& session);

CesiumUtility::IntrusivePointer<const CesiumRasterOverlays::RasterOverlay>
MakePolygonOverlay(
    const std::string& name,
    const std::vector<CesiumGeospatial::CartographicPolygon>& polygons,
    bool invert_selection,
    const CesiumGeospatial::Projection& projection);

CesiumUtility::IntrusivePointer<const CesiumRasterOverlays::RasterOverlay>
MakeVectorTilesOverlay(const std::string& name, const std::string& url);

CesiumUtility::IntrusivePointer<const CesiumRasterOverlays::RasterOverlay>
MakeVectorTilesIonOverlay(const std::string& name,
                          std::int64_t asset_id,
                          const std::string& access_token);

}  // namespace cesium_flycube
