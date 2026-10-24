// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "cesium_flycube/gpu/draped_terrain.h"
#include "cesium_flycube/gpu/flycube_upload.h"
#include "cesium_flycube/gpu/scene_present.h"
#include "cesium_flycube/tiles/tileset_host.h"
#include "cesium_flycube/view/session.h"

#include <cmath>
#include <iostream>
#include <string>

namespace {

cesium_flycube::FlyCubeMesh ToFlyCube(const cesium_flycube::PresentMesh& mesh) {
  return mesh.mesh;
}

}  // namespace

// Ellipsoid tileset over Shanghai with debug colorize via CaptureDrapedEllipsoid,
// two AdditionalCameras, height sample, then PresentScene.
int main() {
  cesium_flycube::Session session;

  cesium_flycube::Camera overview;
  overview.viewport_size = glm::dvec2(1280.0, 720.0);
  overview.location = glm::dvec3(0.0, 0.0, 12000.0);
  overview.direction = glm::dvec3(0.0, 0.0, -1.0);
  overview.up = glm::dvec3(0.0, 1.0, 0.0);

  cesium_flycube::Camera detail = overview;
  detail.viewport_size = glm::dvec2(960.0, 540.0);
  detail.location = glm::dvec3(400.0, -200.0, 1800.0);
  detail.horizontal_fov_degrees = 40.0;
  session.cameras().additional_cameras() = {overview, detail};

  cesium_flycube::TilesetHost tileset;
  cesium_flycube::DrapedTerrainOptions opts;
  const cesium_flycube::DrapedTerrainResult terrain =
      cesium_flycube::CaptureDrapedEllipsoid(&session, &tileset, opts);
  std::string reason;
  if (!session.CamerasShareOrigin(&reason)) {
    std::cerr << reason << "\n";
    return 1;
  }
  if (!terrain.ok) {
    std::cerr << "ellipsoid terrain: " << terrain.message << "\n";
    return 1;
  }

  auto height_future = tileset.SampleHeightDegrees(121.4737, 31.2304);
  for (int frame = 0; frame < 40 && !height_future.isReady(); ++frame) {
    tileset.Tick(session.BuildViewStates(), 0.016f);
  }
  const double height = std::move(height_future).waitInMainThread();

  std::vector<cesium_flycube::FlyCubeMesh> gpu_meshes;
  gpu_meshes.reserve(terrain.pictures.size());
  for (const cesium_flycube::PresentMesh& picture : terrain.pictures) {
    gpu_meshes.push_back(ToFlyCube(picture));
  }
  const cesium_flycube::FlyCubeUploadResult upload =
      cesium_flycube::UploadToFlyCube(gpu_meshes);

  std::cout << "meshes " << terrain.pictures.size() << " vertices "
            << terrain.vertex_count << " draped " << terrain.draped << "\n";
  std::cout << "height_m " << height << "\n";
  std::cout << "flycube " << upload.adapter_name << " verts "
            << upload.vertex_count << " " << upload.message << "\n";

  if (terrain.vertex_count == 0) {
    std::cerr << "ellipsoid tileset produced no mesh\n";
    return 1;
  }
  if (!terrain.draped) {
    std::cerr << "ellipsoid tileset produced no draped raster\n";
    return 1;
  }
  if (std::abs(height) > 1.0) {
    std::cerr << "ellipsoid height should be near 0\n";
    return 1;
  }
  if (!upload.ok) {
    std::cerr << "flycube upload failed: " << upload.message << "\n";
    return 1;
  }

  cesium_flycube::PresentCamera camera;
  camera.eye[0] = 0.0f;
  camera.eye[1] = -1800.0f;
  camera.eye[2] = 4200.0f;
  camera.target[0] = 0.0f;
  camera.target[1] = 0.0f;
  camera.target[2] = 0.0f;
  camera.up[0] = 0.0f;
  camera.up[1] = 1.0f;
  camera.up[2] = 0.0f;
  camera.fov_y_radians = 0.9f;
  camera.skip_shade = true;
  const cesium_flycube::PresentResult shown = cesium_flycube::PresentScene(
      "ellipsoid_city", camera, terrain.pictures,
      std::string(CESIUM_FLYCUBE_CAPTURE_DIR) + "/ellipsoid_city.png",
      /*open_window=*/false, /*stamp_fps=*/false);
  std::cout << "png " << shown.png_path << " pixels " << shown.drawn_pixels
            << " " << shown.message << "\n";
  if (!shown.ok) {
    std::cerr << "ellipsoid was not presented: " << shown.message << "\n";
    return 1;
  }
  std::cout << "ellipsoid_city ok\n";
  return 0;
}
