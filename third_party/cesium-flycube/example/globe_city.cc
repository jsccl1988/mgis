// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "cesium_flycube/gpu/frame_present.h"
#include "cesium_flycube/gpu/scene_present.h"
#include "cesium_flycube/tiles/raster_overlay.h"
#include "cesium_flycube/tiles/tileset_host.h"
#include "cesium_flycube/view/session.h"

#include <glm/geometric.hpp>

#include <cmath>
#include <iostream>
#include <string>
#include <vector>

namespace {

float HeightAt(int column, int row) {
  const int n = (column * 17 + row * 31) % 11;
  if (n < 2) {
    return 28.0f;
  }
  if (n < 6) {
    return 55.0f + static_cast<float>(n) * 8.0f;
  }
  return 110.0f + static_cast<float>((column * row) % 7) * 18.0f;
}

void ColorForHeight(float height, float rgba[4]) {
  if (height < 40.0f) {
    rgba[0] = 0.55f;
    rgba[1] = 0.56f;
    rgba[2] = 0.58f;
  } else if (height < 120.0f) {
    rgba[0] = 0.45f;
    rgba[1] = 0.62f;
    rgba[2] = 0.72f;
  } else {
    rgba[0] = 0.82f;
    rgba[1] = 0.72f;
    rgba[2] = 0.55f;
  }
  rgba[3] = 1.0f;
}

void AddQuad(std::vector<cesium_flycube::PresentMesh>* meshes,
             float x0,
             float y0,
             float z0,
             float x1,
             float y1,
             float z1,
             float x2,
             float y2,
             float z2,
             float x3,
             float y3,
             float z3,
             const float rgba[4]) {
  cesium_flycube::PresentMesh mesh;
  mesh.mesh.xyz = {x0, y0, z0, x1, y1, z1, x2, y2, z2, x3, y3, z3};
  mesh.mesh.indices = {0, 1, 2, 0, 2, 3};
  mesh.color[0] = rgba[0];
  mesh.color[1] = rgba[1];
  mesh.color[2] = rgba[2];
  mesh.color[3] = rgba[3];
  meshes->push_back(std::move(mesh));
}

void AddBox(std::vector<cesium_flycube::PresentMesh>* meshes,
            float x,
            float y,
            float z,
            float sx,
            float sy,
            float sz,
            const float rgba[4]) {
  const float x1 = x + sx;
  const float y1 = y + sy;
  const float z1 = z + sz;
  AddQuad(meshes, x, y, z1, x1, y, z1, x1, y1, z1, x, y1, z1, rgba);
  AddQuad(meshes, x, y, z, x, y1, z, x, y1, z1, x, y, z1, rgba);
  AddQuad(meshes, x1, y, z, x1, y, z1, x1, y1, z1, x1, y1, z, rgba);
  AddQuad(meshes, x, y, z, x1, y, z, x1, y, z1, x, y, z1, rgba);
  AddQuad(meshes, x, y1, z, x, y1, z1, x1, y1, z1, x1, y1, z, rgba);
}

void AddCity(std::vector<cesium_flycube::PresentMesh>* meshes) {
  const float block = 42.0f;
  const float gap = 18.0f;
  const float span = 7.0f * (block + gap);
  const float origin = -span * 0.5f;
  const float road[4] = {0.28f, 0.30f, 0.32f, 1.0f};
  const float park[4] = {0.22f, 0.48f, 0.30f, 1.0f};
  AddQuad(meshes, origin - 30.0f, origin - 30.0f, 0.4f, -origin + 30.0f,
          origin - 30.0f, 0.4f, -origin + 30.0f, -origin + 30.0f, 0.4f,
          origin - 30.0f, -origin + 30.0f, 0.4f, road);

  for (int row = 0; row < 7; ++row) {
    for (int column = 0; column < 7; ++column) {
      const float x = origin + static_cast<float>(column) * (block + gap);
      const float y = origin + static_cast<float>(row) * (block + gap);
      if (column >= 2 && column <= 4 && row >= 2 && row <= 4) {
        continue;
      }
      float rgba[4];
      const float height = HeightAt(column, row);
      ColorForHeight(height, rgba);
      AddBox(meshes, x, y, 0.5f, block, block, height, rgba);
    }
  }
  AddQuad(meshes, origin + 2.0f * (block + gap), origin + 2.0f * (block + gap),
          0.6f, origin + 5.0f * (block + gap) - gap,
          origin + 2.0f * (block + gap), 0.6f,
          origin + 5.0f * (block + gap) - gap,
          origin + 5.0f * (block + gap) - gap, 0.6f,
          origin + 2.0f * (block + gap),
          origin + 5.0f * (block + gap) - gap, 0.6f, park);

  const float tower[4] = {0.93f, 0.84f, 0.62f, 1.0f};
  const float cap[4] = {0.75f, 0.86f, 0.95f, 1.0f};
  const float tx = origin + 3.0f * (block + gap) + 6.0f;
  const float ty = origin + 3.0f * (block + gap) + 6.0f;
  AddBox(meshes, tx, ty, 0.5f, 30.0f, 30.0f, 260.0f, tower);
  AddBox(meshes, tx + 6.0f, ty + 6.0f, 260.5f, 18.0f, 18.0f, 48.0f, cap);
}

// Horizontal bounds of one ellipsoid tile in the local east-north frame.
struct TileExtent {
  double min_x = 0.0;
  double min_y = 0.0;
  double max_x = 0.0;
  double max_y = 0.0;
  double span = 0.0;
  bool valid = false;
};

TileExtent ExtentOf(const cesium_flycube::TileMesh& tile,
                    const cesium_flycube::Georeference& geo) {
  TileExtent extent;
  if (tile.ecef_positions.empty()) {
    return extent;
  }
  const glm::dvec3 first = geo.EcefToLocal(tile.ecef_positions.front());
  extent.min_x = extent.max_x = first.x;
  extent.min_y = extent.max_y = first.y;
  for (const glm::dvec3& ecef : tile.ecef_positions) {
    const glm::dvec3 local = geo.EcefToLocal(ecef);
    extent.min_x = std::min(extent.min_x, local.x);
    extent.min_y = std::min(extent.min_y, local.y);
    extent.max_x = std::max(extent.max_x, local.x);
    extent.max_y = std::max(extent.max_y, local.y);
  }
  extent.span = std::max(extent.max_x - extent.min_x, extent.max_y - extent.min_y);
  extent.valid = true;
  return extent;
}

bool CoversCity(const TileExtent& extent) {
  return extent.valid && extent.min_x < 500.0 && extent.max_x > -500.0 &&
         extent.min_y < 500.0 && extent.max_y > -500.0;
}

}  // namespace

// Ellipsoid terrain over Shanghai, a park, streets, and lit blocks.
int main() {
  cesium_flycube::Session session;
  session.georeference().SetOriginDegrees(121.4737, 31.2304, 0.0);

  // The beauty camera, not a 9 km nadir, selects tiles. Otherwise district
  // colors stay continent-sized and fill the frame behind the blocks.
  const glm::dvec3 eye(-400.0, -520.0, 380.0);
  const glm::dvec3 look_at(0.0, 0.0, 70.0);
  cesium_flycube::Camera overview;
  overview.viewport_size = glm::dvec2(960.0, 540.0);
  overview.location = eye;
  overview.direction = glm::normalize(look_at - eye);
  overview.up = glm::dvec3(0.0, 0.0, 1.0);
  const double aspect = 960.0 / 540.0;
  overview.horizontal_fov_degrees =
      2.0 * std::atan(std::tan(0.39) * aspect) * 180.0 / 3.141592653589793;
  session.cameras().additional_cameras() = {overview};

  cesium_flycube::TilesetHost tileset;
  tileset.settings().maximum_screen_space_error = 8.0;
  tileset.settings().maximum_simultaneous_tile_loads = 64;
  tileset.settings().loading_descendant_limit = 64;
  tileset.LoadEllipsoid();
  tileset.AddRasterOverlay(
      cesium_flycube::MakeDebugColorizeTilesOverlay("districts"));
  const auto views = session.BuildViewStates();
  cesium_flycube::TilesetFrame frame;
  double city_span = 1.0e12;
  for (int i = 0; i < 160; ++i) {
    frame = tileset.Tick(views, 0.05f);
    bool draped = false;
    city_span = 1.0e12;
    for (const cesium_flycube::TileMesh& tile : frame.meshes) {
      if (!tile.rasters.empty() && !tile.texcoords.empty()) {
        draped = true;
      }
      const TileExtent extent = ExtentOf(tile, session.georeference());
      if (CoversCity(extent)) {
        city_span = std::min(city_span, extent.span);
      }
    }
    // Root tiles arrive before the debug overlay. Stopping there leaves the
    // skirt the flat fallback blue and lets one coarse triangle hide the park.
    if (frame.vertex_count > 0 && draped && city_span < 800.0 && i > 4) {
      break;
    }
  }
  std::cout << "city_tile_span_m " << city_span << "\n";
  if (frame.vertex_count == 0) {
    std::cerr << "globe city terrain produced no mesh\n";
    return 1;
  }

  const bool refined = city_span < 800.0;
  std::vector<cesium_flycube::PresentMesh> pictures;
  pictures.reserve(frame.meshes.size() + 80);
  for (const cesium_flycube::TileMesh& tile : frame.meshes) {
    const TileExtent extent = ExtentOf(tile, session.georeference());
    // A parent that still covers the blocks would paint one color across the city.
    if (refined && CoversCity(extent) && extent.span > 1200.0) {
      continue;
    }
    pictures.push_back(
        cesium_flycube::TileMeshToPresent(tile, session.georeference()));
  }
  AddCity(&pictures);

  cesium_flycube::PresentCamera camera;
  camera.eye[0] = -400.0f;
  camera.eye[1] = -520.0f;
  camera.eye[2] = 380.0f;
  camera.target[0] = 0.0f;
  camera.target[1] = 0.0f;
  camera.target[2] = 70.0f;
  camera.up[0] = 0.0f;
  camera.up[1] = 0.15f;
  camera.up[2] = 1.0f;
  camera.fov_y_radians = 0.78f;
  camera.draw_edges = false;

  const cesium_flycube::PresentResult shown = cesium_flycube::PresentScene(
      "globe_city", camera, pictures,
      std::string(CESIUM_FLYCUBE_CAPTURE_DIR) + "/globe_city.png");
  std::cout << "png " << shown.png_path << " pixels " << shown.drawn_pixels
            << " meshes " << pictures.size() << " " << shown.message << "\n";
  if (!shown.ok || shown.drawn_pixels < 8000) {
    std::cerr << "globe city was not presented: " << shown.message << "\n";
    return 1;
  }
  std::cout << "globe_city ok\n";
  return 0;
}
