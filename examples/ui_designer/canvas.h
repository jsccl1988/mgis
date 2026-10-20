// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#ifndef EXAMPLES_UI_DESIGNER_CANVAS_H_
#define EXAMPLES_UI_DESIGNER_CANVAS_H_

#include "base/basictypes.h"
#include "ui/views/view.h"

namespace ui_designer {

// Hosts the live View tree produced by BuildMarkupTree.
class MarkupCanvas : public views::View {
 public:
  MarkupCanvas();
  void Install(views::View* root);

  virtual void Layout() OVERRIDE;

 private:
  DISALLOW_COPY_AND_ASSIGN(MarkupCanvas);
};

}  // namespace ui_designer

#endif  // EXAMPLES_UI_DESIGNER_CANVAS_H_
