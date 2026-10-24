// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "app/mgis/horizon/status_bar.h"

#include <algorithm>

#include "base/utf_string_conversions.h"
#include "third_party/skia/include/core/SkColor.h"
#include "ui/base/win/dpi.h"
#include "ui/gfx/canvas.h"
#include "ui/views/controls/label.h"

namespace mgis {
namespace {

int ScaleDip(int dip) {
  const float scale = ui::win::GetDeviceScaleFactor();
  return static_cast<int>(dip * (scale > 0.0f ? scale : 1.0f) + 0.5f);
}

views::Label* MakeField(const string16& text) {
  views::Label* label = new views::Label(text);
  label->SetHorizontalAlignment(gfx::ALIGN_LEFT);
  label->SetEnabledColor(SkColorSetRGB(0x1a, 0x23, 0x30));
  label->SetAutoColorReadabilityEnabled(false);
  return label;
}

}  // namespace

StatusBar::StatusBar()
    : message_(NULL), scale_(NULL), crs_(NULL), coord_(NULL) {
  // Use UTF8ToUTF16: em dash is not ASCII (ASCIIToUTF16 DCHECKs).
  message_ = MakeField(UTF8ToUTF16("Ready"));
  scale_ = MakeField(UTF8ToUTF16("1:—"));
  crs_ = MakeField(UTF8ToUTF16("EPSG:4326"));
  coord_ = MakeField(UTF8ToUTF16("XY —"));
  AddChildView(message_);
  AddChildView(scale_);
  AddChildView(crs_);
  AddChildView(coord_);
}

StatusBar::~StatusBar() {}

void StatusBar::SetMessage(const std::string& utf8) {
  if (message_)
    message_->SetText(UTF8ToUTF16(utf8));
}

void StatusBar::SetScaleText(const std::string& utf8) {
  if (scale_)
    scale_->SetText(UTF8ToUTF16(utf8));
}

void StatusBar::SetCrsText(const std::string& utf8) {
  if (crs_)
    crs_->SetText(UTF8ToUTF16(utf8));
}

void StatusBar::SetCoordText(const std::string& utf8) {
  if (coord_)
    coord_->SetText(UTF8ToUTF16(utf8));
}

void StatusBar::OnPaint(gfx::Canvas* canvas) {
  canvas->FillRect(GetLocalBounds(), SkColorSetRGB(0xee, 0xf2, 0xf6));
  View::OnPaint(canvas);
}

void StatusBar::Layout() {
  const int pad = ScaleDip(6);
  const int h = std::max(0, height() - 2);
  const int w = std::max(0, width() - 2 * pad);
  // Flexible message, then fixed-ish fields on the right (DIP widths).
  const int scale_w = ScaleDip(120);
  const int crs_w = ScaleDip(100);
  const int coord_w = ScaleDip(180);
  const int msg_w =
      std::max(ScaleDip(80), w - scale_w - crs_w - coord_w - 3 * pad);
  int x = pad;
  if (message_) {
    message_->SetBounds(x, 1, msg_w, h);
    x += msg_w + pad;
  }
  if (scale_) {
    scale_->SetBounds(x, 1, scale_w, h);
    x += scale_w + pad;
  }
  if (crs_) {
    crs_->SetBounds(x, 1, crs_w, h);
    x += crs_w + pad;
  }
  if (coord_)
    coord_->SetBounds(x, 1, coord_w, h);
}

}  // namespace mgis
