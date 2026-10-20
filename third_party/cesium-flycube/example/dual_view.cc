// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "cesium_flycube/geo/globe_anchor.h"
#include "cesium_flycube/gpu/scene_present.h"
#include "cesium_flycube/view/camera.h"
#include "cesium_flycube/view/session.h"

#include <CesiumGeospatial/Cartographic.h>

#include <cmath>
#include <iostream>
#include <string>

namespace {

bool Expect(bool condition, const char* message) {
  if (!condition) {
    std::cerr << message << "\n";
  }
  return condition;
}

}  // namespace

// Two AdditionalCameras over one city, plus anchors. A second pair placed on
// another continent reports origin_conflict and leaves the origin alone.
int main() {
  cesium_flycube::Session session;
  session.georeference().SetOriginDegrees(121.4737, 31.2304, 0.0);

  cesium_flycube::GlobeAnchor bund;
  bund.SetDegrees(121.4998, 31.2397, 0.0);
  cesium_flycube::GlobeAnchor east;
  east.SetDegrees(121.4937, 31.2304, 20.0);

  const glm::dvec3 bund_local = bund.LocalPosition(session.georeference());
  const glm::dvec3 east_local = east.LocalPosition(session.georeference());

  cesium_flycube::Camera overview;
  overview.viewport_size = glm::dvec2(1280.0, 720.0);
  overview.location = glm::dvec3(0.0, 0.0, 8000.0);
  overview.direction = glm::dvec3(0.0, 0.0, -1.0);
  overview.up = glm::dvec3(0.0, 1.0, 0.0);
  overview.horizontal_fov_degrees = 60.0;

  cesium_flycube::Camera detail = overview;
  detail.viewport_size = glm::dvec2(960.0, 540.0);
  detail.location = glm::dvec3(east_local.x, east_local.y, 600.0);
  detail.horizontal_fov_degrees = 45.0;

  session.cameras().use_player_cameras = false;
  session.cameras().use_editor_cameras = false;
  session.cameras().use_scene_captures = false;
  session.cameras().additional_cameras() = {overview, detail};

  std::string reason;
  const bool shared = session.CamerasShareOrigin(&reason);
  const auto views = session.BuildViewStates();

  std::cout << "origin_local_bund_m " << bund_local.x << " " << bund_local.y
            << " " << bund_local.z << "\n";
  std::cout << "anchors " << east_local.x << " " << east_local.y << "\n";
  std::cout << "views " << views.size() << " shared " << shared << "\n";

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

  cesium_flycube::PresentMesh origin;
  cesium_flycube::PresentMesh bund_mark;
  cesium_flycube::PresentMesh east_mark;
  const auto add_diamond = [](cesium_flycube::PresentMesh* mesh, double x, double y, float half,
                             const float color[4]) {
    const std::uint32_t base = static_cast<std::uint32_t>(mesh->mesh.xyz.size() / 3);
    const float fx = static_cast<float>(x);
    const float fy = static_cast<float>(y);
    mesh->mesh.xyz.insert(mesh->mesh.xyz.end(),
                          {fx, fy + half, 30.0f, fx - half, fy - half, 30.0f, fx + half, fy - half, 30.0f});
    mesh->mesh.indices.insert(mesh->mesh.indices.end(), {base, base + 1, base + 2});
    mesh->color[0] = color[0];
    mesh->color[1] = color[1];
    mesh->color[2] = color[2];
    mesh->color[3] = color[3];
  };
  const float amber[4] = {0.95f, 0.55f, 0.15f, 1};
  const float green[4] = {0.25f, 0.8f, 0.35f, 1};
  const float blue[4] = {0.25f, 0.55f, 0.95f, 1};
  add_diamond(&origin, 0.0, 0.0, 350.0f, amber);
  add_diamond(&bund_mark, bund_local.x, bund_local.y, 280.0f, green);
  add_diamond(&east_mark, east_local.x, east_local.y, 280.0f, blue);

  cesium_flycube::PresentCamera picture;
  picture.eye[0] = 1200.0f;
  picture.eye[1] = 400.0f;
  picture.eye[2] = 9000.0f;
  picture.target[0] = 1200.0f;
  picture.target[1] = 400.0f;
  picture.target[2] = 0.0f;
  picture.up[0] = 0.0f;
  picture.up[1] = 1.0f;
  picture.up[2] = 0.0f;
  picture.fov_y_radians = 0.9f;
  const cesium_flycube::PresentResult shown = cesium_flycube::PresentScene(
      "dual_view", picture, {origin, bund_mark, east_mark},
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
