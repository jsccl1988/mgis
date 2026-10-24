// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "content/browser/viewport/map/fake_scene.h"

#include <cmath>

#include <windows.h>

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
      tool_id == "view.zoom_out") {
    return mode == SceneMode::kScene2D || mode == SceneMode::kColumbusView;
  }
  if (tool_id == "view3d.trackball")
    return mode == SceneMode::kScene3D;
  if (tool_id == "selection.point" || tool_id == "edit.append.point")
    return mode == SceneMode::kScene2D || mode == SceneMode::kScene3D;
  return false;
}

}  // namespace

FakeScene::FakeScene()
    : child_(NULL),
      mode_(SceneMode::kScene2D),
      tool_id_(),
      draft_points_(0),
      lon_(0),
      lat_(0),
      height_(0),
      orbit_delta_(0),
      trackball_armed_(false) {
  primitive_ids_.push_back(kBasemapId);
  primitive_ids_.push_back(kGroundwaterId);
}

FakeScene::~FakeScene() {
  if (child_)
    DestroyWindow(reinterpret_cast<HWND>(child_));
}

bool FakeScene::Attach(void* parent_hwnd,
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

  HWND child = CreateWindowExW(0, L"STATIC", L"",
                               WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, 0, 0,
                               width, height,
                               reinterpret_cast<HWND>(parent_hwnd), NULL,
                               GetModuleHandleW(NULL), NULL);
  if (!child)
    return false;

  if (child_)
    DestroyWindow(reinterpret_cast<HWND>(child_));
  child_ = child;
  mode_ = SceneMode::kScene2D;
  lon_ = 0;
  lat_ = 0;
  height_ = 0;
  orbit_delta_ = 0;
  trackball_armed_ = false;
  return true;
}

void FakeScene::SetSceneMode(SceneMode mode, TraceLog* trace) {
  mode_ = mode;
  if (trace) {
    trace->Append(kTraceConsole,
                  std::string("scene.mode ") + SceneModeLabel(mode));
  }
}

SceneMode FakeScene::scene_mode() const {
  return mode_;
}

bool FakeScene::HasPrimitive(const std::string& id) const {
  for (size_t i = 0; i < primitive_ids_.size(); ++i) {
    if (primitive_ids_[i] == id)
      return true;
  }
  return false;
}

bool FakeScene::HasLayer(const std::string& id) const {
  return HasPrimitive(id);
}

bool FakeScene::HasEntity(const std::string& id) const {
  if (mode_ == SceneMode::kScene2D)
    return id == "m0" && HasPrimitive(kBasemapId);
  if (mode_ == SceneMode::kScene3D)
    return id == "g0" && HasPrimitive(kGroundwaterId);
  return false;
}

std::string FakeScene::Pick(int /*x*/, int /*y*/) const {
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

void FakeScene::ArmTrackball() {
  trackball_armed_ = true;
}

void FakeScene::NotePointer(const InputEvent& event) {
  if (!trackball_armed_ || event.view_id != kViewIdScene)
    return;
  if (event.kind == kInputEventMouse && event.mouse_action == kMouseMove)
    orbit_delta_ = 1;
}

bool FakeScene::ExpectCamera(bool has_target,
                             double lon,
                             double lat,
                             double height,
                             std::string* error) const {
  if (!child_) {
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
  const double kDegrees = 1e-6;
  const double kHeightMeters = 1;
  if (std::fabs(lon_ - lon) > kDegrees || std::fabs(lat_ - lat) > kDegrees ||
      std::fabs(height_ - height) > kHeightMeters) {
    if (error)
      *error = "expect_camera: scene camera mismatch";
    return false;
  }
  return true;
}

bool FakeScene::SetModeLabel(const std::string& label, TraceLog* trace) {
  SceneMode mode = SceneMode::kScene2D;
  if (label == "scene2d")
    mode = SceneMode::kScene2D;
  else if (label == "scene3d")
    mode = SceneMode::kScene3D;
  else if (label == "columbus")
    mode = SceneMode::kColumbusView;
  else
    return false;
  draft_points_ = 0;
  SetSceneMode(mode, trace);
  return true;
}

bool FakeScene::ActivateTool(const std::string& tool_id,
                             const std::string& view_id) {
  if (view_id != kViewIdScene || !ToolAllowed(mode_, tool_id))
    return false;
  tool_id_ = tool_id;
  if (tool_id == "edit.append.point")
    draft_points_ = 0;
  return true;
}

bool FakeScene::OnPointer(const InputEvent& event, std::string* mark) {
  if (mark)
    mark->clear();
  if (event.view_id != kViewIdScene || !ToolAllowed(mode_, tool_id_))
    return false;
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

bool FakeScene::Commit(std::string* mark) {
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

}  // namespace content
