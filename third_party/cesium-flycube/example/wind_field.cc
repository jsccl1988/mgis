// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "cesium_flycube/gpu/draped_terrain.h"
#include "cesium_flycube/gpu/scene_present.h"
#include "cesium_flycube/tiles/tileset_host.h"
#include "cesium_flycube/view/session.h"

#include <glm/geometric.hpp>

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

// One ASOS station with a usable wind report.
struct StationWind {
  std::string station;
  double lon = 0.0;
  double lat = 0.0;
  float sknt = 0.0f;
  float drct = 0.0f;
};

constexpr Rgba kGround{0.20f, 0.28f, 0.22f, 1.0f};

// Arrow length is 1.6 km per knot so a 20 kt wind is 32 km long. That is a
// display scale, not a displacement. Shaft width is at least 6 km so the barb
// still covers pixels when the whole state is in frame.
constexpr float kMetersPerKnot = 1600.0f;
constexpr float kMinShaftWidth = 6000.0f;
constexpr float kShaftHeight = 3500.0f;

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

bool LoadAsos(const std::filesystem::path& path, std::vector<StationWind>* winds) {
  std::ifstream input(path);
  if (!input) {
    return false;
  }
  std::string line;
  if (!std::getline(input, line)) {
    return false;
  }
  const std::vector<std::string> header = SplitCsv(line);
  int station_col = -1;
  int lon_col = -1;
  int lat_col = -1;
  int sknt_col = -1;
  int drct_col = -1;
  for (int i = 0; i < static_cast<int>(header.size()); ++i) {
    if (header[i] == "station") {
      station_col = i;
    } else if (header[i] == "lon") {
      lon_col = i;
    } else if (header[i] == "lat") {
      lat_col = i;
    } else if (header[i] == "sknt") {
      sknt_col = i;
    } else if (header[i] == "drct") {
      drct_col = i;
    }
  }
  if (station_col < 0 || lon_col < 0 || lat_col < 0 || sknt_col < 0 ||
      drct_col < 0) {
    return false;
  }
  while (std::getline(input, line)) {
    if (Trim(line).empty()) {
      continue;
    }
    const std::vector<std::string> fields = SplitCsv(line);
    const int need = std::max({station_col, lon_col, lat_col, sknt_col, drct_col});
    if (static_cast<int>(fields.size()) <= need) {
      continue;
    }
    if (fields[sknt_col].empty() || fields[drct_col].empty()) {
      continue;
    }
    try {
      StationWind wind;
      wind.station = fields[station_col];
      wind.lon = std::stod(fields[lon_col]);
      wind.lat = std::stod(fields[lat_col]);
      wind.sknt = std::stof(fields[sknt_col]);
      wind.drct = std::stof(fields[drct_col]);
      if (!std::isfinite(wind.lon) || !std::isfinite(wind.lat) ||
          !std::isfinite(wind.sknt) || !std::isfinite(wind.drct)) {
        continue;
      }
      winds->push_back(std::move(wind));
    } catch (const std::exception&) {
      continue;
    }
  }
  return !winds->empty();
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

void AddTri(std::vector<cesium_flycube::PresentMesh>* meshes,
            float x0,
            float y0,
            float z0,
            float x1,
            float y1,
            float z1,
            float x2,
            float y2,
            float z2,
            Rgba color) {
  cesium_flycube::PresentMesh mesh;
  mesh.mesh.xyz = {x0, y0, z0, x1, y1, z1, x2, y2, z2};
  mesh.mesh.indices = {0, 1, 2};
  mesh.color[0] = color.r;
  mesh.color[1] = color.g;
  mesh.color[2] = color.b;
  mesh.color[3] = color.a;
  meshes->push_back(std::move(mesh));
}

Rgba SpeedColor(float sknt) {
  const float t = std::clamp(sknt / 28.0f, 0.0f, 1.0f);
  if (t < 0.5f) {
    const float u = t / 0.5f;
    return {0.20f + 0.55f * u, 0.45f + 0.30f * u, 0.92f - 0.40f * u, 1.0f};
  }
  const float u = (t - 0.5f) / 0.5f;
  return {0.75f + 0.18f * u, 0.75f - 0.40f * u, 0.52f - 0.32f * u, 1.0f};
}

void AddArrow(std::vector<cesium_flycube::PresentMesh>* meshes,
              float x,
              float y,
              float east,
              float north,
              float length,
              Rgba color) {
  const float px = -north;
  const float py = east;
  const float half = std::max(kMinShaftWidth, length * 0.16f) * 0.5f;
  const float head = std::min(length * 0.32f, half * 4.5f);
  const float shaft = std::max(length - head, length * 0.45f);
  const float z0 = 800.0f;
  const float z1 = z0 + kShaftHeight;
  const float zm = z0 + kShaftHeight * 0.5f;

  const float x0 = x;
  const float y0 = y;
  const float xs = x + east * shaft;
  const float ys = y + north * shaft;
  const float xt = x + east * length;
  const float yt = y + north * length;

  AddQuad(meshes, x0 + px * half, y0 + py * half, z0, xs + px * half,
          ys + py * half, z0, xs - px * half, ys - py * half, z0,
          x0 - px * half, y0 - py * half, z0, color);
  AddQuad(meshes, x0 + px * half, y0 + py * half, z1, x0 - px * half,
          y0 - py * half, z1, xs - px * half, ys - py * half, z1,
          xs + px * half, ys + py * half, z1, color);
  AddQuad(meshes, x0 + px * half, y0 + py * half, z0, x0 + px * half,
          y0 + py * half, z1, xs + px * half, ys + py * half, z1,
          xs + px * half, ys + py * half, z0, color);
  AddQuad(meshes, x0 - px * half, y0 - py * half, z0, xs - px * half,
          ys - py * half, z0, xs - px * half, ys - py * half, z1,
          x0 - px * half, y0 - py * half, z1, color);

  const float hx = half * 2.35f;
  AddTri(meshes, xs + px * hx, ys + py * hx, z1, xt, yt, zm, xs - px * hx,
         ys - py * hx, z1, color);
  AddTri(meshes, xs - px * hx, ys - py * hx, z0, xt, yt, zm, xs + px * hx,
         ys + py * hx, z0, color);
  AddTri(meshes, xs + px * hx, ys + py * hx, z0, xs + px * hx, ys + py * hx, z1,
         xt, yt, zm, color);
  AddTri(meshes, xs - px * hx, ys - py * hx, z1, xs - px * hx, ys - py * hx, z0,
         xt, yt, zm, color);
}

}  // namespace

// Iowa ASOS winds for 2024-01-12 18Z on draped ellipsoid terrain.
// Each arrow sits on the station lon/lat. drct is FROM (0 = north).
int main() {
  const std::filesystem::path csv_path =
      std::filesystem::path(CESIUM_FLYCUBE_CAPTURE_DIR) / ".." / "data" /
      "ia_asos_20240112T18Z.csv";
  std::vector<StationWind> winds;
  if (!LoadAsos(csv_path, &winds)) {
    std::cerr << "wind field could not read " << csv_path.string() << "\n";
    return 1;
  }

  double sum_lon = 0.0;
  double sum_lat = 0.0;
  for (const StationWind& wind : winds) {
    sum_lon += wind.lon;
    sum_lat += wind.lat;
  }
  const double mean_lon = sum_lon / static_cast<double>(winds.size());
  const double mean_lat = sum_lat / static_cast<double>(winds.size());
  constexpr double kMetersPerDegLat = 111320.0;
  const double meters_per_deg_lon =
      kMetersPerDegLat * std::cos(mean_lat * 3.14159265358979323846 / 180.0);

  cesium_flycube::Session session;
  cesium_flycube::Camera overview;
  overview.viewport_size = glm::dvec2(960.0, 540.0);
  overview.location = glm::dvec3(0.0, -180000.0, 220000.0);
  overview.direction = glm::normalize(glm::dvec3(0.0, 0.5, -1.0));
  overview.up = glm::dvec3(0.0, 0.0, 1.0);
  session.cameras().additional_cameras() = {overview};

  cesium_flycube::TilesetHost tileset;
  cesium_flycube::DrapedTerrainOptions opts;
  opts.longitude_degrees = mean_lon;
  opts.latitude_degrees = mean_lat;
  opts.max_ticks = 160;
  const cesium_flycube::DrapedTerrainResult terrain =
      cesium_flycube::CaptureDrapedEllipsoid(&session, &tileset, opts);
  if (!terrain.ok) {
    std::cerr << "wind field terrain: " << terrain.message << "\n";
    return 1;
  }

  std::vector<cesium_flycube::PresentMesh> pictures = terrain.pictures;
  float min_x = 1.0e30f;
  float min_y = 1.0e30f;
  float max_x = -1.0e30f;
  float max_y = -1.0e30f;
  for (const StationWind& wind : winds) {
    const float x = static_cast<float>((wind.lon - mean_lon) * meters_per_deg_lon);
    const float y = static_cast<float>((wind.lat - mean_lat) * kMetersPerDegLat);
    const float from = wind.drct * 3.14159265f / 180.0f;
    const float east = -std::sin(from);
    const float north = -std::cos(from);
    const float length = std::max(wind.sknt, 1.0f) * kMetersPerKnot;
    min_x = std::min(min_x, std::min(x, x + east * length));
    max_x = std::max(max_x, std::max(x, x + east * length));
    min_y = std::min(min_y, std::min(y, y + north * length));
    max_y = std::max(max_y, std::max(y, y + north * length));
    AddArrow(&pictures, x, y, east, north, length, SpeedColor(wind.sknt));
  }

  const float cx = 0.5f * (min_x + max_x);
  const float cy = 0.5f * (min_y + max_y);
  const float span = std::max(max_x - min_x, max_y - min_y);

  cesium_flycube::PresentCamera camera;
  camera.eye[0] = cx - span * 0.08f;
  camera.eye[1] = cy - span * 0.95f;
  camera.eye[2] = span * 0.88f;
  camera.target[0] = cx;
  camera.target[1] = cy;
  camera.target[2] = kShaftHeight * 0.4f;
  camera.up[0] = 0.0f;
  camera.up[1] = 0.25f;
  camera.up[2] = 1.0f;
  camera.fov_y_radians = 0.68f;
  camera.draw_edges = false;
  camera.skip_shade = true;

  const cesium_flycube::PresentResult shown = cesium_flycube::PresentScene(
      "wind_field", camera, pictures,
      std::string(CESIUM_FLYCUBE_CAPTURE_DIR) + "/wind_field.png",
      /*open_window=*/false, /*stamp_fps=*/false);
  std::cout << "png " << shown.png_path << " pixels " << shown.drawn_pixels
            << " meshes " << pictures.size() << " stations " << winds.size()
            << " draped " << terrain.draped << " " << shown.message << "\n";
  if (!shown.ok || shown.drawn_pixels < 8000) {
    std::cerr << "wind field was not presented: " << shown.message << "\n";
    return 1;
  }
  std::cout << "wind_field ok\n";
  return 0;
}
