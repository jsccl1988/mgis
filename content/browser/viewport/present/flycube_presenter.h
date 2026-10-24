// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#ifndef CONTENT_BROWSER_VIEWPORT_PRESENT_FLYCUBE_PRESENTER_H_
#define CONTENT_BROWSER_VIEWPORT_PRESENT_FLYCUBE_PRESENTER_H_

#include "base/memory/scoped_ptr.h"
#include "content/browser/viewport/present/scene_presenter.h"

namespace content {

class FlyCubeHost;

// SCENE3D offline fallback after UE pipe failure.
// Prefer ViewHost FlyCube hooks when the shell registered them; else a
// content-safe FlyCubeHost (vista::WorldPass DX12 when Create succeeds, else
// real OpenGL globe). No navy GDI stub.
class FlyCubePresenter : public ScenePresenter {
 public:
  FlyCubePresenter();
  ~FlyCubePresenter() override;

  bool Create(void* parent_hwnd,
              int width,
              int height,
              TraceLog* trace) override;
  void* hwnd() const override;
  void Resize(int width, int height) override;
  void Present() override;
  void Orbit(double dx_px, double dy_px) override;
  void Zoom(double wheel_dy) override;
  void SetPointerCallback(PointerFn fn, void* user) override;
  void Camera(double* lon, double* lat, double* height) const override;
  void SetCamera(double lon, double lat, double height) override;
  std::string BackendName() const override;

 private:
  FlyCubePresenter(const FlyCubePresenter&) = delete;
  FlyCubePresenter& operator=(const FlyCubePresenter&) = delete;

  void DestroyChild();

  void* child_;
  bool hooks_owned_;
  scoped_ptr<FlyCubeHost> host_;
  int width_;
  int height_;
};

}  // namespace content

#endif  // CONTENT_BROWSER_VIEWPORT_PRESENT_FLYCUBE_PRESENTER_H_
