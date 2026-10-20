// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "ui/views/markup/factory/placeholder_view.h"

#include "base/utf_string_conversions.h"
#include "third_party/skia/include/core/SkColor.h"
#include "ui/gfx/canvas.h"
#include "ui/gfx/font.h"

namespace views {

namespace {
const int kPlaceholderWidth = 160;
const int kPlaceholderHeight = 48;
}  // namespace

PlaceholderView::PlaceholderView(std::string caption)
    : caption_(std::move(caption)) {}

PlaceholderView::~PlaceholderView() {}

gfx::Size PlaceholderView::GetPreferredSize() {
  return gfx::Size(kPlaceholderWidth, kPlaceholderHeight);
}

std::string PlaceholderView::GetClassName() const {
  return "views/PlaceholderView";
}

void PlaceholderView::OnPaint(gfx::Canvas* canvas) {
  if (!canvas)
    return;
  const gfx::Rect b = GetLocalBounds();
  canvas->FillRect(b, SkColorSetRGB(0xF4, 0xF6, 0xF8));
  canvas->DrawRect(b, SkColorSetRGB(0x25, 0x63, 0xA8));
  if (caption_.empty() || b.width() <= 8 || b.height() <= 8)
    return;
  canvas->DrawStringInt(UTF8ToUTF16(caption_),
                         gfx::Font("Segoe UI", 12),
                         SkColorSetRGB(0x1A, 0x23, 0x30),
                         8,
                         8,
                         b.width() - 16,
                         b.height() - 16);
}

}  // namespace views
