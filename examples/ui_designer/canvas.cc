// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "examples/ui_designer/canvas.h"

namespace ui_designer {

MarkupCanvas::MarkupCanvas() {}

void MarkupCanvas::Install(views::View* root) {
  RemoveAllChildViews(true);
  if (root)
    AddChildView(root);
  Layout();
}

void MarkupCanvas::Layout() {
  if (child_count() == 1)
    child_at(0)->SetBounds(0, 0, width(), height());
}

}  // namespace ui_designer
