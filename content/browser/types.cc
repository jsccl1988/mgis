// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "content/public/types.h"

namespace content {

const char kViewIdMap[] = "map";
const char kViewIdView0[] = "view0";
const char kViewIdView1[] = "view1";
const char kViewIdScene[] = "scene";

bool is_known_view_id(const std::string& view_id) {
  return view_id == kViewIdMap || view_id == kViewIdView0 ||
         view_id == kViewIdView1;
}

InputEvent::InputEvent()
    : kind(kInputEventMouse),
      x(0),
      y(0),
      modifiers(kInputModifierNone),
      mouse_action(kMouseMove),
      button(0),
      wheel_delta(0),
      key_code(0),
      key_down(false) {}

}  // namespace content
