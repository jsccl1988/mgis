// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "cesium_flycube/geo/globe_anchor.h"
#include "cesium_flycube/geo/origin_shift.h"
#include "cesium_flycube/tiles/feature_table.h"
#include "cesium_flycube/tiles/raster_image.h"
#include "cesium_flycube/tiles/raster_overlay.h"
#include "cesium_flycube/tiles/tile_mesh.h"
#include "cesium_flycube/tiles/tileset_host.h"
#include "cesium_flycube/tiles/tileset_settings.h"
#include "cesium_flycube/view/session.h"

#include <CesiumGeospatial/Cartographic.h>
#include <CesiumGltf/Accessor.h>
#include <CesiumGltf/Buffer.h>
#include <CesiumGltf/BufferView.h>
#include <CesiumGltf/Mesh.h>
#include <CesiumGltf/MeshPrimitive.h>
#include <CesiumGltf/Model.h>

#include <glm/mat4x4.hpp>

#include <cstring>
#include <iostream>
#include <string>

namespace {

bool Expect(bool condition, const char* message) {
  if (!condition) {
    std::cerr << message << "\n";
  }
  return condition;
}

CesiumGltf::Model MakeFeatureTriangle() {
  CesiumGltf::Model model;
  const float positions[9] = {0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 1.f, 0.f};
  const std::uint16_t features[3] = {7, 7, 8};

  CesiumGltf::Buffer buffer;
  buffer.cesium.data.resize(sizeof(positions) + sizeof(features));
  std::memcpy(buffer.cesium.data.data(), positions, sizeof(positions));
  std::memcpy(buffer.cesium.data.data() + sizeof(positions),
              features,
              sizeof(features));
  model.buffers.push_back(std::move(buffer));

  CesiumGltf::BufferView position_view;
  position_view.buffer = 0;
  position_view.byteOffset = 0;
  position_view.byteLength = static_cast<int64_t>(sizeof(positions));
  model.bufferViews.push_back(position_view);

  CesiumGltf::BufferView feature_view;
  feature_view.buffer = 0;
  feature_view.byteOffset = static_cast<int64_t>(sizeof(positions));
  feature_view.byteLength = static_cast<int64_t>(sizeof(features));
  model.bufferViews.push_back(feature_view);

  CesiumGltf::Accessor position_accessor;
  position_accessor.bufferView = 0;
  position_accessor.componentType = CesiumGltf::Accessor::ComponentType::FLOAT;
  position_accessor.count = 3;
  position_accessor.type = CesiumGltf::Accessor::Type::VEC3;
  model.accessors.push_back(std::move(position_accessor));

  CesiumGltf::Accessor feature_accessor;
  feature_accessor.bufferView = 1;
  feature_accessor.componentType =
      CesiumGltf::Accessor::ComponentType::UNSIGNED_SHORT;
  feature_accessor.count = 3;
  feature_accessor.type = CesiumGltf::Accessor::Type::SCALAR;
  model.accessors.push_back(std::move(feature_accessor));

  CesiumGltf::MeshPrimitive primitive;
  primitive.attributes["POSITION"] = 0;
  primitive.attributes["_FEATURE_ID_0"] = 1;
  CesiumGltf::Mesh mesh;
  mesh.primitives.push_back(std::move(primitive));
  model.meshes.push_back(std::move(mesh));
  return model;
}

bool CheckOriginShift() {
  cesium_flycube::Session session;
  session.georeference().SetOriginDegrees(0.0, 0.0, 0.0);
  cesium_flycube::GlobeAnchor paris;
  paris.SetDegrees(2.3522, 48.8566, 35.0);

  session.origin_shift().mode = cesium_flycube::OriginShiftMode::kDisabled;
  if (!Expect(!session.ShiftOrigin(paris), "disabled shift must not move")) {
    return false;
  }
  if (!Expect(session.georeference().origin().longitude == 0.0,
              "disabled origin changed")) {
    return false;
  }

  session.origin_shift().mode =
      cesium_flycube::OriginShiftMode::kSwitchSubLevelsOnly;
  if (!Expect(!session.ShiftOrigin(paris),
              "sub-level mode has no levels to enter")) {
    return false;
  }

  session.origin_shift().mode =
      cesium_flycube::OriginShiftMode::kChangeGeoreference;
  session.origin_shift().distance_meters = 1000.0;
  if (!Expect(session.ShiftOrigin(paris), "far anchor must shift origin")) {
    return false;
  }
  const CesiumGeospatial::Cartographic expected =
      CesiumGeospatial::Cartographic::fromDegrees(2.3522, 48.8566, 35.0);
  if (!Expect(session.georeference().origin().longitude == expected.longitude &&
                  session.georeference().origin().latitude == expected.latitude,
              "origin did not follow the anchor")) {
    return false;
  }

  cesium_flycube::GlobeAnchor nearby = paris;
  if (!Expect(!session.ShiftOrigin(nearby),
              "anchor on the origin must stay put")) {
    return false;
  }
  return true;
}

bool CheckSettings() {
  cesium_flycube::TilesetSettings settings;
  settings.maximum_screen_space_error = 8.0;
  settings.forbid_holes = true;
  settings.maximum_cached_bytes = 32;
  Cesium3DTilesSelection::TilesetOptions options;
  cesium_flycube::ApplyTilesetSettings(settings, &options);
  return Expect(options.maximumScreenSpaceError == 8.0 && options.forbidHoles &&
                    options.maximumCachedBytes == 32,
                "tileset settings did not copy");
}

bool CheckFeatures() {
  const CesiumGltf::Model model = MakeFeatureTriangle();
  const cesium_flycube::TileMesh mesh =
      cesium_flycube::ExtractTileMesh(model, glm::dmat4(1.0));
  return Expect(mesh.ecef_positions.size() == 3 &&
                    cesium_flycube::FeatureIdAt(mesh, 0) == 7 &&
                    cesium_flycube::FeatureIdAt(mesh, 2) == 8 &&
                    cesium_flycube::FeatureIdAt(mesh, 9) == -1,
                "feature ids were not read from _FEATURE_ID_0");
}

bool CheckRasterSample() {
  cesium_flycube::RasterDrape drape;
  drape.image = std::make_shared<cesium_flycube::RasterImage>();
  drape.image->width = 2;
  drape.image->height = 1;
  drape.image->channels = 4;
  drape.image->bytes_per_channel = 1;
  drape.image->pixels = {
      std::byte{255}, std::byte{0}, std::byte{0}, std::byte{255},
      std::byte{0},   std::byte{255}, std::byte{0}, std::byte{255}};
  std::uint8_t rgba[4] = {};
  if (!Expect(cesium_flycube::SampleRaster(drape, glm::vec2(0.1f, 0.5f), rgba) &&
                  rgba[0] == 255 && rgba[1] == 0,
              "left raster texel")) {
    return false;
  }
  if (!Expect(cesium_flycube::SampleRaster(drape, glm::vec2(0.9f, 0.5f), rgba) &&
                  rgba[1] == 255 && rgba[0] == 0,
              "right raster texel")) {
    return false;
  }
  const auto overlay = cesium_flycube::MakeDebugColorizeTilesOverlay("tiles");
  const auto ion = cesium_flycube::MakeIonRasterOverlay("ion", 2, "token");
  const auto tms = cesium_flycube::MakeTileMapServiceOverlay(
      "tms", "https://example.test/tms");
  return Expect(overlay && ion && tms, "overlay factory returned null");
}

bool CheckTilesetHooks() {
  cesium_flycube::Session session;
  session.georeference().SetOriginDegrees(121.4737, 31.2304, 0.0);
  cesium_flycube::Camera camera;
  camera.location = glm::dvec3(0.0, 0.0, 8000.0);
  session.cameras().additional_cameras() = {camera};

  cesium_flycube::TilesetHost tileset;
  tileset.settings().maximum_screen_space_error = 32.0;
  tileset.LoadEllipsoid();
  tileset.AddRasterOverlay(cesium_flycube::MakeDebugColorizeTilesOverlay("tiles"));
  const auto views = session.BuildViewStates();
  cesium_flycube::TilesetFrame frame;
  for (int i = 0; i < 20; ++i) {
    frame = tileset.Tick(views, 0.016f);
    if (frame.vertex_count > 0) {
      break;
    }
  }
  const float progress = tileset.LoadProgress();
  return Expect(progress >= 0.0f && progress <= 100.0f &&
                    frame.vertex_count > 0 && tileset.last_load_error().empty(),
                "ellipsoid tileset with overlay produced no mesh");
}

}  // namespace

int main() {
  if (!CheckOriginShift() || !CheckSettings() || !CheckFeatures() ||
      !CheckRasterSample() || !CheckTilesetHooks()) {
    return 1;
  }
  std::cout << "align_native ok\n";
  return 0;
}
