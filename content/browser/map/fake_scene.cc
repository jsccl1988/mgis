// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "content/browser/map/fake_scene.h"

#include <stdlib.h>

#include <cmath>
#include <vector>

#include <windows.h>

#include "content/public/trace_log.h"
#include "content/public/types.h"

namespace content {

namespace {

const char* SceneModeLabel(SceneMode mode) {
  if (mode == SceneMode::kScene3D)
    return "scene3d";
  if (mode == SceneMode::kColumbusView)
    return "columbus";
  return "scene2d";
}

bool ToolAllowed(SceneMode mode, const std::string& tool_id) {
  if (tool_id == "view.pan") {
    return mode == SceneMode::kScene2D || mode == SceneMode::kColumbusView;
  }
  if (tool_id == "selection.point" || tool_id == "edit.append.point") {
    return mode == SceneMode::kScene2D || mode == SceneMode::kScene3D;
  }
  return false;
}

void SplitTokens(const std::string& text, std::vector<std::string>* out) {
  size_t index = 0;
  while (index < text.size()) {
    while (index < text.size() && (text[index] == ' ' || text[index] == '\t'))
      ++index;
    if (index >= text.size())
      break;
    size_t end = index;
    while (end < text.size() && text[end] != ' ' && text[end] != '\t')
      ++end;
    out->push_back(text.substr(index, end - index));
    index = end;
  }
}

bool ParseDouble(const std::string& text, double* out) {
  if (text.empty() || !out)
    return false;
  char* end = NULL;
  *out = strtod(text.c_str(), &end);
  return end && end != text.c_str() && *end == '\0';
}

}  // namespace

FakeScene::FakeScene()
    : child_(NULL),
      mode_(SceneMode::kScene2D),
      tool_id_(),
      draft_points_(0),
      lon_(0),
      lat_(0),
      height_(0) {}

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

std::string FakeScene::Pick() const {
  if (mode_ == SceneMode::kScene2D)
    return "primitive:basemap:m0";
  if (mode_ == SceneMode::kScene3D)
    return "primitive:groundwater:g0";
  return std::string();
}

bool FakeScene::OnPointer(const InputEvent& event, std::string* mark) {
  if (mark)
    mark->clear();
  if (event.view_id != kViewIdScene || !ToolAllowed(mode_, tool_id_))
    return false;
  if (tool_id_ == "selection.point" && event.mouse_action == kMouseDown &&
      event.kind == kInputEventMouse) {
    const std::string hit = Pick();
    if (hit.empty())
      return false;
    if (mark)
      *mark = hit;
    return true;
  }
  if (tool_id_ == "edit.append.point" && event.mouse_action == kMouseDown &&
      event.kind == kInputEventMouse) {
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

bool FakeScene::ExpectCamera(const std::string& spec) const {
  std::vector<std::string> tokens;
  SplitTokens(spec, &tokens);
  if (tokens.size() != 4 || tokens[0] != kViewIdScene)
    return false;
  if (mode_ != SceneMode::kScene2D && mode_ != SceneMode::kColumbusView)
    return false;
  double lon = 0;
  double lat = 0;
  double height = 0;
  if (!ParseDouble(tokens[1], &lon) || !ParseDouble(tokens[2], &lat) ||
      !ParseDouble(tokens[3], &height))
    return false;
  return std::fabs(lon_ - lon) <= 1e-6 && std::fabs(lat_ - lat) <= 1e-6 &&
         std::fabs(height_ - height) <= 1.0;
}

}  // namespace content
