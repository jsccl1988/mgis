// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "content/browser/map/fake_scene.h"

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

}  // namespace

FakeScene::FakeScene()
    : child_(NULL),
      mode_(SceneMode::kScene2D),
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

}  // namespace content
