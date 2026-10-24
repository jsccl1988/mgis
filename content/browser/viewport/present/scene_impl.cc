// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "content/browser/viewport/present/scene_impl.h"

#include <cmath>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include "content/browser/viewport/present/flycube_presenter.h"
#include "content/browser/viewport/present/maplibre_presenter.h"
#include "content/browser/viewport/present/scene_presenter.h"
#include "content/browser/viewport/present/ue_presenter.h"
#include "content/public/browser/trace_log.h"
#include "content/public/browser/types.h"

namespace content {
namespace {

const char kBasemapId[] = "basemap";
const char kGroundwaterId[] = "groundwater";

const char* SceneModeLabel(SceneMode mode) {
  if (mode == SceneMode::kScene3D)
    return "scene3d";
  if (mode == SceneMode::kColumbusView)
    return "columbus";
  return "scene2d";
}

bool ToolAllowed(SceneMode mode, const std::string& tool_id) {
  if (tool_id == "view.pan" || tool_id == "view.zoom_in" ||
      tool_id == "view.zoom_out" || tool_id == "wheel.zoom") {
    return mode == SceneMode::kScene2D || mode == SceneMode::kColumbusView;
  }
  if (tool_id == "view3d.trackball")
    return mode == SceneMode::kScene3D;
  if (tool_id == "selection.point" || tool_id == "edit.append.point")
    return mode == SceneMode::kScene2D || mode == SceneMode::kScene3D;
  return false;
}

}  // namespace

SceneImpl::SceneImpl()
    : mode_(SceneMode::kScene2D),
      tool_id_(),
      draft_points_(0),
      trackball_armed_(false),
      orbit_delta_(0),
      width_(0),
      height_(0),
      input_forward_(NULL),
      input_context_(NULL),
      pointer_down_(false),
      pointer_right_(false),
      last_x_(0),
      last_y_(0) {
  primitive_ids_.push_back(kBasemapId);
  primitive_ids_.push_back(kGroundwaterId);
}

SceneImpl::~SceneImpl() {}

bool SceneImpl::Attach(void* parent_hwnd,
                       const std::string& view_id,
                       int width,
                       int height,
                       TraceLog* trace) {
  if (parent_hwnd == NULL || view_id != kViewIdScene) {
    if (trace) {
      trace->Append(kTraceConsole,
                    std::string("backend.unavailable ") + view_id);
    }
    return false;
  }
  width_ = width < 2 ? 2 : width;
  height_ = height < 2 ? 2 : height;

  scoped_ptr<MapLibrePresenter> map(new MapLibrePresenter());
  if (!map->Create(parent_hwnd, width_, height_, trace))
    return false;
  presenter_2d_.reset(map.release());

  scoped_ptr<UePresenter> ue(new UePresenter());
  if (ue->TryConnect(trace) &&
      ue->Create(parent_hwnd, width_, height_, trace)) {
    presenter_3d_.reset(ue.release());
  } else {
    scoped_ptr<FlyCubePresenter> fly(new FlyCubePresenter());
    if (!fly->Create(parent_hwnd, width_, height_, trace))
      return false;
    presenter_3d_.reset(fly.release());
  }

  mode_ = SceneMode::kScene2D;
  BindPresenterPointers();
  Present();
  return true;
}

void SceneImpl::SetInputForwarder(InputForwardFn fn, void* context) {
  input_forward_ = fn;
  input_context_ = context;
  BindPresenterPointers();
}

void SceneImpl::BindPresenterPointers() {
  if (presenter_2d_.get())
    presenter_2d_->SetPointerCallback(&SceneImpl::OnHwndPointer, this);
  if (presenter_3d_.get())
    presenter_3d_->SetPointerCallback(&SceneImpl::OnHwndPointer, this);
}

void SceneImpl::OnHwndPointer(void* user,
                              int kind,
                              int action,
                              double x,
                              double y,
                              double wheel_dx,
                              double wheel_dy) {
  SceneImpl* self = static_cast<SceneImpl*>(user);
  if (!self || !self->input_forward_)
    return;

  const int kKindMouse = 0;
  const int kKindWheel = 1;
  const int kActionMove = 1;
  const int kActionPress = 2;
  const int kActionRelease = 3;

  InputEvent event;
  event.view_id = kViewIdScene;
  if (kind == kKindWheel) {
    event.kind = kInputEventWheel;
    event.x = static_cast<int>(x);
    event.y = static_cast<int>(y);
    event.wheel_delta = wheel_dy > 0 ? 1 : -1;
    self->input_forward_(self->input_context_, event);
    return;
  }
  if (kind != kKindMouse)
    return;

  event.kind = kInputEventMouse;
  event.button = wheel_dx != 0.0 ? 2 : 1;
  if (action == kActionPress) {
    self->pointer_down_ = true;
    self->pointer_right_ = wheel_dx != 0.0;
    self->last_x_ = x;
    self->last_y_ = y;
    event.mouse_action = kMouseDown;
    event.x = static_cast<int>(x);
    event.y = static_cast<int>(y);
    self->input_forward_(self->input_context_, event);
    return;
  }
  if (action == kActionRelease) {
    self->pointer_down_ = false;
    event.mouse_action = kMouseUp;
    event.x = static_cast<int>(x);
    event.y = static_cast<int>(y);
    event.button = self->pointer_right_ ? 2 : 1;
    self->pointer_right_ = false;
    self->input_forward_(self->input_context_, event);
    return;
  }
  if (action == kActionMove && self->pointer_down_) {
    event.mouse_action = kMouseMove;
    // SceneImpl::OnPointer treats x/y as deltas for pan/orbit.
    event.x = static_cast<int>(x - self->last_x_);
    event.y = static_cast<int>(y - self->last_y_);
    event.button = self->pointer_right_ ? 2 : 1;
    self->last_x_ = x;
    self->last_y_ = y;
    if (event.x == 0 && event.y == 0)
      return;
    self->input_forward_(self->input_context_, event);
  }
}

ScenePresenter* SceneImpl::ActivePresenter() const {
  if (mode_ == SceneMode::kScene3D)
    return presenter_3d_.get();
  return presenter_2d_.get();
}

void* SceneImpl::child_hwnd() const {
  ScenePresenter* p = ActivePresenter();
  return p ? p->hwnd() : NULL;
}

void* SceneImpl::child_hwnd_for(SceneMode mode) const {
  if (mode == SceneMode::kScene3D)
    return presenter_3d_ ? presenter_3d_->hwnd() : NULL;
  return presenter_2d_ ? presenter_2d_->hwnd() : NULL;
}

void SceneImpl::SetSceneMode(SceneMode mode, TraceLog* trace) {
  mode_ = mode;
  if (trace) {
    trace->Append(kTraceConsole,
                  std::string("scene.mode ") + SceneModeLabel(mode));
  }
  Present();
}

SceneMode SceneImpl::scene_mode() const {
  return mode_;
}

bool SceneImpl::HasPrimitive(const std::string& id) const {
  for (size_t i = 0; i < primitive_ids_.size(); ++i) {
    if (primitive_ids_[i] == id)
      return true;
  }
  return false;
}

bool SceneImpl::HasLayer(const std::string& id) const {
  return HasPrimitive(id);
}

bool SceneImpl::HasEntity(const std::string& id) const {
  if (mode_ == SceneMode::kScene2D)
    return id == "m0" && HasPrimitive(kBasemapId);
  if (mode_ == SceneMode::kScene3D)
    return id == "g0" && HasPrimitive(kGroundwaterId);
  return false;
}

std::string SceneImpl::Pick(int /*x*/, int /*y*/) const {
  const char* id = NULL;
  const char* feature = NULL;
  if (mode_ == SceneMode::kScene2D) {
    id = kBasemapId;
    feature = "m0";
  } else if (mode_ == SceneMode::kScene3D) {
    id = kGroundwaterId;
    feature = "g0";
  } else {
    return std::string();
  }
  if (!HasPrimitive(id))
    return std::string();
  return std::string("primitive:") + id + ":" + feature;
}

void SceneImpl::Resize(int width, int height) {
  width_ = width < 2 ? 2 : width;
  height_ = height < 2 ? 2 : height;
  // Resize the active presenter always. Also push size to the inactive one
  // when its HWND is still a live window: otherwise FlyCube stays at the
  // tiny create size until the first 3D tab switch, and SyncActiveSurfaceSize
  // alone cannot recover if Attach is skipped. Skip DX12 recreate while the
  // child is parented to the hidden parking HWND (Detach) — that path AVs.
  ScenePresenter* active = ActivePresenter();
  if (active)
    active->Resize(width_, height_);
  ScenePresenter* inactive =
      mode_ == SceneMode::kScene3D ? presenter_2d_.get() : presenter_3d_.get();
  if (!inactive || !inactive->hwnd())
    return;
  HWND hwnd = reinterpret_cast<HWND>(inactive->hwnd());
  if (!IsWindow(hwnd))
    return;
  HWND parent = GetParent(hwnd);
  // ui::GetHiddenWindow() is a WS_POPUP parking parent used by Detach.
  if (!parent || (GetWindowLongPtr(parent, GWL_STYLE) & WS_POPUP))
    return;
  inactive->Resize(width_, height_);
}

void SceneImpl::Present() {
  ScenePresenter* p = ActivePresenter();
  if (p)
    p->Present();
}

void SceneImpl::ArmTrackball() {
  trackball_armed_ = true;
}

void SceneImpl::NotePointer(const InputEvent& event) {
  if (!trackball_armed_ || event.view_id != kViewIdScene)
    return;
  if (event.kind == kInputEventMouse && event.mouse_action == kMouseMove)
    orbit_delta_ = 1;
}

bool SceneImpl::ExpectCamera(bool has_target,
                             double lon,
                             double lat,
                             double height,
                             std::string* error) const {
  ScenePresenter* p = ActivePresenter();
  if (!p || !p->hwnd()) {
    if (error)
      *error = "expect_camera: scene is not attached";
    return false;
  }
  if (mode_ == SceneMode::kScene3D) {
    if (orbit_delta_ <= 0) {
      if (error)
        *error = "expect_camera: orbit delta is not positive";
      return false;
    }
    return true;
  }
  if (!has_target) {
    if (error)
      *error = "expect_camera: needs lon lat height";
    return false;
  }
  double cur_lon = 0;
  double cur_lat = 0;
  double cur_h = 0;
  p->Camera(&cur_lon, &cur_lat, &cur_h);
  const double kDegrees = 1e-6;
  const double kHeightMeters = 1;
  if (std::fabs(cur_lon - lon) > kDegrees ||
      std::fabs(cur_lat - lat) > kDegrees ||
      std::fabs(cur_h - height) > kHeightMeters) {
    if (error)
      *error = "expect_camera: scene camera mismatch";
    return false;
  }
  return true;
}

bool SceneImpl::ActivateTool(const std::string& tool_id,
                             const std::string& view_id) {
  if (view_id != kViewIdScene || !ToolAllowed(mode_, tool_id))
    return false;
  tool_id_ = tool_id;
  if (tool_id == "edit.append.point")
    draft_points_ = 0;
  return true;
}

bool SceneImpl::OnPointer(const InputEvent& event, std::string* mark) {
  if (mark)
    mark->clear();
  if (event.view_id != kViewIdScene || !ToolAllowed(mode_, tool_id_))
    return false;

  ScenePresenter* p = ActivePresenter();
  // Wheel always zooms in 2D while a navigation tool is active.
  if (event.kind == kInputEventWheel && p &&
      (mode_ == SceneMode::kScene2D || mode_ == SceneMode::kColumbusView) &&
      (tool_id_ == "view.pan" || tool_id_ == "wheel.zoom" ||
       tool_id_ == "view.zoom_in" || tool_id_ == "view.zoom_out")) {
    p->Zoom(static_cast<double>(event.wheel_delta));
    return true;
  }
  if (tool_id_ == "view.pan" && event.kind == kInputEventMouse &&
      event.mouse_action == kMouseMove && p) {
    if (event.button == 2) {
      p->Tilt(static_cast<double>(event.x), static_cast<double>(event.y));
      return true;
    }
    p->Pan(static_cast<double>(event.x), static_cast<double>(event.y));
    return true;
  }
  if ((tool_id_ == "wheel.zoom" || tool_id_ == "view.zoom_in" ||
       tool_id_ == "view.zoom_out") &&
      event.kind == kInputEventWheel && p) {
    p->Zoom(static_cast<double>(event.wheel_delta));
    return true;
  }
  if (tool_id_ == "view3d.trackball" && event.kind == kInputEventWheel && p) {
    p->Zoom(static_cast<double>(event.wheel_delta));
    return true;
  }
  if (tool_id_ == "view3d.trackball" && event.kind == kInputEventMouse &&
      event.mouse_action == kMouseMove && p) {
    p->Orbit(static_cast<double>(event.x), static_cast<double>(event.y));
    orbit_delta_ = 1;
    return true;
  }
  if (tool_id_ == "selection.point" && event.kind == kInputEventMouse &&
      event.mouse_action == kMouseDown) {
    const std::string hit = Pick(event.x, event.y);
    if (hit.empty())
      return false;
    if (mark) {
      if (mode_ == SceneMode::kScene2D)
        *mark = "pick.2d";
      else if (mode_ == SceneMode::kScene3D)
        *mark = "pick.3d";
      else
        *mark = hit;
    }
    return true;
  }
  if (tool_id_ == "edit.append.point" && event.kind == kInputEventMouse &&
      event.mouse_action == kMouseDown) {
    ++draft_points_;
    return true;
  }
  return true;
}

bool SceneImpl::Commit(std::string* mark) {
  if (mark)
    mark->clear();
  if (tool_id_ != "edit.append.point" || draft_points_ < 1)
    return false;
  std::string name;
  if (mode_ == SceneMode::kScene2D)
    name = "draft.basemap";
  else if (mode_ == SceneMode::kScene3D)
    name = "draft.groundwater";
  else
    return false;
  draft_points_ = 0;
  if (mark)
    *mark = name;
  return true;
}

Scene* CreateProductScene() {
  return new SceneImpl();
}

}  // namespace content
