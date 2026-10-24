// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "app/mgis/horizon/viewport_placeholder.h"

#include "ui/gfx/canvas.h"
#include "ui/views/controls/label.h"
#include "ui/views/theme/theme.h"

namespace mgis {

ViewportPlaceholder::ViewportPlaceholder(const string16& caption, SkColor fill)
    : caption_(caption),
      fill_(fill),
      label_(NULL) {
  label_ = new views::Label(caption_);
  label_->SetHorizontalAlignment(gfx::ALIGN_CENTER);
  label_->SetEnabledColor(views::Theme::Current().text_muted);
  label_->SetAutoColorReadabilityEnabled(false);
  AddChildView(label_);
}

ViewportPlaceholder::~ViewportPlaceholder() {}

void ViewportPlaceholder::OnPaint(gfx::Canvas* canvas) {
  canvas->FillRect(GetLocalBounds(), fill_);
  View::OnPaint(canvas);
}

void ViewportPlaceholder::OnThemeChanged() {
  if (label_)
    label_->SetEnabledColor(views::Theme::Current().text_muted);
  SchedulePaint();
}

void ViewportPlaceholder::Layout() {
  if (label_)
    label_->SetBounds(0, 0, width(), height());
}

}  // namespace mgis
