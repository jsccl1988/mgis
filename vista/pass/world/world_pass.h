// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#ifndef VISTA_PASS_WORLD_WORLD_PASS_H_
#define VISTA_PASS_WORLD_WORLD_PASS_H_

// 3D world present pathway: cesium_flycube + FlyCube DX12 child HWND.
// C++ product surface for content presenters (replaces mvf_* C ABI).

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include "vista/vista_export.h"

namespace vista {

class VISTA_EXPORT WorldPass {
 public:
  WorldPass();
  ~WorldPass();

  bool Create(HWND parent, int width, int height);
  void Destroy();

  HWND hwnd() const;
  void Resize(int width, int height);
  // True when a non-stub frame was presented.
  bool Present();
  void Orbit(double dx_px, double dy_px);
  void SetCamera(double lon, double lat, double height_m);
  void GetCamera(double* lon, double* lat, double* height_m) const;

 private:
  struct WorldPassImpl* impl_;

  WorldPass(const WorldPass&) = delete;
  WorldPass& operator=(const WorldPass&) = delete;
};

}  // namespace vista

#endif  // VISTA_PASS_WORLD_WORLD_PASS_H_
