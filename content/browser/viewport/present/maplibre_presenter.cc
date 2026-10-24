// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "content/browser/viewport/present/maplibre_presenter.h"

#include <windows.h>

#include "content/public/browser/trace_log.h"

namespace content {
namespace {

double HeightFromZoom(double zoom) {
  return (20.0 - zoom) * 1000.0;
}

double ZoomFromHeight(double height) {
  if (height <= 0)
    return 10.0;
  double zoom = 20.0 - (height / 1000.0);
  if (zoom < 1.0)
    return 1.0;
  if (zoom > 20.0)
    return 20.0;
  return zoom;
}

}  // namespace

MapLibrePresenter::MapLibrePresenter()
    : map_(NULL), last_present_kind_(0), fallback_traced_(false) {}

MapLibrePresenter::~MapLibrePresenter() {
  if (map_) {
    delete map_;
    map_ = NULL;
  }
}

bool MapLibrePresenter::Create(void* parent_hwnd,
                               int width,
                               int height,
                               TraceLog* trace) {
  if (!parent_hwnd)
    return false;
  if (width < 2)
    width = 2;
  if (height < 2)
    height = 2;
  map_ = new vista::MapPass();
  if (!map_->Create(reinterpret_cast<HWND>(parent_hwnd), width, height, 1.0)) {
    delete map_;
    map_ = NULL;
    return false;
  }
  // China overview: ~105E / 35N; zoom ~3.8 keeps water around the mainland
  // so the offline China fill is not a full-viewport green slab.
  map_->SetCamera(105.0, 35.0, 3.8, 0.0, 0.0);
  Present();
  if (!map_->hwnd()) {
    delete map_;
    map_ = NULL;
    return false;
  }
  if (trace) {
    if (last_present_kind_ == 2)
      trace->Append(kTraceConsole, "presenter.2d maplibre");
    else {
      trace->Append(kTraceConsole, "presenter.2d mercator");
      if (!fallback_traced_) {
        trace->Append(kTraceConsole, "presenter.fallback mercator");
        fallback_traced_ = true;
      }
    }
  }
  return true;
}

void* MapLibrePresenter::hwnd() const {
  return map_ ? map_->hwnd() : NULL;
}

void MapLibrePresenter::Resize(int width, int height) {
  if (!map_)
    return;
  if (width < 2)
    width = 2;
  if (height < 2)
    height = 2;
  map_->Resize(width, height, 1.0);
}

void MapLibrePresenter::Present() {
  if (!map_)
    return;
  last_present_kind_ = map_->Present();
}

void MapLibrePresenter::Pan(double dx_px, double dy_px) {
  if (!map_)
    return;
  double lon = 0;
  double lat = 0;
  double zoom = 0;
  double bearing = 0;
  double pitch = 0;
  map_->GetCamera(&lon, &lat, &zoom, &bearing, &pitch);
  double denom = 256.0;
  for (int i = 0; i < static_cast<int>(zoom) && i < 20; ++i)
    denom *= 2.0;
  const double scale = 360.0 / denom;
  lon -= dx_px * scale;
  lat += dy_px * scale;
  if (lat > 85.0)
    lat = 85.0;
  if (lat < -85.0)
    lat = -85.0;
  map_->SetCamera(lon, lat, zoom, bearing, pitch);
  Present();
}

void MapLibrePresenter::Zoom(double wheel_dy) {
  if (!map_)
    return;
  double lon = 0;
  double lat = 0;
  double zoom = 0;
  double bearing = 0;
  double pitch = 0;
  map_->GetCamera(&lon, &lat, &zoom, &bearing, &pitch);
  zoom += wheel_dy > 0 ? 0.25 : -0.25;
  if (zoom < 1.0)
    zoom = 1.0;
  if (zoom > 20.0)
    zoom = 20.0;
  map_->SetCamera(lon, lat, zoom, bearing, pitch);
  Present();
}

void MapLibrePresenter::Tilt(double dx_px, double dy_px) {
  if (!map_)
    return;
  double lon = 0;
  double lat = 0;
  double zoom = 0;
  double bearing = 0;
  double pitch = 0;
  map_->GetCamera(&lon, &lat, &zoom, &bearing, &pitch);
  bearing += dx_px * 0.25;
  pitch -= dy_px * 0.15;
  if (pitch < 0.0)
    pitch = 0.0;
  if (pitch > 60.0)
    pitch = 60.0;
  map_->SetCamera(lon, lat, zoom, bearing, pitch);
  Present();
}

void MapLibrePresenter::SetPointerCallback(PointerFn fn, void* user) {
  if (!map_)
    return;
  map_->SetPointerCallback(fn, user);
}

void MapLibrePresenter::Camera(double* lon, double* lat, double* height) const {
  if (!map_)
    return;
  double zoom = 0;
  double bearing = 0;
  double pitch = 0;
  double cur_lon = 0;
  double cur_lat = 0;
  map_->GetCamera(&cur_lon, &cur_lat, &zoom, &bearing, &pitch);
  if (lon)
    *lon = cur_lon;
  if (lat)
    *lat = cur_lat;
  if (height)
    *height = HeightFromZoom(zoom);
}

void MapLibrePresenter::SetCamera(double lon, double lat, double height) {
  if (!map_)
    return;
  map_->SetCamera(lon, lat, ZoomFromHeight(height), 0.0, 0.0);
  Present();
}

std::string MapLibrePresenter::BackendName() const {
  return last_present_kind_ == 2 ? "maplibre" : "mercator";
}

}  // namespace content
