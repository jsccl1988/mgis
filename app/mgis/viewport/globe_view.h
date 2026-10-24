// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#ifndef APP_MGIS_VIEWPORT_GLOBE_VIEW_H_
#define APP_MGIS_VIEWPORT_GLOBE_VIEW_H_

#include <windows.h>

// In-process 3D browse surface. Draws a lit textured sphere with OpenGL into
// a child HWND. Drag orbits and the wheel changes distance.
typedef void (*GlobePointerFn)(void* user,
                               int kind,
                               int action,
                               double x,
                               double y,
                               double wheel_dx,
                               double wheel_dy);

class GlobeView {
 public:
  GlobeView();
  ~GlobeView();

  bool Attach(HWND parent, int width, int height, double yaw_radians);
  void Resize(int width, int height);
  void SetDrawSize(int width, int height);
  bool attached() const { return child_ != NULL; }

  // True only after a mesh draw, a non-flat readback, and SwapBuffers.
  bool Present();
  void Notify(int kind,
              int action,
              double x,
              double y,
              double wheel_dx,
              double wheel_dy);
  void Orbit(double dx_px, double dy_px);
  void Zoom(double wheel_dy);
  void SetPointerCallback(GlobePointerFn fn, void* user);
  HWND hwnd() const { return child_; }
  unsigned frame_token() const { return token_; }
  int draw_calls() const { return draw_calls_; }

 private:
  bool EnsureGl();

  HWND child_;
  HDC dc_;
  HGLRC gl_;
  bool in_present_;
  bool mesh_ready_;
  double yaw_;
  double pitch_;
  double distance_;
  int width_;
  int height_;
  unsigned token_;
  int draw_calls_;
  GlobePointerFn callback_;
  void* user_;

  GlobeView(const GlobeView&) = delete;
  GlobeView& operator=(const GlobeView&) = delete;

  friend LRESULT CALLBACK GlobeViewProc(HWND hwnd,
                                        UINT msg,
                                        WPARAM wparam,
                                        LPARAM lparam);
};

#endif  // APP_MGIS_VIEWPORT_GLOBE_VIEW_H_
