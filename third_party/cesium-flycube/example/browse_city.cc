// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "cesium_flycube/gpu/draped_terrain.h"
#include "cesium_flycube/gpu/scene_present.h"
#include "cesium_flycube/tiles/tileset_host.h"
#include "cesium_flycube/view/camera_flight.h"
#include "cesium_flycube/view/session.h"

#include <glm/geometric.hpp>

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

namespace {

cesium_flycube::PresentCamera MakeCamera(const cesium_flycube::CameraPose& pose) {
  cesium_flycube::PresentCamera camera;
  camera.eye[0] = static_cast<float>(pose.eye[0]);
  camera.eye[1] = static_cast<float>(pose.eye[1]);
  camera.eye[2] = static_cast<float>(pose.eye[2]);
  camera.target[0] = static_cast<float>(pose.target[0]);
  camera.target[1] = static_cast<float>(pose.target[1]);
  camera.target[2] = static_cast<float>(pose.target[2]);
  camera.up[0] = 0;
  camera.up[1] = 0;
  camera.up[2] = 1;
  camera.fov_y_radians = 0.70f;
  camera.draw_edges = false;
  camera.skip_shade = true;
  return camera;
}

}  // namespace

// Shanghai draped ellipsoid browse. Left drag orbits, right pans, wheel zooms.
// --check proves draped meshes and the flight without a window.
// --present <seconds> opens the FlyCube window then exits (0 keeps open).
// --imagery prefers Esri World Imagery URL overlay (needs network).
int main(int argc, char** argv) {
  bool check = false;
  bool prefer_imagery = false;
  double present_seconds = 0;
  for (int i = 1; i < argc; ++i) {
    const std::string arg = argv[i];
    if (arg == "--check") {
      check = true;
    } else if (arg == "--imagery") {
      prefer_imagery = true;
    } else if (arg == "--present") {
      if (i + 1 < argc && argv[i + 1][0] != '-') {
        present_seconds = std::atof(argv[++i]);
      }
    }
  }

  cesium_flycube::Session session;
  cesium_flycube::Camera overview;
  overview.viewport_size = glm::dvec2(960.0, 540.0);
  overview.location = glm::dvec3(0.0, -12000.0, 28000.0);
  overview.direction = glm::normalize(glm::dvec3(0.0, 0.35, -1.0));
  overview.up = glm::dvec3(0.0, 0.0, 1.0);
  overview.horizontal_fov_degrees = 55.0;
  session.cameras().additional_cameras() = {overview};

  cesium_flycube::TilesetHost tileset;
  cesium_flycube::DrapedTerrainOptions opts;
  opts.prefer_imagery = prefer_imagery;
  opts.max_ticks = prefer_imagery ? 200 : 120;
  const cesium_flycube::DrapedTerrainResult terrain =
      cesium_flycube::CaptureDrapedEllipsoid(&session, &tileset, opts);
  std::cout << "terrain " << terrain.message << " draped " << terrain.draped
            << " verts " << terrain.vertex_count << " meshes "
            << terrain.pictures.size() << "\n";
  if (!terrain.ok) {
    std::cerr << "browse_city terrain failed\n";
    return 1;
  }

  cesium_flycube::OrbitController far_orbit;
  far_orbit.target[0] = 0;
  far_orbit.target[1] = 0;
  far_orbit.target[2] = 20;
  far_orbit.distance = 45000;
  far_orbit.yaw_radians = -0.9;
  far_orbit.pitch_radians = 0.55;
  cesium_flycube::OrbitController near_orbit = far_orbit;
  near_orbit.distance = 3500;
  near_orbit.yaw_radians = 0.4;
  near_orbit.pitch_radians = 0.42;

  const cesium_flycube::CameraPose start =
      cesium_flycube::PoseFromOrbit(far_orbit);
  const cesium_flycube::CameraPose end = cesium_flycube::PoseFromOrbit(near_orbit);
  cesium_flycube::Flight flight;
  cesium_flycube::BeginFlight(&flight, start, end, 4.0);
  flight.elapsed_seconds = 4.0;
  const cesium_flycube::CameraPose arrived = cesium_flycube::SampleFlight(flight);
  if (std::fabs(arrived.eye[0] - end.eye[0]) > 1.0 ||
      std::fabs(arrived.target[0] - end.target[0]) > 1.0) {
    std::cerr << "flight did not reach the near orbit\n";
    return 1;
  }

  const std::uint32_t far_pixels =
      cesium_flycube::CountDrawnPixels(MakeCamera(start), terrain.pictures);
  const std::uint32_t near_pixels =
      cesium_flycube::CountDrawnPixels(MakeCamera(end), terrain.pictures);
  std::cout << "pixels far " << far_pixels << " near " << near_pixels << "\n";
  if (far_pixels < 8000 || near_pixels < 8000 || far_pixels == near_pixels) {
    std::cerr << "browse views did not change between far and near\n";
    return 1;
  }
  if (check) {
    std::cout << "browse_city check ok\n";
    return 0;
  }

  cesium_flycube::BrowseOptions browse;
  browse.seconds = present_seconds;
  browse.auto_fly = true;
  browse.fly_target = MakeCamera(end);
  browse.show_fps = true;
  browse.prefer_gpu = true;
  const char* title = prefer_imagery ? "Shanghai ellipsoid + Esri imagery"
                                     : "Shanghai ellipsoid + debug colorize";
  const cesium_flycube::BrowseResult shown = cesium_flycube::PresentBrowse(
      title, MakeCamera(start), terrain.pictures, browse);
  std::cout << "browse backend " << shown.backend << " frames " << shown.frames
            << " flew " << shown.flew << " " << shown.message << "\n";
  if (!shown.ok) {
    std::cerr << "browse failed: " << shown.message << "\n";
    return 1;
  }
  std::cout << "browse_city ok\n";
  return 0;
}
