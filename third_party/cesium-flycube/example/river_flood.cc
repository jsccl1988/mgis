// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "cesium_flycube/gpu/draped_terrain.h"
#include "cesium_flycube/gpu/scene_present.h"
#include "cesium_flycube/tiles/tileset_host.h"
#include "cesium_flycube/view/session.h"

#include <algorithm>
#include <cctype>
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

constexpr Rgba kPlain{0.34f, 0.50f, 0.28f, 1.0f};
constexpr Rgba kBank{0.72f, 0.60f, 0.36f, 1.0f};
constexpr Rgba kWater{0.12f, 0.38f, 0.78f, 1.0f};
constexpr Rgba kGauge{0.90f, 0.91f, 0.93f, 1.0f};
constexpr Rgba kGaugeCap{0.78f, 0.18f, 0.16f, 1.0f};
constexpr Rgba kLabel{0.96f, 0.94f, 0.88f, 1.0f};

// Display width in meters = discharge_cfs * kMetersPerCfs. Not a surveyed
// cross section and not an inundation outline.
constexpr float kMetersPerCfs = 0.045f;

std::string Trim(std::string text) {
  const auto begin = text.find_first_not_of(" \t\r\n");
  if (begin == std::string::npos) {
    return {};
  }
  const auto end = text.find_last_not_of(" \t\r\n");
  return text.substr(begin, end - begin + 1);
}

bool ReadFile(const std::filesystem::path& path, std::string* text) {
  std::ifstream input(path, std::ios::binary);
  if (!input) {
    return false;
  }
  std::ostringstream buffer;
  buffer << input.rdbuf();
  *text = buffer.str();
  return !text->empty();
}

bool ParseNumberAt(const std::string& text, size_t pos, double* value) {
  while (pos < text.size() && (text[pos] == ' ' || text[pos] == '\t')) {
    ++pos;
  }
  if (pos >= text.size()) {
    return false;
  }
  size_t end = pos;
  if (text[pos] == '"') {
    ++pos;
    end = text.find('"', pos);
    if (end == std::string::npos) {
      return false;
    }
  } else {
    end = pos;
    while (end < text.size() &&
           (std::isdigit(static_cast<unsigned char>(text[end])) || text[end] == '-' ||
            text[end] == '+' || text[end] == '.' || text[end] == 'e' ||
            text[end] == 'E')) {
      ++end;
    }
  }
  try {
    *value = std::stod(text.substr(pos, end - pos));
  } catch (const std::exception&) {
    return false;
  }
  return std::isfinite(*value);
}

bool FindLabeledNumber(const std::string& text, const std::string& key, double* value) {
  const size_t key_pos = text.find(key);
  if (key_pos == std::string::npos) {
    return false;
  }
  const size_t colon = text.find(':', key_pos + key.size());
  if (colon == std::string::npos) {
    return false;
  }
  return ParseNumberAt(text, colon + 1, value);
}

// The instantaneous sample is the value object that carries dateTime.
bool ReadGauge(const std::string& json,
               double* cfs,
               double* latitude,
               double* longitude,
               std::string* when) {
  if (json.find("05481650") == std::string::npos ||
      json.find("\"00060\"") == std::string::npos) {
    return false;
  }
  const size_t time_key = json.find("\"dateTime\"");
  if (time_key == std::string::npos) {
    return false;
  }
  const size_t value_key = json.rfind("\"value\"", time_key);
  if (value_key == std::string::npos || !ParseNumberAt(json, json.find(':', value_key) + 1, cfs)) {
    return false;
  }
  if (!std::isfinite(*cfs) || *cfs < 0.0 || *cfs >= 999999.0) {
    return false;
  }
  const size_t quote = json.find('"', time_key + 10);
  const size_t quote_end = quote == std::string::npos ? std::string::npos : json.find('"', quote + 1);
  if (quote != std::string::npos && quote_end != std::string::npos) {
    *when = json.substr(quote + 1, quote_end - quote - 1);
  }
  if (!FindLabeledNumber(json, "\"latitude\"", latitude) ||
      !FindLabeledNumber(json, "\"longitude\"", longitude)) {
    return false;
  }
  return true;
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
  if (sz <= 0.25f || sx <= 0.25f || sy <= 0.25f) {
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

const char* DigitRows(char digit) {
  switch (digit) {
    case '0':
      return "01110"
             "10001"
             "10001"
             "10001"
             "10001"
             "10001"
             "01110";
    case '1':
      return "00100"
             "01100"
             "00100"
             "00100"
             "00100"
             "00100"
             "01110";
    case '2':
      return "01110"
             "10001"
             "00001"
             "00010"
             "00100"
             "01000"
             "11111";
    case '3':
      return "11110"
             "00001"
             "00001"
             "01110"
             "00001"
             "00001"
             "11110";
    case '4':
      return "10001"
             "10001"
             "10001"
             "11111"
             "00001"
             "00001"
             "00001";
    case '5':
      return "11111"
             "10000"
             "10000"
             "11110"
             "00001"
             "00001"
             "11110";
    case '6':
      return "01110"
             "10000"
             "10000"
             "11110"
             "10001"
             "10001"
             "01110";
    case '7':
      return "11111"
             "00001"
             "00010"
             "00100"
             "01000"
             "01000"
             "01000";
    case '8':
      return "01110"
             "10001"
             "10001"
             "01110"
             "10001"
             "10001"
             "01110";
    case '9':
      return "01110"
             "10001"
             "10001"
             "01111"
             "00001"
             "00001"
             "01110";
    default:
      return "00000"
             "00000"
             "00000"
             "00000"
             "00000"
             "00000"
             "00000";
  }
}

void AddLabel(std::vector<cesium_flycube::PresentMesh>* meshes,
              const std::string& text,
              float origin_x,
              float origin_y,
              float origin_z,
              float cell) {
  float cursor = origin_x;
  for (char digit : text) {
    const char* rows = DigitRows(digit);
    for (int row = 0; row < 7; ++row) {
      for (int column = 0; column < 5; ++column) {
        if (rows[row * 5 + column] != '1') {
          continue;
        }
        AddBox(meshes, cursor + static_cast<float>(column) * cell, origin_y,
               origin_z + static_cast<float>(6 - row) * cell, cell * 0.86f, cell * 0.55f,
               cell * 0.86f, kLabel);
      }
    }
    cursor += cell * 6.0f;
  }
}

}  // namespace

// USGS gauge 05481650 as a straight channel on draped ellipsoid terrain.
// Width tracks discharge. There is no mapped flood polygon in the input.
int main() {
  const std::filesystem::path json_path =
      std::filesystem::path(CESIUM_FLYCUBE_CAPTURE_DIR) / ".." / "data" / "nwis_iv.json";
  std::string json;
  double cfs = 0.0;
  double latitude = 42.0;
  double longitude = -93.6;
  std::string when;
  const bool usable = ReadFile(json_path, &json) &&
                      ReadGauge(json, &cfs, &latitude, &longitude, &when);

  float width = 80.0f;
  float water_height = 8.0f;
  if (usable) {
    width = static_cast<float>(cfs) * kMetersPerCfs;
    water_height = std::max(10.0f, width * 0.12f);
    std::cout << "site 05481650 discharge_cfs " << cfs << " time " << when
              << " lat " << latitude << " lon " << longitude << " width_m "
              << width << "\n";
  } else {
    std::cout << "nwis_iv.json has no usable discharge; schematic channel\n";
  }

  cesium_flycube::Session session;
  cesium_flycube::TilesetHost tileset;
  cesium_flycube::DrapedTerrainOptions opts;
  opts.longitude_degrees = longitude;
  opts.latitude_degrees = latitude;
  const cesium_flycube::DrapedTerrainResult terrain =
      cesium_flycube::CaptureDrapedEllipsoid(&session, &tileset, opts);
  if (!terrain.ok) {
    std::cerr << "river_flood terrain: " << terrain.message << "\n";
    return 1;
  }

  std::vector<cesium_flycube::PresentMesh> pictures = terrain.pictures;
  const float half_w = width * 0.5f;
  const float half_l = std::max(width * 3.2f, 420.0f);
  const float bank = std::max(14.0f, width * 0.16f);
  const float bank_h = std::max(4.0f, water_height * 0.38f);

  if (usable) {
    AddBox(&pictures, -half_w - bank, -half_l, 0.0f, bank, half_l * 2.0f, bank_h,
           kBank);
    AddBox(&pictures, half_w, -half_l, 0.0f, bank, half_l * 2.0f, bank_h, kBank);
    AddBox(&pictures, -half_w, -half_l, 0.0f, width, half_l * 2.0f, water_height,
           kWater);
  }
  AddBox(&pictures, -6.0f, -6.0f, water_height, 12.0f, 12.0f, 46.0f, kGauge);
  AddBox(&pictures, -9.0f, -9.0f, water_height + 46.0f, 18.0f, 18.0f, 8.0f,
         kGaugeCap);

  constexpr float kCell = 8.0f;
  AddLabel(&pictures, "05481650", half_w + 6.0f, -kCell * 3.5f, 8.0f, kCell);

  cesium_flycube::PresentCamera camera;
  camera.eye[0] = -half_w * 0.35f;
  camera.eye[1] = -half_l * 1.55f;
  camera.eye[2] = half_l * 0.72f;
  camera.target[0] = half_w * 0.85f;
  camera.target[1] = 0.0f;
  camera.target[2] = water_height;
  camera.up[0] = 0.0f;
  camera.up[1] = 0.22f;
  camera.up[2] = 1.0f;
  camera.fov_y_radians = 0.62f;
  camera.draw_edges = false;
  camera.skip_shade = true;

  const cesium_flycube::PresentResult shown = cesium_flycube::PresentScene(
      "river_flood", camera, pictures,
      std::string(CESIUM_FLYCUBE_CAPTURE_DIR) + "/river_flood.png",
      /*open_window=*/false, /*stamp_fps=*/false);
  std::cout << "png " << shown.png_path << " pixels " << shown.drawn_pixels
            << " meshes " << pictures.size() << " draped " << terrain.draped
            << " " << shown.message << "\n";
  if (!shown.ok || shown.drawn_pixels < 8000) {
    std::cerr << "river flood was not presented: " << shown.message << "\n";
    return 1;
  }
  std::cout << "river_flood ok\n";
  return 0;
}
