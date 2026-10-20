// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "app/mgis/flycube_bridge.h"
#include "app/mgis/globe_view.h"

#include "content/public/types.h"
#include "content/public/view_host.h"

namespace mgis {
namespace {

struct FlySlot {
  FlySlot() : globe(NULL), dragging(false), last_x(0.0), last_y(0.0) {}

  GlobeView* globe;
  bool dragging;
  double last_x;
  double last_y;
};

FlySlot g_slot;
content::ViewHost* g_host = NULL;

const int kKindMouse = 0;
const int kKindWheel = 1;
const int kActionMove = 1;
const int kActionPress = 2;
const int kActionRelease = 3;

void OnFlyPointer(void* user,
                  int kind,
                  int action,
                  double x,
                  double y,
                  double wheel_dx,
                  double wheel_dy) {
  (void)user;
  (void)wheel_dx;
  if (!g_host || !g_slot.globe)
    return;
  content::InputEvent event;
  event.view_id = g_host->view_id();
  event.x = static_cast<int>(x);
  event.y = static_cast<int>(y);
  if (kind == kKindWheel) {
    event.kind = content::kInputEventWheel;
    event.wheel_delta = static_cast<int>(wheel_dy * 120.0);
  } else {
    event.kind = content::kInputEventMouse;
    if (action == kActionPress)
      event.mouse_action = content::kMouseDown;
    else if (action == kActionRelease)
      event.mouse_action = content::kMouseUp;
    else
      event.mouse_action = content::kMouseMove;
  }
  const bool consumed = g_host->NotifyGesture(event);
  if (!consumed)
    return;
  if (kind == kKindWheel) {
    g_slot.globe->Zoom(wheel_dy);
  } else if (action == kActionPress) {
    g_slot.dragging = true;
    g_slot.last_x = x;
    g_slot.last_y = y;
  } else if (action == kActionMove && g_slot.dragging) {
    g_slot.globe->Orbit(x - g_slot.last_x, y - g_slot.last_y);
    g_slot.last_x = x;
    g_slot.last_y = y;
  } else if (action == kActionRelease) {
    g_slot.dragging = false;
  }
  g_slot.globe->Present();
}

void* CreateFlyCube(void* parent_hwnd, int width, int height, void* user) {
  (void)user;
  if (g_slot.globe) {
    delete g_slot.globe;
    g_slot.globe = NULL;
  }
  g_host = content::ViewHost::AttachingHost();
  GlobeView* globe = new GlobeView();
  if (!globe->Attach(reinterpret_cast<HWND>(parent_hwnd), width, height, 0.35)) {
    delete globe;
    g_host = NULL;
    return NULL;
  }
  globe->SetPointerCallback(&OnFlyPointer, &g_slot);
  g_slot.globe = globe;
  g_slot.dragging = false;
  return globe->hwnd();
}

void ResizeFlyCube(void* child, int width, int height, void* user) {
  (void)child;
  (void)user;
  if (!g_slot.globe)
    return;
  g_slot.globe->SetDrawSize(width, height);
  g_slot.globe->Present();
}

void DestroyFlyCube(void* child, void* user) {
  (void)child;
  (void)user;
  delete g_slot.globe;
  g_slot.globe = NULL;
  g_host = NULL;
}

void PresentFlyCube(void* child, void* user) {
  (void)child;
  (void)user;
  if (g_slot.globe)
    g_slot.globe->Present();
}

}  // namespace

void InstallFlyCubeBackend() {
  content::ViewBackendHooks hooks;
  hooks.create = &CreateFlyCube;
  hooks.resize = &ResizeFlyCube;
  hooks.destroy = &DestroyFlyCube;
  hooks.present = &PresentFlyCube;
  content::ViewHost::SetBackendHooks(content::kViewBackendFlyCube, hooks);
}

}  // namespace mgis
