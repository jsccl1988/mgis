// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "app/mgis/horizon/viewport_placeholder.h"

#include "ui/gfx/canvas.h"
#include "ui/views/controls/label.h"

namespace mgis {

ViewportPlaceholder::ViewportPlaceholder(const string16& caption, SkColor fill)
    : caption_(caption),
      fill_(fill),
      label_(NULL) {
  label_ = new views::Label(caption_);
  label_->SetHorizontalAlignment(gfx::ALIGN_CENTER);
  label_->SetEnabledColor(SkColorSetRGB(0x5a, 0x6a, 0x7e));
  label_->SetAutoColorReadabilityEnabled(false);
  AddChildView(label_);
}

ViewportPlaceholder::~ViewportPlaceholder() {}

void ViewportPlaceholder::OnPaint(gfx::Canvas* canvas) {
  canvas->FillRect(GetLocalBounds(), fill_);
  View::OnPaint(canvas);
}

void ViewportPlaceholder::Layout() {
  if (label_)
    label_->SetBounds(0, 0, width(), height());
}

}  // namespace mgis
