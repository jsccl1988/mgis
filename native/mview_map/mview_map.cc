// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "native/mview_map/mview_map.h"

#include "native/mview_map/camera_follow.h"
#include "native/mview_map/map_surface.h"

#include <cmath>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <new>
#include <sstream>
#include <string>
#include <vector>

namespace {

const int kJsonCap = 64 * 1024;
const int kStyleCap = 1 << 20;

// One style or GeoJSON layer kept in memory. Visibility is the checkbox.
struct Layer {
  std::string id;
  std::string type;
  int visible;
};

struct GeoSource {
  std::string id;
  std::wstring path;
};

std::string ReadFileLimited(const std::string& path) {
  std::ifstream in(path.c_str(), std::ios::in | std::ios::binary);
  if (!in)
    return std::string();
  std::ostringstream buf;
  buf << in.rdbuf();
  std::string text = buf.str();
  if (text.size() > static_cast<size_t>(kStyleCap))
    text.resize(static_cast<size_t>(kStyleCap));
  return text;
}

std::string StripFileUrl(const char* url) {
  if (!url)
    return std::string();
  const std::string raw(url);
  const std::string prefix = "file://";
  if (raw.compare(0, prefix.size(), prefix) == 0) {
    std::string path = raw.substr(prefix.size());
    if (path.size() >= 3 && path[0] == '/' && path[2] == ':')
      path.erase(path.begin());
    return path;
  }
  return raw;
}

size_t MatchBrace(const std::string& s, size_t open) {
  int depth = 0;
  bool in_string = false;
  bool escape = false;
  for (size_t i = open; i < s.size(); ++i) {
    const char c = s[i];
    if (in_string) {
      if (escape) {
        escape = false;
        continue;
      }
      if (c == '\\') {
        escape = true;
        continue;
      }
      if (c == '"')
        in_string = false;
      continue;
    }
    if (c == '"') {
      in_string = true;
      continue;
    }
    if (c == '{')
      ++depth;
    else if (c == '}') {
      --depth;
      if (depth == 0)
        return i;
    }
  }
  return std::string::npos;
}

bool ExtractString(const std::string& object, const char* key, std::string* out) {
  const std::string pattern = std::string("\"") + key + "\"";
  const size_t key_pos = object.find(pattern);
  if (key_pos == std::string::npos)
    return false;
  const size_t colon = object.find(':', key_pos + pattern.size());
  if (colon == std::string::npos)
    return false;
  const size_t quote = object.find('"', colon + 1);
  if (quote == std::string::npos)
    return false;
  std::string value;
  bool escape = false;
  for (size_t i = quote + 1; i < object.size(); ++i) {
    const char c = object[i];
    if (escape) {
      value.push_back(c);
      escape = false;
      continue;
    }
    if (c == '\\') {
      escape = true;
      continue;
    }
    if (c == '"') {
      *out = value;
      return true;
    }
    value.push_back(c);
  }
  return false;
}

bool ObjectIsVisible(const std::string& object) {
  const size_t key = object.find("\"visibility\"");
  if (key == std::string::npos)
    return true;
  std::string value;
  if (!ExtractString(object.substr(key), "visibility", &value))
    return true;
  return value != "none";
}

void ParseLayers(const std::string& json, std::vector<Layer>* layers) {
  const size_t key = json.find("\"layers\"");
  if (key == std::string::npos)
    return;
  const size_t bracket = json.find('[', key);
  if (bracket == std::string::npos)
    return;
  size_t i = bracket + 1;
  while (i < json.size()) {
    if (json[i] == ']')
      break;
    if (json[i] != '{') {
      ++i;
      continue;
    }
    const size_t end = MatchBrace(json, i);
    if (end == std::string::npos)
      break;
    const std::string object = json.substr(i, end - i + 1);
    Layer layer;
    layer.visible = 1;
    if (ExtractString(object, "id", &layer.id) && !layer.id.empty()) {
      ExtractString(object, "type", &layer.type);
      layer.visible = ObjectIsVisible(object) ? 1 : 0;
      layers->push_back(layer);
    }
    i = end + 1;
  }
}

std::string EscapeJson(const std::string& text) {
  std::string out;
  for (size_t i = 0; i < text.size(); ++i) {
    const char c = text[i];
    if (c == '\\' || c == '"')
      out.push_back('\\');
    out.push_back(c);
  }
  return out;
}

std::string BuildLayerJson(const std::vector<Layer>& layers) {
  std::string json = "[";
  for (size_t i = 0; i < layers.size(); ++i) {
    if (i)
      json.push_back(',');
    json += "{\"id\":\"";
    json += EscapeJson(layers[i].id);
    json += "\",\"type\":\"";
    json += EscapeJson(layers[i].type);
    json += "\",\"visible\":";
    json += layers[i].visible ? "1" : "0";
    json += "}";
  }
  json.push_back(']');
  return json;
}

Layer* FindLayer(std::vector<Layer>* layers, const char* layer_id) {
  if (!layers || !layer_id)
    return NULL;
  for (size_t i = 0; i < layers->size(); ++i) {
    if ((*layers)[i].id == layer_id)
      return &(*layers)[i];
  }
  return NULL;
}

}  // namespace

// In-memory map session. Camera and style layers live here. There is no
// GL context and no basemap texture.
struct MvmMap {
  HWND parent;
  HWND notify_target;
  UINT notify_message;
  int width;
  int height;
  double pixel_ratio;
  double lon;
  double lat;
  double zoom;
  double bearing;
  double pitch;
  double height_m;
  std::string style_url;
  std::string last_query_json;
  std::vector<Layer> layers;
  std::vector<GeoSource> sources;
  MapSurface* surface;
  MvmPointerFn pointer_fn;
  void* pointer_user;

  MvmMap()
      : parent(NULL),
        notify_target(NULL),
        notify_message(0),
        width(0),
        height(0),
        pixel_ratio(1.0),
        lon(0.0),
        lat(0.0),
        zoom(0.0),
        bearing(0.0),
        pitch(0.0),
        height_m(0.0),
        surface(NULL),
        pointer_fn(NULL),
        pointer_user(NULL) {}
};

namespace {

void RefreshHeight(MvmMap* map) {
  mview_map::MapCameraState state;
  state.lon = map->lon;
  state.lat = map->lat;
  state.zoom = map->zoom;
  state.bearing = map->bearing;
  state.pitch = map->pitch;
  mview_map::ViewCameraState view;
  mview_map::Follow2dMapToView(state, &view);
  map->height_m = view.height_m;
}

bool Finite(double value) {
  return std::isfinite(value) != 0;
}

}  // namespace

MvmMap* mvm_create(HWND parent, int width, int height, double pixel_ratio) {
  MvmMap* map = new (std::nothrow) MvmMap();
  if (!map)
    return NULL;
  map->parent = parent;
  map->width = width;
  map->height = height;
  if (Finite(pixel_ratio) && pixel_ratio > 0.0)
    map->pixel_ratio = pixel_ratio;
  RefreshHeight(map);
  return map;
}

void mvm_resize(MvmMap* map, int width, int height, double pixel_ratio) {
  if (!map)
    return;
  map->width = width;
  map->height = height;
  if (Finite(pixel_ratio) && pixel_ratio > 0.0)
    map->pixel_ratio = pixel_ratio;
  if (map->surface)
    map->surface->Resize(width, height);
}

void mvm_destroy(MvmMap* map) {
  if (!map)
    return;
  delete map->surface;
  map->surface = NULL;
  delete map;
}

void mvm_set_pointer_callback(MvmMap* map, MvmPointerFn fn, void* user) {
  if (!map)
    return;
  map->pointer_fn = fn;
  map->pointer_user = user;
}

int mvm_present(MvmMap* map) {
  if (!map || !map->parent)
    return 0;
  if (!map->surface)
    map->surface = new (std::nothrow) MapSurface();
  if (!map->surface)
    return 0;
  return map->surface->Present(map->parent, map->width, map->height, map->lon,
                               map->lat, map->zoom, map->bearing, map->pitch,
                               map->pointer_fn, map->pointer_user);
}

unsigned int mvm_frame_token(MvmMap* map) {
  if (!map || !map->surface)
    return 0;
  return map->surface->token();
}

int mvm_last_draw_calls(MvmMap* map) {
  if (!map || !map->surface)
    return 0;
  return map->surface->draw_calls();
}

void mvm_set_notify(MvmMap* map, HWND target, UINT message) {
  if (!map)
    return;
  map->notify_target = target;
  map->notify_message = message;
}

void mvm_set_style_url(MvmMap* map, const char* url) {
  if (!map)
    return;
  map->layers.clear();
  map->style_url = StripFileUrl(url);
  if (map->style_url.empty())
    return;
  const std::string json = ReadFileLimited(map->style_url);
  if (json.empty())
    return;
  ParseLayers(json, &map->layers);
}

void mvm_set_camera(MvmMap* map, double lon, double lat, double zoom,
                    double bearing, double pitch) {
  if (!map)
    return;
  if (!Finite(lon) || !Finite(lat) || !Finite(zoom) || !Finite(bearing) ||
      !Finite(pitch))
    return;
  map->lon = lon;
  map->lat = lat;
  map->zoom = zoom;
  map->bearing = bearing;
  map->pitch = pitch;
  RefreshHeight(map);
}

void mvm_get_camera(MvmMap* map, double* lon, double* lat, double* zoom,
                    double* bearing, double* pitch) {
  if (!map || !lon || !lat || !zoom || !bearing || !pitch)
    return;
  *lon = map->lon;
  *lat = map->lat;
  *zoom = map->zoom;
  *bearing = map->bearing;
  *pitch = map->pitch;
}

void mvm_set_layer_visible(MvmMap* map, const char* layer_id, int visible) {
  if (!map)
    return;
  Layer* layer = FindLayer(&map->layers, layer_id);
  if (!layer)
    return;
  layer->visible = visible ? 1 : 0;
}

int mvm_list_layers(MvmMap* map, char* json, int cap) {
  if (!map)
    return -1;
  const std::string body = BuildLayerJson(map->layers);
  if (body.size() > static_cast<size_t>(kJsonCap))
    return -1;
  if (!json || cap <= 0)
    return static_cast<int>(body.size());
  if (static_cast<int>(body.size()) + 1 > cap)
    return -1;
  memcpy(json, body.data(), body.size());
  json[body.size()] = '\0';
  return static_cast<int>(body.size());
}

void mvm_add_geojson(MvmMap* map, const char* source_id, const wchar_t* path) {
  if (!map || !source_id || !source_id[0])
    return;
  GeoSource source;
  source.id = source_id;
  if (path)
    source.path = path;
  map->sources.push_back(source);
  if (FindLayer(&map->layers, source_id))
    return;
  Layer layer;
  layer.id = source_id;
  layer.type = "geojson";
  layer.visible = 1;
  map->layers.push_back(layer);
}

void mvm_query_point(MvmMap* map, double x, double y, uint64_t request_id) {
  if (!map)
    return;
  const double width = map->width > 0 ? static_cast<double>(map->width) : 1.0;
  const double height = map->height > 0 ? static_cast<double>(map->height) : 1.0;
  const double lon = map->lon + (x - width * 0.5) / width;
  const double lat = map->lat - (y - height * 0.5) / height;
  char buf[256];
  std::snprintf(buf, sizeof(buf),
                "{\"request\":%llu,\"lon\":%.8f,\"lat\":%.8f}",
                static_cast<unsigned long long>(request_id), lon, lat);
  map->last_query_json = buf;
  if (map->notify_target && map->notify_message)
    PostMessage(map->notify_target, map->notify_message,
                static_cast<WPARAM>(request_id), 0);
}
