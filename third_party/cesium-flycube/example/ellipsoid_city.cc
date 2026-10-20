// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "cesium_flycube/gpu/flycube_upload.h"
#include "cesium_flycube/gpu/scene_present.h"
#include "cesium_flycube/tiles/tileset_host.h"
#include "cesium_flycube/view/session.h"

#include <cmath>
#include <iostream>
#include <string>

namespace {

cesium_flycube::FlyCubeMesh ToFlyCube(const cesium_flycube::TileMesh& mesh,
                                      const cesium_flycube::Georeference& geo) {
  cesium_flycube::FlyCubeMesh out;
  out.xyz.reserve(mesh.ecef_positions.size() * 3);
  for (const glm::dvec3& ecef : mesh.ecef_positions) {
    const glm::dvec3 local = geo.EcefToLocal(ecef);
    out.xyz.push_back(static_cast<float>(local.x));
    out.xyz.push_back(static_cast<float>(local.y));
    out.xyz.push_back(static_cast<float>(local.z));
  }
  out.indices = mesh.indices;
  return out;
}

}  // namespace

// Ellipsoid tileset over Shanghai, two AdditionalCameras, height sample, then
// a DX12 upload of the selected tile meshes.
int main() {
  cesium_flycube::Session session;
  session.georeference().SetOriginDegrees(121.4737, 31.2304, 0.0);

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
  std::string reason;
  if (!session.CamerasShareOrigin(&reason)) {
    std::cerr << reason << "\n";
    return 1;
  }

  cesium_flycube::TilesetHost tileset;
  tileset.LoadEllipsoid();
  const auto views = session.BuildViewStates();

  cesium_flycube::TilesetFrame last;
  for (int frame = 0; frame < 40; ++frame) {
    last = tileset.Tick(views, 0.016f);
    if (last.vertex_count > 0 && frame > 2) {
      break;
    }
  }

  auto height_future = tileset.SampleHeightDegrees(121.4737, 31.2304);
  for (int frame = 0; frame < 40 && !height_future.isReady(); ++frame) {
    last = tileset.Tick(views, 0.016f);
  }
  const double height = std::move(height_future).waitInMainThread();

  std::vector<cesium_flycube::FlyCubeMesh> gpu_meshes;
  gpu_meshes.reserve(last.meshes.size());
  for (const cesium_flycube::TileMesh& mesh : last.meshes) {
    gpu_meshes.push_back(ToFlyCube(mesh, session.georeference()));
  }
  const cesium_flycube::FlyCubeUploadResult upload =
      cesium_flycube::UploadToFlyCube(gpu_meshes);

  std::cout << "tiles " << last.tiles_to_render << " meshes "
            << last.meshes_ready << " vertices " << last.vertex_count
            << " indices " << last.index_count << "\n";
  std::cout << "height_m " << height << "\n";
  std::cout << "flycube " << upload.adapter_name << " verts "
            << upload.vertex_count << " " << upload.message << "\n";
  for (const std::string& credit : last.credits) {
    std::cout << "credit " << credit << "\n";
  }

  if (last.tiles_to_render == 0 || last.vertex_count == 0) {
    std::cerr << "ellipsoid tileset produced no mesh\n";
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

  std::vector<cesium_flycube::PresentMesh> pictures;
  pictures.reserve(gpu_meshes.size());
  for (size_t i = 0; i < gpu_meshes.size(); ++i) {
    cesium_flycube::PresentMesh picture;
    picture.mesh = gpu_meshes[i];
    const float shade = 0.35f + 0.55f * static_cast<float>(i % 5) / 4.0f;
    picture.color[0] = (i % 3 == 0) ? shade : 0.25f;
    picture.color[1] = 0.45f + 0.4f * shade;
    picture.color[2] = (i % 3 == 2) ? shade : 0.3f;
    picture.color[3] = 1.0f;
    pictures.push_back(std::move(picture));
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
  const cesium_flycube::PresentResult shown = cesium_flycube::PresentScene(
      "ellipsoid_city", camera, pictures,
      std::string(CESIUM_FLYCUBE_CAPTURE_DIR) + "/ellipsoid_city.png");
  std::cout << "png " << shown.png_path << " pixels " << shown.drawn_pixels
            << " " << shown.message << "\n";
  if (!shown.ok) {
    std::cerr << "ellipsoid was not presented: " << shown.message << "\n";
    return 1;
  }
  std::cout << "ellipsoid_city ok\n";
  return 0;
}
