// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#ifndef CONTENT_PUBLIC_TYPES_H_
#define CONTENT_PUBLIC_TYPES_H_

#include <string>

namespace content {

// Viewport ids for the map pane and the two 3D panes. Commands, input, and
// suite verbs all name a view with one of these strings.
extern const char kViewIdMap[];
extern const char kViewIdView0[];
extern const char kViewIdView1[];

// True when |view_id| is map, view0, or view1.
bool is_known_view_id(const std::string& view_id);

// Stable feature identity carried on selection events. P0 stores the raw id
// text; it does not parse GeoJSON or UE entity handles.
typedef std::string FeatureId;

// Bit mask stored on InputEvent::modifiers.
enum InputModifier {
  kInputModifierNone = 0,
  kInputModifierShift = 1 << 0,
  kInputModifierControl = 1 << 1,
  kInputModifierAlt = 1 << 2,
};

// Which device sample an InputEvent carries. The shell fills one kind per
// event after it translates a native message.
enum InputEventKind {
  kInputEventMouse,
  kInputEventWheel,
  kInputEventKey,
};

// Mouse phase. Meaningful when kind is kInputEventMouse.
enum MouseAction {
  kMouseMove,
  kMouseDown,
  kMouseUp,
};

// One pointer, wheel, or key sample in a view's pixel space.
// Tools consume this value. They do not see HWND messages.
struct InputEvent {
  InputEvent();

  InputEventKind kind;
  std::string view_id;
  int x;
  int y;
  int modifiers;

  // Mouse. Ignored unless kind is kInputEventMouse.
  MouseAction mouse_action;
  int button;

  // Wheel. Ignored unless kind is kInputEventWheel.
  // Positive means the wheel rotated away from the user.
  int wheel_delta;

  // Key. Ignored unless kind is kInputEventKey.
  int key_code;
  bool key_down;
};

}  // namespace content

#endif  // CONTENT_PUBLIC_TYPES_H_
