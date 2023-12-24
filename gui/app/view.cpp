// Copyright (c) 2023 The MGIS Authors.
// All rights reserved.

#include "gui/app/view.h"

#include "gui/app/frame.h"

namespace gui {
View::View() {}
View::~View() {}

LRESULT View::OnSize(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam,
                     BOOL& bHandled) {
  if (frame_) {
    frame_->Resize();
  }

  return 0;
}
}  // namespace gui
