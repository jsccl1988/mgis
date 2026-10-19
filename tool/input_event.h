// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#ifndef TOOL_INPUT_EVENT_H_
#define TOOL_INPUT_EVENT_H_

#include <string>

namespace tool {

// Pointer, wheel, and key input already expressed in one view's pixel space.
// Field names match the later content::InputEvent adapter: view_id, x_px,
// y_px, wheel_dx, wheel_dy, and kind. action separates move/press/release
// because kind alone is only mouse, wheel, or key.
enum InputKind {
  kMouse,
  kWheel,
  kKey,
};

enum MouseAction {
  kMouseNone,
  kMouseMove,
  kMousePress,
  kMouseRelease,
};

struct InputEvent {
  InputKind kind;
  MouseAction action;
  std::string view_id;
  double x_px;
  double y_px;
  double wheel_dx;
  double wheel_dy;
  int modifiers;

  InputEvent()
      : kind(kMouse),
        action(kMouseMove),
        x_px(0.0),
        y_px(0.0),
        wheel_dx(0.0),
        wheel_dy(0.0),
        modifiers(0) {}
};

}  // namespace tool

#endif  // TOOL_INPUT_EVENT_H_
