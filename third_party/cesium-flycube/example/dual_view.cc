// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "cesium_flycube/geo/globe_anchor.h"
#include "cesium_flycube/gpu/draped_terrain.h"
#include "cesium_flycube/gpu/scene_present.h"
#include "cesium_flycube/tiles/tileset_host.h"
#include "cesium_flycube/view/camera.h"
#include "cesium_flycube/view/session.h"

#include <CesiumGeospatial/Cartographic.h>

#include <cmath>
#include <iostream>
#include <string>
#include <vector>

namespace {

bool Expect(bool condition, const char* message) {
  if (!condition) {
    std::cerr << message << "\n";
  }
  return condition;
}

void AddDiamond(cesium_flycube::PresentMesh* mesh,
                double x,
                double y,
                float half,
                const float color[4]) {
  const float fx = static_cast<float>(x);
  const float fy = static_cast<float>(y);
  const float z = 40.0f;
  mesh->mesh.xyz = {fx, fy + half, z, fx + half, fy, z, fx, fy - half, z,
                     fx - half, fy, z};
  mesh->mesh.indices = {0, 1, 2, 0, 2, 3};
  mesh->color[0] = color[0];
  mesh->color[1] = color[1];
  mesh->color[2] = color[2];
  mesh->color[3] = color[3];
}

void AddPyramid(cesium_flycube::PresentMesh* mesh,
                double x,
                double y,
                float half,
                float height,
                const float color[4]) {
  const float fx = static_cast<float>(x);
  const float fy = static_cast<float>(y);
  const float z = 40.0f;
  mesh->mesh.xyz = {fx, fy + half, z, fx + half, fy, z, fx, fy - half, z,
                     fx - half, fy, z, fx, fy, z + height};
  mesh->mesh.indices = {0, 1, 4, 1, 2, 4, 2, 3, 4, 3, 0, 4};
  mesh->color[0] = color[0];
  mesh->color[1] = color[1];
  mesh->color[2] = color[2];
  mesh->color[3] = color[3];
}

}  // namespace

// Two AdditionalCameras over draped ellipsoid terrain, plus anchors.
// A second pair on another continent reports origin_conflict.
int main() {
  cesium_flycube::Session session;
  cesium_flycube::GlobeAnchor bund;
  bund.SetDegrees(121.4998, 31.2397, 0.0);
  cesium_flycube::GlobeAnchor east;
  east.SetDegrees(121.4937, 31.2304, 20.0);

  cesium_flycube::Camera overview;
  overview.viewport_size = glm::dvec2(1280.0, 720.0);
  overview.location = glm::dvec3(0.0, 0.0, 8000.0);
  overview.direction = glm::dvec3(0.0, 0.0, -1.0);
  overview.up = glm::dvec3(0.0, 1.0, 0.0);
  overview.horizontal_fov_degrees = 60.0;

  session.cameras().additional_cameras() = {overview};
  cesium_flycube::TilesetHost tileset;
  cesium_flycube::DrapedTerrainOptions terrain_opts;
  const cesium_flycube::DrapedTerrainResult terrain =
      cesium_flycube::CaptureDrapedEllipsoid(&session, &tileset, terrain_opts);
  if (!terrain.ok) {
    std::cerr << "dual_view terrain: " << terrain.message << "\n";
    return 1;
  }

  const glm::dvec3 bund_local = bund.LocalPosition(session.georeference());
  const glm::dvec3 east_local = east.LocalPosition(session.georeference());

  cesium_flycube::Camera detail = overview;
  detail.viewport_size = glm::dvec2(960.0, 540.0);
  detail.location = glm::dvec3(east_local.x, east_local.y, 2400.0);
  detail.horizontal_fov_degrees = 45.0;
  session.cameras().additional_cameras() = {overview, detail};

  std::string reason;
  const bool shared = session.CamerasShareOrigin(&reason);
  const auto views = session.BuildViewStates();

  std::cout << "origin_local_bund_m " << bund_local.x << " " << bund_local.y
            << " " << bund_local.z << "\n";
  std::cout << "anchors " << east_local.x << " " << east_local.y << "\n";
  std::cout << "views " << views.size() << " shared " << shared << " draped "
            << terrain.draped << " verts " << terrain.vertex_count << "\n";

  bool ok = true;
  ok = Expect(shared, "nearby cameras must share an origin") && ok;
  ok = Expect(views.size() == 2, "expected two view states") && ok;
  ok = Expect(bund_local.x > 1000.0 && std::abs(bund_local.y) < 2000.0,
              "bund should sit east of the origin") &&
       ok;
  ok = Expect(east_local.x > 500.0 && std::abs(east_local.z - 20.0) < 1.0,
              "east anchor should be east and 20 m up") &&
       ok;

  const double origin_longitude = session.georeference().origin().longitude;
  cesium_flycube::Camera far_view = overview;
  far_view.location = session.georeference().EcefToLocal(
      session.georeference().ellipsoid().cartographicToCartesian(
          CesiumGeospatial::Cartographic::fromDegrees(-74.006, 40.7128, 8000.0)));
  session.cameras().additional_cameras() = {overview, far_view};
  std::string conflict;
  const bool still_shared = session.CamerasShareOrigin(&conflict);
  ok = Expect(!still_shared && conflict == "origin_conflict",
              "distant cameras must report origin_conflict") &&
       ok;
  ok = Expect(session.georeference().origin().longitude == origin_longitude,
              "origin_conflict must not move the georeference") &&
       ok;

  const float amber[4] = {0.95f, 0.55f, 0.15f, 1};
  const float green[4] = {0.25f, 0.8f, 0.35f, 1};
  const float blue[4] = {0.25f, 0.55f, 0.95f, 1};
  cesium_flycube::PresentMesh origin_mark;
  cesium_flycube::PresentMesh bund_mark;
  cesium_flycube::PresentMesh east_mark;
  AddDiamond(&origin_mark, 0.0, 0.0, 350.0f, amber);
  AddDiamond(&bund_mark, bund_local.x, bund_local.y, 280.0f, green);
  AddPyramid(&east_mark, east_local.x, east_local.y, 180.0f, 220.0f, blue);

  std::vector<cesium_flycube::PresentMesh> pictures = terrain.pictures;
  pictures.push_back(std::move(origin_mark));
  pictures.push_back(std::move(bund_mark));
  pictures.push_back(std::move(east_mark));

  const auto look_down = [](cesium_flycube::PresentCamera* camera, double x,
                            double y, double z, float fov_y) {
    camera->eye[0] = static_cast<float>(x);
    camera->eye[1] = static_cast<float>(y);
    camera->eye[2] = static_cast<float>(z);
    camera->target[0] = static_cast<float>(x);
    camera->target[1] = static_cast<float>(y);
    camera->target[2] = 0.0f;
    camera->up[0] = 0.0f;
    camera->up[1] = 1.0f;
    camera->up[2] = 0.0f;
    camera->fov_y_radians = fov_y;
    camera->draw_edges = false;
    camera->skip_shade = true;
  };
  cesium_flycube::PresentCamera overview_picture;
  cesium_flycube::PresentCamera detail_picture;
  look_down(&overview_picture, 0.0, 0.0, 8000.0, 0.627f);
  look_down(&detail_picture, east_local.x, east_local.y, 2400.0, 0.457f);

  const cesium_flycube::PresentResult shown = cesium_flycube::PresentSplitScene(
      "dual_view", overview_picture, detail_picture, pictures,
      std::string(CESIUM_FLYCUBE_CAPTURE_DIR) + "/dual_view.png");
  std::cout << "png " << shown.png_path << " pixels " << shown.drawn_pixels
            << " " << shown.message << "\n";
  ok = Expect(shown.ok, "dual_view did not present a frame") && ok;

  if (!ok) {
    return 1;
  }
  std::cout << "dual_view ok\n";
  return 0;
}
