// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#ifndef CONTENT_BROWSER_VIEWPORT_PRESENT_UE_PRESENTER_H_
#define CONTENT_BROWSER_VIEWPORT_PRESENT_UE_PRESENTER_H_

#include "base/memory/scoped_ptr.h"
#include "content/browser/viewport/present/scene_presenter.h"

namespace content {

class PipeClient;

// SCENE3D UE path. Connects to \\.\pipe\mview. Real game HWND reparent is
// deferred; this knife hosts a labeled child and forwards camera.set.
class UePresenter : public ScenePresenter {
 public:
  UePresenter();
  ~UePresenter() override;

  // Tries the named pipe. On failure returns false without creating a child.
  bool TryConnect(TraceLog* trace);

  bool Create(void* parent_hwnd,
              int width,
              int height,
              TraceLog* trace) override;
  void* hwnd() const override;
  void Resize(int width, int height) override;
  void Present() override;
  void Orbit(double dx_px, double dy_px) override;
  void Camera(double* lon, double* lat, double* height) const override;
  void SetCamera(double lon, double lat, double height) override;
  std::string BackendName() const override;

 private:
  UePresenter(const UePresenter&) = delete;
  UePresenter& operator=(const UePresenter&) = delete;

  void SendCameraSet();

  scoped_ptr<PipeClient> pipe_;
  void* child_;
  int width_;
  int height_;
  double lon_;
  double lat_;
  double height_m_;
  double orbit_delta_;
};

}  // namespace content

#endif  // CONTENT_BROWSER_VIEWPORT_PRESENT_UE_PRESENTER_H_
