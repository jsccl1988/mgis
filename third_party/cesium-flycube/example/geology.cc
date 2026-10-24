// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "cesium_flycube/gpu/draped_terrain.h"
#include "cesium_flycube/gpu/scene_present.h"
#include "cesium_flycube/tiles/tileset_host.h"
#include "cesium_flycube/view/session.h"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace {

struct Rgba {
  float r;
  float g;
  float b;
  float a;
};

// One logged interval. Depths are meters down the hole, top to base.
struct Interval {
  std::string code;
  std::string name;
  float top_m = 0.0f;
  float base_m = 0.0f;
};

// Intervals for one well, in file order. The CSV has no collar coordinates.
struct WellColumn {
  std::string well;
  std::vector<Interval> intervals;
};

// Logged thickness is drawn 1:1 on Z (positive up). Column width and the gap
// between wells are display sizes only.
constexpr float kVerticalExaggeration = 1.0f;
constexpr float kColumnWidth = 96.0f;
constexpr float kColumnDepth = 150.0f;
constexpr float kColumnGap = 28.0f;

std::string Trim(std::string text) {
  const auto begin = text.find_first_not_of(" \t\r\n");
  if (begin == std::string::npos) {
    return {};
  }
  const auto end = text.find_last_not_of(" \t\r\n");
  return text.substr(begin, end - begin + 1);
}

std::vector<std::string> SplitCsv(const std::string& line) {
  std::vector<std::string> fields;
  std::stringstream stream(line);
  std::string field;
  while (std::getline(stream, field, ',')) {
    fields.push_back(Trim(field));
  }
  return fields;
}

bool LoadLithology(const std::filesystem::path& path,
                   std::vector<WellColumn>* wells) {
  std::ifstream input(path);
  if (!input) {
    return false;
  }
  std::string line;
  if (!std::getline(input, line)) {
    return false;
  }
  const std::vector<std::string> header = SplitCsv(line);
  int well_col = -1;
  int code_col = -1;
  int top_col = -1;
  int base_col = -1;
  int name_col = -1;
  for (int i = 0; i < static_cast<int>(header.size()); ++i) {
    if (header[i] == "Well") {
      well_col = i;
    } else if (header[i] == "Unit") {
      code_col = i;
    } else if (header[i] == "m1") {
      top_col = i;
    } else if (header[i] == "m2") {
      base_col = i;
    } else if (header[i] == "Unit_Name") {
      name_col = i;
    }
  }
  if (well_col < 0 || top_col < 0 || base_col < 0) {
    return false;
  }
  while (std::getline(input, line)) {
    if (Trim(line).empty()) {
      continue;
    }
    const std::vector<std::string> fields = SplitCsv(line);
    if (fields.empty() || fields[0].empty()) {
      continue;
    }
    const int need = std::max({well_col, top_col, base_col});
    if (static_cast<int>(fields.size()) <= need) {
      continue;
    }
    try {
      Interval interval;
      interval.top_m = std::stof(fields[top_col]);
      interval.base_m = std::stof(fields[base_col]);
      if (code_col >= 0 && code_col < static_cast<int>(fields.size())) {
        interval.code = fields[code_col];
      }
      if (name_col >= 0 && name_col < static_cast<int>(fields.size())) {
        interval.name = fields[name_col];
      }
      if (!std::isfinite(interval.top_m) || !std::isfinite(interval.base_m) ||
          interval.base_m <= interval.top_m) {
        continue;
      }
      const std::string& well_name = fields[well_col];
      if (wells->empty() || wells->back().well != well_name) {
        wells->push_back(WellColumn{well_name, {}});
      }
      wells->back().intervals.push_back(std::move(interval));
    } catch (const std::exception&) {
      continue;
    }
  }
  return !wells->empty();
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
  if (sz <= 0.4f || sx <= 0.4f || sy <= 0.4f) {
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

Rgba ColorForUnit(const std::string& code) {
  if (code == "QTs") {
    return {0.78f, 0.68f, 0.42f, 1.0f};
  }
  if (code == "Tba") {
    return {0.22f, 0.32f, 0.28f, 1.0f};
  }
  if (code == "Tlt") {
    return {0.90f, 0.84f, 0.46f, 1.0f};
  }
  if (code == "Trd") {
    return {0.74f, 0.38f, 0.32f, 1.0f};
  }
  if (code == "Tha") {
    return {0.48f, 0.36f, 0.24f, 1.0f};
  }
  if (code == "Ta") {
    return {0.58f, 0.34f, 0.20f, 1.0f};
  }
  if (code == "Tvb") {
    return {0.50f, 0.20f, 0.16f, 1.0f};
  }
  if (code == "Td") {
    return {0.84f, 0.50f, 0.26f, 1.0f};
  }
  if (code == "Mzq") {
    return {0.84f, 0.86f, 0.88f, 1.0f};
  }
  if (code == "Mzqp") {
    return {0.62f, 0.58f, 0.70f, 1.0f};
  }
  if (code == "Mzp") {
    return {0.40f, 0.34f, 0.56f, 1.0f};
  }
  if (code == "Mza") {
    return {0.34f, 0.50f, 0.30f, 1.0f};
  }
  if (code == "Mzr") {
    return {0.72f, 0.44f, 0.52f, 1.0f};
  }
  if (code == "Mzra") {
    return {0.58f, 0.38f, 0.42f, 1.0f};
  }
  if (code == "Mzgr") {
    return {0.80f, 0.56f, 0.58f, 1.0f};
  }
  if (code == "Mzu") {
    return {0.46f, 0.46f, 0.48f, 1.0f};
  }
  return {0.55f, 0.55f, 0.52f, 1.0f};
}

}  // namespace

// Fallon FORGE lithology as a fence of well columns on draped ellipsoid terrain.
// Depth increases downward. Vertical exaggeration is 1 (see kVerticalExaggeration).
int main() {
  const std::filesystem::path csv_path =
      std::filesystem::path(CESIUM_FLYCUBE_CAPTURE_DIR) / ".." / "data" /
      "fallon_forge_lithology.csv";
  std::vector<WellColumn> wells;
  if (!LoadLithology(csv_path, &wells)) {
    std::cerr << "geology could not read " << csv_path.string() << "\n";
    return 1;
  }

  // Fallon, NV approx. Columns sit in local meters with depth along -Z.
  cesium_flycube::Session session;
  cesium_flycube::TilesetHost tileset;
  cesium_flycube::DrapedTerrainOptions opts;
  opts.longitude_degrees = -118.78;
  opts.latitude_degrees = 39.47;
  const cesium_flycube::DrapedTerrainResult terrain =
      cesium_flycube::CaptureDrapedEllipsoid(&session, &tileset, opts);
  if (!terrain.ok) {
    std::cerr << "geology terrain: " << terrain.message << "\n";
    return 1;
  }

  std::vector<cesium_flycube::PresentMesh> pictures = terrain.pictures;
  float max_depth = 1.0f;
  int interval_count = 0;
  const float pitch = kColumnWidth + kColumnGap;
  for (size_t i = 0; i < wells.size(); ++i) {
    const float x = static_cast<float>(i) * pitch - 200.0f;
    for (const Interval& interval : wells[i].intervals) {
      const float top = -interval.top_m * kVerticalExaggeration;
      const float base = -interval.base_m * kVerticalExaggeration;
      const float thickness = top - base;
      AddBox(&pictures, x, 0.0f, base, kColumnWidth, kColumnDepth, thickness,
             ColorForUnit(interval.code));
      max_depth = std::max(max_depth, interval.base_m * kVerticalExaggeration);
      ++interval_count;
    }
  }

  const float length = std::max(pitch * static_cast<float>(wells.size()), pitch);
  cesium_flycube::PresentCamera camera;
  camera.eye[0] = length * 0.20f;
  camera.eye[1] = -max_depth * 1.85f;
  camera.eye[2] = max_depth * 0.55f;
  camera.target[0] = length * 0.20f;
  camera.target[1] = kColumnDepth * 0.5f;
  camera.target[2] = -max_depth * 0.25f;
  camera.up[0] = 0.0f;
  camera.up[1] = 0.16f;
  camera.up[2] = 1.0f;
  camera.fov_y_radians = 0.70f;
  camera.draw_edges = false;
  camera.skip_shade = true;

  const cesium_flycube::PresentResult shown = cesium_flycube::PresentScene(
      "geology", camera, pictures,
      std::string(CESIUM_FLYCUBE_CAPTURE_DIR) + "/geology.png",
      /*open_window=*/false, /*stamp_fps=*/false);
  std::cout << "png " << shown.png_path << " pixels " << shown.drawn_pixels
            << " meshes " << pictures.size() << " wells " << wells.size()
            << " intervals " << interval_count << " draped " << terrain.draped
            << " " << shown.message << "\n";
  if (!shown.ok || shown.drawn_pixels < 8000) {
    std::cerr << "geology was not presented: " << shown.message << "\n";
    return 1;
  }
  std::cout << "geology ok\n";
  return 0;
}
