// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#ifndef THIRD_PARTY_MAPLIBRE_MVIEW_MAP_MAP_SURFACE_H_
#define THIRD_PARTY_MAPLIBRE_MVIEW_MAP_MAP_SURFACE_H_

// Child HWND that shows either a MapLibre Native frame or an OpenGL mercator
// fallback. Present returns 2, 1, or 0. Headless callers never construct it.

#include "third_party/maplibre/mview_map/mview_map.h"

class MapSurface {
 public:
  MapSurface();
  ~MapSurface();

  // 2 = MapLibre draw+blit, 1 = OpenGL mercator, 0 = nothing was presented.
  int Present(HWND parent,
              int width,
              int height,
              double lon,
              double lat,
              double zoom,
              double bearing,
              double pitch,
              MvmPointerFn callback,
              void* user);
  void Resize(int width, int height);
  void Notify(int kind,
              int action,
              double x,
              double y,
              double wheel_dx,
              double wheel_dy);
  unsigned token() const { return token_; }
  int draw_calls() const { return draw_calls_; }

 private:
  bool EnsureChild(HWND parent, int width, int height);
  int PresentMapLibre(int width,
                      int height,
                      double lon,
                      double lat,
                      double zoom,
                      double bearing,
                      double pitch);
  int PresentMercator(int width,
                      int height,
                      double lon,
                      double lat,
                      double zoom);

  HWND child_;
  HDC dc_;
  HGLRC gl_;
  bool in_present_;
  bool maplibre_disabled_;
  HMODULE maplibre_;
  void* session_;
  int (*open_)(void**, int, int, float);
  int (*render_)(void*, HWND, int, int, double, double, double, double, double,
                 unsigned*, int*);
  void (*close_)(void*);
  MvmPointerFn callback_;
  void* user_;
  unsigned token_;
  int draw_calls_;
  int width_;
  int height_;

  MapSurface(const MapSurface&) = delete;
  MapSurface& operator=(const MapSurface&) = delete;

  friend LRESULT CALLBACK MapSurfaceProc(HWND hwnd,
                                         UINT msg,
                                         WPARAM wparam,
                                         LPARAM lparam);
};

#endif  // THIRD_PARTY_MAPLIBRE_MVIEW_MAP_MAP_SURFACE_H_
