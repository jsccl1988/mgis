// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#ifndef CONTENT_BROWSER_VIEWPORT_PRESENT_FLYCUBE_HOST_H_
#define CONTENT_BROWSER_VIEWPORT_PRESENT_FLYCUBE_HOST_H_

#include <windows.h>

#include "vista/pass/world/world_pass.h"

namespace content {

// Content-safe 3D child host for FlyCubePresenter. Prefers vista::WorldPass
// (cesium_flycube DX12). Falls back to a lit OpenGL sphere when WorldPass
// Create fails.
class FlyCubeHost {
 public:
  FlyCubeHost();
  ~FlyCubeHost();

  bool Create(HWND parent, int width, int height);
  void Destroy();
  HWND hwnd() const;
  void Resize(int width, int height);
  // True after a successful WorldPass present, or GL mesh draw + SwapBuffers.
  bool Present();
  void Orbit(double dx_px, double dy_px);
  void Zoom(double wheel_dy);
  void Camera(double* lon, double* lat, double* height_m) const;
  void SetCamera(double lon, double lat, double height_m);
  typedef void (*PointerFn)(void* user,
                            int kind,
                            int action,
                            double x,
                            double y,
                            double wheel_dx,
                            double wheel_dy);
  void SetPointerCallback(PointerFn fn, void* user);
  // "flycube-dx12" or "flycube-gl".
  const char* backend() const { return backend_; }

 private:
  friend LRESULT CALLBACK FlyCubeHostProc(HWND hwnd,
                                          UINT msg,
                                          WPARAM wparam,
                                          LPARAM lparam);
  void NotifyPointer(int kind,
                     int action,
                     double x,
                     double y,
                     double wheel_dx,
                     double wheel_dy);
  bool TryWorldCreate(HWND parent, int width, int height);
  void DestroyWorld();
  bool PresentWorld();
  bool EnsureGl();
  bool PresentGl();

  HWND child_;
  HDC dc_;
  HGLRC gl_;
  bool in_present_;
  bool mesh_ready_;
  bool dx12_;
  vista::WorldPass* world_;
  double lon_;
  double lat_;
  double height_m_;
  double yaw_;
  double pitch_;
  double distance_;
  int width_;
  int height_;
  unsigned token_;
  const char* backend_;
  PointerFn pointer_fn_;
  void* pointer_user_;
  bool right_down_;

  FlyCubeHost(const FlyCubeHost&) = delete;
  FlyCubeHost& operator=(const FlyCubeHost&) = delete;
};

}  // namespace content

#endif  // CONTENT_BROWSER_VIEWPORT_PRESENT_FLYCUBE_HOST_H_
