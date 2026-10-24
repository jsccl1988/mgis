// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "content/browser/viewport/present/flycube_presenter.h"

#include <windows.h>

#include <string>

#include "content/browser/viewport/present/flycube_host.h"
#include "content/public/browser/trace_log.h"
#include "content/public/browser/view_host.h"

namespace content {
namespace {

bool TryHooksCreate(void* parent_hwnd,
                    int width,
                    int height,
                    void** child_out,
                    bool* hooks_owned_out) {
  *child_out = NULL;
  *hooks_owned_out = false;
  ViewBackendHooks hooks = ViewHost::GetBackendHooks(kViewBackendFlyCube);
  if (!hooks.create)
    return false;
  void* child = hooks.create(parent_hwnd, width, height, hooks.user);
  if (!child)
    return false;
  *child_out = child;
  *hooks_owned_out = true;
  if (hooks.present)
    hooks.present(child, hooks.user);
  return true;
}

}  // namespace

FlyCubePresenter::FlyCubePresenter()
    : child_(NULL),
      hooks_owned_(false),
      width_(0),
      height_(0) {}

FlyCubePresenter::~FlyCubePresenter() {
  DestroyChild();
}

void FlyCubePresenter::DestroyChild() {
  if (host_.get()) {
    host_.reset();
    child_ = NULL;
    hooks_owned_ = false;
    return;
  }
  if (!child_)
    return;
  if (hooks_owned_) {
    ViewBackendHooks hooks = ViewHost::GetBackendHooks(kViewBackendFlyCube);
    if (hooks.destroy)
      hooks.destroy(child_, hooks.user);
  }
  child_ = NULL;
  hooks_owned_ = false;
}

bool FlyCubePresenter::Create(void* parent_hwnd,
                              int width,
                              int height,
                              TraceLog* trace) {
  if (!parent_hwnd)
    return false;
  width_ = width < 2 ? 2 : width;
  height_ = height < 2 ? 2 : height;

  void* hooked = NULL;
  bool owned = false;
  if (TryHooksCreate(parent_hwnd, width_, height_, &hooked, &owned)) {
    DestroyChild();
    child_ = hooked;
    hooks_owned_ = owned;
  } else {
    DestroyChild();
    scoped_ptr<FlyCubeHost> host(new FlyCubeHost());
    if (!host->Create(reinterpret_cast<HWND>(parent_hwnd), width_, height_))
      return false;
    child_ = host->hwnd();
    host_.reset(host.release());
    hooks_owned_ = false;
  }

  if (trace) {
    trace->Append(kTraceConsole, "presenter.3d flycube");
    trace->Append(kTraceConsole, "presenter.fallback flycube");
    if (host_.get()) {
      std::string detail = std::string("presenter.flycube_backend ") +
                           host_->backend();
      trace->Append(kTraceConsole, detail);
    }
  }
  Present();
  return true;
}

void* FlyCubePresenter::hwnd() const {
  if (host_.get())
    return host_->hwnd();
  return child_;
}

void FlyCubePresenter::Resize(int width, int height) {
  width_ = width < 2 ? 2 : width;
  height_ = height < 2 ? 2 : height;
  if (host_.get()) {
    host_->Resize(width_, height_);
    return;
  }
  if (!child_)
    return;
  if (hooks_owned_) {
    ViewBackendHooks hooks = ViewHost::GetBackendHooks(kViewBackendFlyCube);
    if (hooks.resize)
      hooks.resize(child_, width_, height_, hooks.user);
  }
}

void FlyCubePresenter::Present() {
  if (host_.get()) {
    host_->Present();
    return;
  }
  if (!child_)
    return;
  if (hooks_owned_) {
    ViewBackendHooks hooks = ViewHost::GetBackendHooks(kViewBackendFlyCube);
    if (hooks.present)
      hooks.present(child_, hooks.user);
  }
}

void FlyCubePresenter::Orbit(double dx_px, double dy_px) {
  if (host_.get()) {
    host_->Orbit(dx_px, dy_px);
    host_->Present();
    return;
  }
  Present();
}

void FlyCubePresenter::Zoom(double wheel_dy) {
  if (host_.get()) {
    host_->Zoom(wheel_dy);
    host_->Present();
  }
}

void FlyCubePresenter::SetPointerCallback(PointerFn fn, void* user) {
  if (host_.get())
    host_->SetPointerCallback(fn, user);
}

void FlyCubePresenter::Camera(double* lon, double* lat, double* height) const {
  if (host_.get()) {
    host_->Camera(lon, lat, height);
    return;
  }
  if (lon)
    *lon = 105.0;
  if (lat)
    *lat = 35.0;
  if (height)
    *height = 1.2e7;
}

void FlyCubePresenter::SetCamera(double lon, double lat, double height) {
  if (host_.get()) {
    host_->SetCamera(lon, lat, height);
    host_->Present();
  }
}

std::string FlyCubePresenter::BackendName() const {
  return "flycube";
}

}  // namespace content
