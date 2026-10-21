// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "cesium_flycube/gpu/scene_present.h"

#include <cmath>
#include <iostream>
#include <string>
#include <vector>

namespace {

struct Rgba {
  float r;
  float g;
  float b;
  float a;
};

constexpr Rgba kBedrock{0.42f, 0.36f, 0.32f, 1.0f};
constexpr Rgba kConfined{0.12f, 0.32f, 0.58f, 1.0f};
constexpr Rgba kAquitard{0.40f, 0.46f, 0.26f, 1.0f};
constexpr Rgba kSaturated{0.16f, 0.52f, 0.78f, 1.0f};
constexpr Rgba kDrySand{0.78f, 0.68f, 0.42f, 1.0f};
constexpr Rgba kSoil{0.38f, 0.26f, 0.16f, 1.0f};
constexpr Rgba kPond{0.10f, 0.38f, 0.62f, 1.0f};
constexpr Rgba kWell{0.82f, 0.84f, 0.86f, 1.0f};
constexpr Rgba kWaterColumn{0.20f, 0.62f, 0.90f, 1.0f};

float WaterTable(float x, float y) {
  return 74.0f + 7.0f * std::sin(x * 0.035f) + 3.5f * std::cos(y * 0.05f);
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
             Rgba color) {
  cesium_flycube::PresentMesh mesh;
  mesh.mesh.xyz = {x0, y0, z0, x1, y1, z1, x2, y2, z2, x3, y3, z3};
  mesh.mesh.indices = {0, 1, 2, 0, 2, 3};
  mesh.color[0] = color.r;
  mesh.color[1] = color.g;
  mesh.color[2] = color.b;
  mesh.color[3] = color.a;
  meshes->push_back(std::move(mesh));
}

void AddBox(std::vector<cesium_flycube::PresentMesh>* meshes,
            float x,
            float y,
            float z,
            float sx,
            float sy,
            float sz,
            Rgba color) {
  if (sz <= 0.2f) {
    return;
  }
  const float x1 = x + sx;
  const float y1 = y + sy;
  const float z1 = z + sz;
  AddQuad(meshes, x, y, z1, x1, y, z1, x1, y1, z1, x, y1, z1, color);
  AddQuad(meshes, x, y, z, x, y1, z, x, y1, z1, x, y, z1, color);
  AddQuad(meshes, x1, y, z, x1, y, z1, x1, y1, z1, x1, y1, z, color);
  AddQuad(meshes, x, y, z, x1, y, z, x1, y, z1, x, y, z1, color);
  AddQuad(meshes, x, y1, z, x, y1, z1, x1, y1, z1, x1, y1, z, color);
}

// The shaft stands just in front of the near face. A bore inside the opaque
// cells only leaves the wellhead, so the water column is drawn on the open side.
void AddWell(std::vector<cesium_flycube::PresentMesh>* meshes,
             float x,
             float y,
             float table) {
  const float sx = 12.0f;
  const float sy = 14.0f;
  const float x1 = x + sx;
  const float y1 = y + sy;
  const float top = 112.0f;
  AddQuad(meshes, x, y, top, x1, y, top, x1, y1, top, x, y1, top, kWell);
  AddQuad(meshes, x1, y, 0.0f, x1, y, top, x1, y1, top, x1, y1, 0.0f, kWell);
  AddQuad(meshes, x, y1, 0.0f, x, y1, top, x1, y1, top, x1, y1, 0.0f, kWell);
  AddQuad(meshes, x, y, 0.0f, x, y1, 0.0f, x, y1, top, x, y, top, kWell);
  AddBox(meshes, x + 3.0f, y - 1.6f, 16.0f, 6.0f, 8.0f, table - 16.0f, kWaterColumn);
  AddBox(meshes, x - 2.0f, y - 2.0f, top, sx + 4.0f, sy + 2.0f, 4.0f, kWell);
}

void AddBlock(std::vector<cesium_flycube::PresentMesh>* meshes) {
  constexpr int kColumns = 12;
  constexpr int kRows = 8;
  constexpr float kCell = 18.0f;
  for (int row = 0; row < kRows; ++row) {
    for (int column = 0; column < kColumns; ++column) {
      // Leave the far corner out so the layer stack is visible.
      if (column > 8 && row > 5) {
        continue;
      }
      const float x = static_cast<float>(column) * kCell;
      const float y = static_cast<float>(row) * kCell;
      const float table = WaterTable(x + kCell * 0.5f, y + kCell * 0.5f);
      AddBox(meshes, x, y, 0.0f, kCell, kCell, 22.0f, kBedrock);
      AddBox(meshes, x, y, 22.0f, kCell, kCell, 26.0f, kConfined);
      AddBox(meshes, x, y, 48.0f, kCell, kCell, 14.0f, kAquitard);
      AddBox(meshes, x, y, 62.0f, kCell, kCell, table - 62.0f, kSaturated);
      AddBox(meshes, x, y, table, kCell, kCell, 98.0f - table, kDrySand);
      AddBox(meshes, x, y, 98.0f, kCell, kCell, 10.0f, kSoil);
    }
  }
  AddBox(meshes, 36.0f, 36.0f, 108.2f, 36.0f, 28.0f, 2.4f, kPond);
  AddWell(meshes, 42.0f, -16.0f, WaterTable(48.0f, 9.0f));
  AddWell(meshes, 126.0f, -16.0f, WaterTable(132.0f, 9.0f));
}

}  // namespace

// Cutaway groundwater block: bedrock, confined aquifer, aquitard, water table,
// dry sand, soil, a recharge pond, and two wells.
int main() {
  std::vector<cesium_flycube::PresentMesh> pictures;
  AddBlock(&pictures);

  cesium_flycube::PresentCamera camera;
  camera.eye[0] = -70.0f;
  camera.eye[1] = -150.0f;
  camera.eye[2] = 190.0f;
  camera.target[0] = 90.0f;
  camera.target[1] = 60.0f;
  camera.target[2] = 55.0f;
  camera.up[0] = 0.0f;
  camera.up[1] = 0.15f;
  camera.up[2] = 1.0f;
  camera.fov_y_radians = 0.78f;
  camera.draw_edges = false;

  const cesium_flycube::PresentResult shown = cesium_flycube::PresentScene(
      "groundwater", camera, pictures,
      std::string(CESIUM_FLYCUBE_CAPTURE_DIR) + "/groundwater.png");
  std::cout << "png " << shown.png_path << " pixels " << shown.drawn_pixels
            << " meshes " << pictures.size() << " " << shown.message << "\n";
  if (!shown.ok || shown.drawn_pixels < 8000) {
    std::cerr << "groundwater was not presented: " << shown.message << "\n";
    return 1;
  }
  std::cout << "groundwater ok\n";
  return 0;
}
