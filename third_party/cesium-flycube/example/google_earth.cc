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

cesium_flycube::PresentCamera MakeCamera(const cesium_flycube::CameraPose& pose,
                                         float fov_y = 0.65f) {
  cesium_flycube::PresentCamera camera;
  camera.eye[0] = static_cast<float>(pose.eye[0]);
  camera.eye[1] = static_cast<float>(pose.eye[1]);
  camera.eye[2] = static_cast<float>(pose.eye[2]);
  camera.target[0] = static_cast<float>(pose.target[0]);
  camera.target[1] = static_cast<float>(pose.target[1]);
  camera.target[2] = static_cast<float>(pose.target[2]);
  camera.up[0] = static_cast<float>(pose.up[0]);
  camera.up[1] = static_cast<float>(pose.up[1]);
  camera.up[2] = static_cast<float>(pose.up[2]);
  camera.fov_y_radians = fov_y;
  camera.draw_edges = false;
  camera.skip_shade = true;
  return camera;
}

}  // namespace

// Google Earth-class browse on the draped ellipsoid pipeline (not a hand-built
// sphere). Space → Shanghai flight, then orbit / pan / zoom.
// --check proves draped meshes and the flight without a window.
// --capture writes google_earth.png / google_earth_near.png and exits.
// --present <seconds> opens the FlyCube browse window.
// --imagery prefers Esri World Imagery URL overlay.
int main(int argc, char** argv) {
  bool check = false;
  bool want_browse = false;
  bool prefer_imagery = false;
  double present_seconds = 0;
  for (int i = 1; i < argc; ++i) {
    const std::string arg = argv[i];
    if (arg == "--check") {
      check = true;
    } else if (arg == "--capture") {
      // Default path already captures.
    } else if (arg == "--imagery") {
      prefer_imagery = true;
    } else if (arg == "--present") {
      want_browse = true;
      if (i + 1 < argc && argv[i + 1][0] != '-') {
        present_seconds = std::atof(argv[++i]);
      }
    }
  }

  cesium_flycube::Session session;
  cesium_flycube::Camera overview;
  overview.viewport_size = glm::dvec2(960.0, 540.0);
  overview.location = glm::dvec3(0.0, -2.0e5, 2.2e5);
  overview.direction = glm::normalize(glm::dvec3(0.0, 0.4, -1.0));
  overview.up = glm::dvec3(0.0, 0.0, 1.0);
  overview.horizontal_fov_degrees = 50.0;
  session.cameras().additional_cameras() = {overview};

  cesium_flycube::TilesetHost tileset;
  cesium_flycube::DrapedTerrainOptions opts;
  opts.prefer_imagery = prefer_imagery;
  opts.max_ticks = prefer_imagery ? 220 : 140;
  const cesium_flycube::DrapedTerrainResult terrain =
      cesium_flycube::CaptureDrapedEllipsoid(&session, &tileset, opts);
  std::cout << "terrain " << terrain.message << " draped " << terrain.draped
            << " verts " << terrain.vertex_count << "\n";
  if (!terrain.ok) {
    std::cerr << "google_earth terrain failed\n";
    return 1;
  }

  // Local ellipsoid tiles only cover the city region — "space" is a high
  // altitude over Shanghai, not a whole-Earth disk (that was the old hand mesh).
  cesium_flycube::OrbitController space;
  space.target[0] = 0;
  space.target[1] = 0;
  space.target[2] = 0;
  space.distance = 2.8e5;
  space.yaw_radians = 0.2;
  space.pitch_radians = 0.42;
  space.max_distance = 4.0e7;
  cesium_flycube::OrbitController close = space;
  close.distance = 2.8e4;
  close.yaw_radians = 0.55;
  close.pitch_radians = 0.48;

  const cesium_flycube::CameraPose far_pose =
      cesium_flycube::PoseFromOrbit(space);
  const cesium_flycube::CameraPose near_pose =
      cesium_flycube::PoseFromOrbit(close);
  cesium_flycube::Flight flight;
  cesium_flycube::BeginFlight(&flight, far_pose, near_pose, 6.0);
  flight.elapsed_seconds = 6.0;
  const cesium_flycube::CameraPose arrived =
      cesium_flycube::SampleFlight(flight);
  if (std::fabs(arrived.eye[0] - near_pose.eye[0]) > 50.0) {
    std::cerr << "radial flight did not reach Shanghai\n";
    return 1;
  }

  const std::uint32_t far_pixels = cesium_flycube::CountDrawnPixels(
      MakeCamera(far_pose), terrain.pictures);
  const std::uint32_t near_pixels = cesium_flycube::CountDrawnPixels(
      MakeCamera(near_pose), terrain.pictures);
  std::cout << "pixels space " << far_pixels << " shanghai " << near_pixels
            << "\n";
  if (far_pixels < 2000 || near_pixels < 8000 || far_pixels == near_pixels) {
    std::cerr << "google earth views did not change between space and near\n";
    return 1;
  }
  if (check) {
    std::cout << "google_earth check ok\n";
    return 0;
  }

  const std::string capture_dir = CESIUM_FLYCUBE_CAPTURE_DIR;
  const cesium_flycube::PresentResult still = cesium_flycube::PresentScene(
      "google_earth", MakeCamera(far_pose), terrain.pictures,
      capture_dir + "/google_earth.png", /*open_window=*/false,
      /*stamp_fps=*/false);
  std::cout << "png " << still.png_path << " pixels " << still.drawn_pixels
            << " " << still.message << "\n";
  if (!still.ok) {
    std::cerr << "google earth still failed: " << still.message << "\n";
    return 1;
  }
  const cesium_flycube::PresentResult near_still = cesium_flycube::PresentScene(
      "google_earth_near", MakeCamera(near_pose), terrain.pictures,
      capture_dir + "/google_earth_near.png", /*open_window=*/false,
      /*stamp_fps=*/false);
  std::cout << "png " << near_still.png_path << " pixels "
            << near_still.drawn_pixels << " " << near_still.message << "\n";
  if (!near_still.ok) {
    std::cerr << "google earth near still failed: " << near_still.message
              << "\n";
    return 1;
  }
  if (!want_browse) {
    std::cout << "google_earth capture ok\n";
    return 0;
  }

  cesium_flycube::BrowseOptions browse;
  browse.seconds = present_seconds;
  browse.auto_fly = true;
  browse.fly_target = MakeCamera(near_pose);
  browse.show_fps = true;
  browse.prefer_gpu = true;
  try {
    const cesium_flycube::BrowseResult shown = cesium_flycube::PresentBrowse(
        "Google Earth — draped ellipsoid", MakeCamera(far_pose),
        terrain.pictures, browse);
    std::cout << "browse backend " << shown.backend << " frames "
              << shown.frames << " " << shown.message << "\n";
    if (!shown.ok) {
      std::cout << "google_earth capture ok (browse skipped)\n";
      return 0;
    }
  } catch (const std::exception& error) {
    std::cerr << "google earth browse unavailable: " << error.what() << "\n";
    std::cout << "google_earth capture ok (browse skipped)\n";
    return 0;
  }
  std::cout << "google_earth ok\n";
  return 0;
}
