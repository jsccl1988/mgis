// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#ifndef CONTENT_BROWSER_VIEWPORT_PRESENT_MAPLIBRE_PRESENTER_H_
#define CONTENT_BROWSER_VIEWPORT_PRESENT_MAPLIBRE_PRESENTER_H_

#include "content/browser/viewport/present/scene_presenter.h"
#include "vista/pass/map/map_pass.h"

namespace content {

// SCENE2D presenter: MapLibre Native when the DLL loads, else OpenGL mercator.
// Talks to vista::MapPass (C++ product surface).
class MapLibrePresenter : public ScenePresenter {
 public:
  MapLibrePresenter();
  ~MapLibrePresenter() override;

  bool Create(void* parent_hwnd,
              int width,
              int height,
              TraceLog* trace) override;
  void* hwnd() const override;
  void Resize(int width, int height) override;
  void Present() override;
  void Pan(double dx_px, double dy_px) override;
  void Zoom(double wheel_dy) override;
  void Tilt(double dx_px, double dy_px) override;
  void SetPointerCallback(PointerFn fn, void* user) override;
  void Camera(double* lon, double* lat, double* height) const override;
  void SetCamera(double lon, double lat, double height) override;
  std::string BackendName() const override;

 private:
  MapLibrePresenter(const MapLibrePresenter&) = delete;
  MapLibrePresenter& operator=(const MapLibrePresenter&) = delete;

  vista::MapPass* map_;
  int last_present_kind_;  // 2=maplibre, 1=mercator, 0=none
  bool fallback_traced_;
};

}  // namespace content

#endif  // CONTENT_BROWSER_VIEWPORT_PRESENT_MAPLIBRE_PRESENTER_H_
