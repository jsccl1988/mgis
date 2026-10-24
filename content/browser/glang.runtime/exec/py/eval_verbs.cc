// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "content/browser/glang.runtime/exec/py/eval_verbs.h"

#include <stdlib.h>

#include <vector>

#include "base/logging.h"
#include "content/browser/glang.runtime/script_runtime.h"
#include "content/public/browser/types.h"

namespace content {
namespace code {
namespace {

void SetError(std::string* error, const std::string& message) {
  if (error)
    *error = message;
}

bool IsSuiteView(const std::string& view_id) {
  return view_id == kViewIdScene || is_known_view_id(view_id);
}

bool ParseInt(const std::string& text, int* out) {
  if (text.empty() || !out)
    return false;
  char* end = nullptr;
  const long value = strtol(text.c_str(), &end, 10);
  if (!end || end == text.c_str() || *end != '\0')
    return false;
  *out = static_cast<int>(value);
  return true;
}

bool ParseDouble(const std::string& text, double* out) {
  if (text.empty() || !out)
    return false;
  char* end = nullptr;
  const double value = strtod(text.c_str(), &end);
  if (!end || end == text.c_str() || *end != '\0')
    return false;
  *out = value;
  return true;
}

bool EvalMark(CapabilityHost& host,
              const ScriptCall& call,
              std::string* error) {
  if (call.positional.size() != 1 || call.positional[0].empty()) {
    SetError(error, "mark requires a name");
    return false;
  }
  host.mark(call.positional[0]);
  return true;
}

bool EvalCommand(CapabilityHost& host,
                 const ScriptCall& call,
                 std::string* error,
                 bool activate) {
  const char* verb = activate ? "activate_tool" : "command";
  if (activate && !host.has_activate_slot()) {
    SetError(error, "activate slot not installed");
    return false;
  }
  if (!activate && !host.has_command_slot()) {
    SetError(error, "command slot not installed");
    return false;
  }
  if (call.positional.size() != 2 || call.positional[0].empty() ||
      !IsSuiteView(call.positional[1])) {
    SetError(error, std::string(verb) + " needs id and view");
    return false;
  }
  const bool ok = activate
                      ? host.RunActivate(call.positional[0], call.positional[1])
                      : host.RunCommand(call.positional[0], call.positional[1]);
  if (!ok) {
    SetError(error, std::string(verb) + " failed: " + call.positional[0]);
    return false;
  }
  return true;
}

bool EvalPointer(CapabilityHost& host,
                 const ScriptCall& call,
                 std::string* error) {
  if (!host.has_pointer_slot()) {
    SetError(error, "pointer slot not installed");
    return false;
  }
  if (call.positional.size() < 4 || call.positional.size() > 5) {
    SetError(error, "pointer needs view phase x y");
    return false;
  }
  if (!IsSuiteView(call.positional[0])) {
    SetError(error, "pointer view id");
    return false;
  }
  int x = 0;
  int y = 0;
  if (!ParseInt(call.positional[2], &x) || !ParseInt(call.positional[3], &y)) {
    SetError(error, "pointer coordinates");
    return false;
  }
  InputEvent event;
  event.view_id = call.positional[0];
  event.x = x;
  event.y = y;
  const std::string& phase = call.positional[1];
  if (phase == "move") {
    event.kind = kInputEventMouse;
    event.mouse_action = kMouseMove;
  } else if (phase == "down") {
    event.kind = kInputEventMouse;
    event.mouse_action = kMouseDown;
  } else if (phase == "up") {
    event.kind = kInputEventMouse;
    event.mouse_action = kMouseUp;
  } else if (phase == "wheel") {
    event.kind = kInputEventWheel;
    int delta = 120;
    if (call.positional.size() == 5 && !ParseInt(call.positional[4], &delta)) {
      SetError(error, "pointer wheel delta");
      return false;
    }
    event.wheel_delta = delta;
  } else {
    SetError(error, "pointer phase");
    return false;
  }
  if (!host.RunPointer(event)) {
    SetError(error, "pointer failed");
    return false;
  }
  return true;
}

bool EvalSceneMode(CapabilityHost& host,
                   const ScriptCall& call,
                   std::string* error) {
  if (call.positional.size() != 1 ||
      (call.positional[0] != "scene2d" && call.positional[0] != "scene3d" &&
       call.positional[0] != "columbus")) {
    SetError(error, "scene_mode accepts scene2d|scene3d|columbus");
    return false;
  }
  if (!host.has_scene_mode_slot()) {
    SetError(error, "scene_mode slot not installed");
    return false;
  }
  if (!host.RunSceneMode(call.positional[0])) {
    SetError(error, "scene_mode failed: " + call.positional[0]);
    return false;
  }
  return true;
}

bool EvalExpectCamera(CapabilityHost& host,
                      const ScriptCall& call,
                      std::string* error) {
  if (call.positional.empty() || call.positional[0] != kViewIdScene) {
    SetError(error, "expect_camera accepts only scene");
    return false;
  }
  bool has_target = false;
  double lon = 0;
  double lat = 0;
  double height = 0;
  if (call.positional.size() == 1) {
    has_target = false;
  } else if (call.positional.size() == 4) {
    if (!ParseDouble(call.positional[1], &lon) ||
        !ParseDouble(call.positional[2], &lat) ||
        !ParseDouble(call.positional[3], &height)) {
      SetError(error, "expect_camera coordinates");
      return false;
    }
    has_target = true;
  } else {
    SetError(error, "expect_camera scene needs lon lat height");
    return false;
  }
  if (!host.has_camera_slot()) {
    SetError(error, "expect_camera: scene is not attached");
    return false;
  }
  std::string camera_error;
  if (!host.RunExpectCamera(call.positional[0], has_target, lon, lat, height,
                            &camera_error)) {
    SetError(error, camera_error.empty() ? "expect_camera: scene is not attached"
                                         : camera_error);
    return false;
  }
  return true;
}

bool EvalAttach(CapabilityHost& host,
                const ScriptCall& call,
                std::string* error) {
  if (call.positional.size() != 1 || call.positional[0] != kViewIdScene) {
    SetError(error, "attach accepts only scene");
    return false;
  }
  if (!host.has_attach_slot()) {
    SetError(error, "parent hwnd missing");
    return false;
  }
  std::string attach_error;
  if (!host.RunAttach(call.positional[0], &attach_error)) {
    SetError(error, attach_error.empty() ? "attach failed" : attach_error);
    return false;
  }
  return true;
}

bool EvalExpectId(CapabilityHost& host,
                  const ScriptCall& call,
                  std::string* error,
                  bool layer) {
  const char* verb = layer ? "expect_layer" : "expect_entity";
  if (layer && !host.has_layer_slot()) {
    SetError(error, "expect_layer slot not installed");
    return false;
  }
  if (!layer && !host.has_entity_slot()) {
    SetError(error, "expect_entity slot not installed");
    return false;
  }
  if (call.positional.size() != 1 || call.positional[0].empty()) {
    SetError(error, std::string(verb) + " needs an id");
    return false;
  }
  std::string id_error;
  const bool ok = layer ? host.RunExpectLayer(call.positional[0], &id_error)
                        : host.RunExpectEntity(call.positional[0], &id_error);
  if (!ok) {
    SetError(error, id_error.empty()
                        ? std::string(verb) + " failed: " + call.positional[0]
                        : id_error);
    return false;
  }
  return true;
}

}  // namespace

bool eval_script_call(CapabilityHost& host,
                      const ScriptCall& call,
                      std::string* error,
                      std::string* result) {
  if (result)
    result->clear();
  if (call.name == "mark")
    return EvalMark(host, call, error);
  if (call.name == "command")
    return EvalCommand(host, call, error, false);
  if (call.name == "activate_tool")
    return EvalCommand(host, call, error, true);
  if (call.name == "pointer")
    return EvalPointer(host, call, error);
  if (call.name == "scene_mode")
    return EvalSceneMode(host, call, error);
  if (call.name == "expect_camera")
    return EvalExpectCamera(host, call, error);
  if (call.name == "attach")
    return EvalAttach(host, call, error);
  if (call.name == "expect_layer")
    return EvalExpectId(host, call, error, true);
  if (call.name == "expect_entity")
    return EvalExpectId(host, call, error, false);
  if (call.name == "capture") {
    SetError(error, call.name + " is not available in this slice");
    return false;
  }
  const std::string text = "unknown verb '" + call.name + "'";
  SetError(error, text);
  LOG(ERROR) << "script: " << text;
  return false;
}

}  // namespace code
}  // namespace content
