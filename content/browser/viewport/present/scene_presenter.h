// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#ifndef CONTENT_BROWSER_VIEWPORT_PRESENT_SCENE_PRESENTER_H_
#define CONTENT_BROWSER_VIEWPORT_PRESENT_SCENE_PRESENTER_H_

#include <string>

namespace content {

class TraceLog;

// One mode's child HWND owner. SceneImpl keeps one presenter per mode.
class ScenePresenter {
 public:
  virtual ~ScenePresenter() {}

  virtual bool Create(void* parent_hwnd,
                      int width,
                      int height,
                      TraceLog* trace) = 0;
  virtual void* hwnd() const = 0;
  virtual void Resize(int width, int height) = 0;
  virtual void Present() = 0;

  // Optional navigation. Default no-ops return true when the mode matches.
  virtual void Pan(double dx_px, double dy_px) {}
  virtual void Zoom(double wheel_dy) {}
  virtual void Orbit(double dx_px, double dy_px) {}
  // Optional 2D pitch/bearing tilt (right-drag). Default no-op.
  virtual void Tilt(double dx_px, double dy_px) {
    (void)dx_px;
    (void)dy_px;
  }

  // Child HWND pointer samples (absolute client px). kind 0=mouse 1=wheel;
  // action 1=move 2=press 3=release. wheel_dx!=0 marks right-button path.
  typedef void (*PointerFn)(void* user,
                            int kind,
                            int action,
                            double x,
                            double y,
                            double wheel_dx,
                            double wheel_dy);
  virtual void SetPointerCallback(PointerFn fn, void* user) {
    (void)fn;
    (void)user;
  }

  virtual void Camera(double* lon, double* lat, double* height) const {
    if (lon)
      *lon = 0;
    if (lat)
      *lat = 0;
    if (height)
      *height = 0;
  }
  virtual void SetCamera(double lon, double lat, double height) {
    (void)lon;
    (void)lat;
    (void)height;
  }

  virtual std::string BackendName() const = 0;
};

}  // namespace content

#endif  // CONTENT_BROWSER_VIEWPORT_PRESENT_SCENE_PRESENTER_H_
