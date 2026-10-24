// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "vista/pass/map/map_pass.h"

#include "vista/pass/map/camera_follow.h"
#include "vista/pass/map/map_surface.h"

#include <cmath>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <new>
#include <sstream>
#include <string>
#include <vector>

namespace vista {
namespace {

const int kJsonCap = 64 * 1024;
const int kStyleCap = 1 << 20;

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

bool Finite(double value) {
  return std::isfinite(value) != 0;
}

}  // namespace

struct MapPass::Impl {
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
  MapPointerFn pointer_fn;
  void* pointer_user;

  Impl()
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

MapPass::MapPass() : impl_(NULL) {}

MapPass::~MapPass() {
  Destroy();
}

bool MapPass::Create(HWND parent, int width, int height, double pixel_ratio) {
  Destroy();
  impl_ = new (std::nothrow) Impl();
  if (!impl_)
    return false;
  impl_->parent = parent;
  impl_->width = width;
  impl_->height = height;
  if (Finite(pixel_ratio) && pixel_ratio > 0.0)
    impl_->pixel_ratio = pixel_ratio;
  {
    MapCameraState state;
    state.lon = impl_->lon;
    state.lat = impl_->lat;
    state.zoom = impl_->zoom;
    state.bearing = impl_->bearing;
    state.pitch = impl_->pitch;
    ViewCameraState view;
    Follow2dMapToView(state, &view);
    impl_->height_m = view.height_m;
  }
  return true;
}

void MapPass::Destroy() {
  if (!impl_)
    return;
  delete impl_->surface;
  impl_->surface = NULL;
  delete impl_;
  impl_ = NULL;
}

void MapPass::Resize(int width, int height, double pixel_ratio) {
  if (!impl_)
    return;
  impl_->width = width;
  impl_->height = height;
  if (Finite(pixel_ratio) && pixel_ratio > 0.0)
    impl_->pixel_ratio = pixel_ratio;
  if (impl_->surface)
    impl_->surface->Resize(width, height);
}

void MapPass::SetPointerCallback(MapPointerFn fn, void* user) {
  if (!impl_)
    return;
  impl_->pointer_fn = fn;
  impl_->pointer_user = user;
}

int MapPass::Present() {
  if (!impl_ || !impl_->parent)
    return 0;
  if (!impl_->surface)
    impl_->surface = new (std::nothrow) MapSurface();
  if (!impl_->surface)
    return 0;
  return impl_->surface->Present(
      impl_->parent, impl_->width, impl_->height, impl_->lon, impl_->lat,
      impl_->zoom, impl_->bearing, impl_->pitch, impl_->pointer_fn,
      impl_->pointer_user);
}

unsigned MapPass::FrameToken() const {
  if (!impl_ || !impl_->surface)
    return 0;
  return impl_->surface->token();
}

int MapPass::LastDrawCalls() const {
  if (!impl_ || !impl_->surface)
    return 0;
  return impl_->surface->draw_calls();
}

HWND MapPass::hwnd() const {
  if (!impl_ || !impl_->surface)
    return NULL;
  return impl_->surface->hwnd();
}

void MapPass::SetNotify(HWND target, UINT message) {
  if (!impl_)
    return;
  impl_->notify_target = target;
  impl_->notify_message = message;
}

void MapPass::SetStyleUrl(const char* url) {
  if (!impl_)
    return;
  impl_->layers.clear();
  impl_->style_url = StripFileUrl(url);
  if (impl_->style_url.empty())
    return;
  const std::string json = ReadFileLimited(impl_->style_url);
  if (json.empty())
    return;
  ParseLayers(json, &impl_->layers);
}

void MapPass::SetCamera(double lon,
                        double lat,
                        double zoom,
                        double bearing,
                        double pitch) {
  if (!impl_)
    return;
  if (!Finite(lon) || !Finite(lat) || !Finite(zoom) || !Finite(bearing) ||
      !Finite(pitch))
    return;
  impl_->lon = lon;
  impl_->lat = lat;
  impl_->zoom = zoom;
  impl_->bearing = bearing;
  impl_->pitch = pitch;
  {
    MapCameraState state;
    state.lon = impl_->lon;
    state.lat = impl_->lat;
    state.zoom = impl_->zoom;
    state.bearing = impl_->bearing;
    state.pitch = impl_->pitch;
    ViewCameraState view;
    Follow2dMapToView(state, &view);
    impl_->height_m = view.height_m;
  }
}

void MapPass::GetCamera(double* lon,
                        double* lat,
                        double* zoom,
                        double* bearing,
                        double* pitch) const {
  if (!impl_ || !lon || !lat || !zoom || !bearing || !pitch)
    return;
  *lon = impl_->lon;
  *lat = impl_->lat;
  *zoom = impl_->zoom;
  *bearing = impl_->bearing;
  *pitch = impl_->pitch;
}

void MapPass::SetLayerVisible(const char* layer_id, int visible) {
  if (!impl_)
    return;
  Layer* layer = FindLayer(&impl_->layers, layer_id);
  if (!layer)
    return;
  layer->visible = visible ? 1 : 0;
}

int MapPass::ListLayers(char* json, int cap) const {
  if (!impl_)
    return -1;
  const std::string body = BuildLayerJson(impl_->layers);
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

void MapPass::AddGeoJson(const char* source_id, const wchar_t* path) {
  if (!impl_ || !source_id || !source_id[0])
    return;
  GeoSource source;
  source.id = source_id;
  if (path)
    source.path = path;
  impl_->sources.push_back(source);
  if (FindLayer(&impl_->layers, source_id))
    return;
  Layer layer;
  layer.id = source_id;
  layer.type = "geojson";
  layer.visible = 1;
  impl_->layers.push_back(layer);
}

void MapPass::QueryPoint(double x, double y, uint64_t request_id) {
  if (!impl_)
    return;
  const double width =
      impl_->width > 0 ? static_cast<double>(impl_->width) : 1.0;
  const double height =
      impl_->height > 0 ? static_cast<double>(impl_->height) : 1.0;
  const double lon = impl_->lon + (x - width * 0.5) / width;
  const double lat = impl_->lat - (y - height * 0.5) / height;
  char buf[256];
  std::snprintf(buf, sizeof(buf),
                "{\"request\":%llu,\"lon\":%.8f,\"lat\":%.8f}",
                static_cast<unsigned long long>(request_id), lon, lat);
  impl_->last_query_json = buf;
  if (impl_->notify_target && impl_->notify_message)
    PostMessage(impl_->notify_target, impl_->notify_message,
                static_cast<WPARAM>(request_id), 0);
}

}  // namespace vista
