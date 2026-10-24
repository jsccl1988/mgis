// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "cesium_flycube/gpu/draped_terrain.h"

#include "cesium_flycube/gpu/frame_present.h"
#include "cesium_flycube/tiles/raster_overlay.h"
#include "cesium_flycube/view/camera.h"

#include <glm/geometric.hpp>

namespace cesium_flycube {
namespace {

constexpr char kEsriWorldImageryUrl[] =
    "https://services.arcgisonline.com/ArcGIS/rest/services/World_Imagery/"
    "MapServer/tile/{z}/{y}/{x}";

}  // namespace

DrapedTerrainResult CaptureDrapedEllipsoid(Session* session,
                                           TilesetHost* tileset,
                                           const DrapedTerrainOptions& options) {
  DrapedTerrainResult result;
  if (session == nullptr || tileset == nullptr) {
    result.message = "session or tileset is null";
    return result;
  }

  session->georeference().SetOriginDegrees(
      options.longitude_degrees, options.latitude_degrees, options.height_meters);

  if (session->cameras().additional_cameras().empty()) {
    Camera camera;
    camera.viewport_size = glm::dvec2(960.0, 540.0);
    camera.location = glm::dvec3(0.0, -800.0, 4200.0);
    camera.direction = glm::normalize(glm::dvec3(0.0, 0.2, -1.0));
    camera.up = glm::dvec3(0.0, 0.0, 1.0);
    camera.horizontal_fov_degrees = 55.0;
    session->cameras().additional_cameras() = {camera};
  }

  tileset->settings().maximum_screen_space_error = 8.0;
  tileset->settings().maximum_simultaneous_tile_loads = 64;
  tileset->settings().loading_descendant_limit = 64;
  tileset->LoadEllipsoid();
  if (options.prefer_imagery) {
    const char* url = options.imagery_url_template.empty()
                          ? kEsriWorldImageryUrl
                          : options.imagery_url_template.c_str();
    tileset->AddRasterOverlay(MakeUrlTemplateOverlay("imagery", url));
  } else {
    tileset->AddRasterOverlay(MakeDebugColorizeTilesOverlay("districts"));
  }

  const auto views = session->BuildViewStates();
  TilesetFrame frame;
  for (int i = 0; i < options.max_ticks; ++i) {
    frame = tileset->Tick(views, options.tick_delta_seconds);
    result.draped = false;
    for (const TileMesh& tile : frame.meshes) {
      if (!tile.rasters.empty()) {
        result.draped = true;
        break;
      }
    }
    if (frame.vertex_count > 0 && (!options.require_drape || result.draped) &&
        i > 2) {
      break;
    }
  }

  result.vertex_count = frame.vertex_count;
  if (frame.vertex_count == 0) {
    result.message = "ellipsoid produced no mesh";
    return result;
  }
  if (options.require_drape && !result.draped) {
    result.message = "ellipsoid produced no draped raster";
    return result;
  }

  result.pictures = FrameToPresent(frame.meshes, session->georeference());
  result.ok = !result.pictures.empty();
  result.message = result.ok ? "draped" : "FrameToPresent empty";
  return result;
}

}  // namespace cesium_flycube
