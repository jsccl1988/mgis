// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#ifndef VISTA_PASS_MAP_MAP_PASS_H_
#define VISTA_PASS_MAP_MAP_PASS_H_

// 2D map present pathway: MapLibre Native when available, else OpenGL mercator.
// C++ product surface for content presenters (replaces mvm_* C ABI).

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include <stdint.h>

#include "vista/vista_export.h"

namespace vista {

// kind 0 = mouse, 1 = wheel. action 1 = move, 2 = press, 3 = release.
typedef void (*MapPointerFn)(void* user,
                             int kind,
                             int action,
                             double x,
                             double y,
                             double wheel_dx,
                             double wheel_dy);

class VISTA_EXPORT MapPass {
 public:
  MapPass();
  ~MapPass();

  // parent may be NULL for headless camera/style tests.
  bool Create(HWND parent, int width, int height, double pixel_ratio);
  void Destroy();

  void SetPointerCallback(MapPointerFn fn, void* user);

  // 2 = MapLibre, 1 = OpenGL mercator, 0 = no draw.
  int Present();
  unsigned FrameToken() const;
  int LastDrawCalls() const;
  HWND hwnd() const;
  void Resize(int width, int height, double pixel_ratio);
  void SetNotify(HWND target, UINT message);

  void SetStyleUrl(const char* url);
  void SetCamera(double lon,
                 double lat,
                 double zoom,
                 double bearing,
                 double pitch);
  void GetCamera(double* lon,
                 double* lat,
                 double* zoom,
                 double* bearing,
                 double* pitch) const;
  void SetLayerVisible(const char* layer_id, int visible);
  int ListLayers(char* json, int cap) const;
  void AddGeoJson(const char* source_id, const wchar_t* path);
  void QueryPoint(double x, double y, uint64_t request_id);

 private:
  struct Impl;
  Impl* impl_;

  MapPass(const MapPass&) = delete;
  MapPass& operator=(const MapPass&) = delete;
};

}  // namespace vista

#endif  // VISTA_PASS_MAP_MAP_PASS_H_
