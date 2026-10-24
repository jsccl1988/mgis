// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "app/mgis/horizon/status_bar.h"

#include <algorithm>

#include "base/utf_string_conversions.h"
#include "ui/base/win/dpi.h"
#include "ui/gfx/canvas.h"
#include "ui/views/controls/label.h"
#include "ui/views/theme/theme.h"

namespace mgis {
namespace {

int ScaleDip(int dip) {
  const float scale = ui::win::GetDeviceScaleFactor();
  return static_cast<int>(dip * (scale > 0.0f ? scale : 1.0f) + 0.5f);
}

views::Label* MakeField(const string16& text) {
  views::Label* label = new views::Label(text);
  label->SetHorizontalAlignment(gfx::ALIGN_LEFT);
  label->SetEnabledColor(views::Theme::Current().text);
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

void StatusBar::ApplyThemeColors() {
  const SkColor ink = views::Theme::Current().text;
  if (message_)
    message_->SetEnabledColor(ink);
  if (scale_)
    scale_->SetEnabledColor(ink);
  if (crs_)
    crs_->SetEnabledColor(ink);
  if (coord_)
    coord_->SetEnabledColor(ink);
}

void StatusBar::OnThemeChanged() {
  ApplyThemeColors();
  SchedulePaint();
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
  canvas->FillRect(GetLocalBounds(), views::Theme::Current().caption_bg);
  View::OnPaint(canvas);
}

void StatusBar::Layout() {
  const int pad = ScaleDip(6);
  const int h = std::max(0, height() - 2);
  const int inner = std::max(0, width() - 2 * pad);
  // Prefer a readable CRS; shrink coord then scale when the bar is narrow.
  int scale_w = ScaleDip(100);
  int crs_w = ScaleDip(120);
  int coord_w = ScaleDip(160);
  int reserved = scale_w + crs_w + coord_w + 3 * pad;
  if (reserved > inner) {
    int overflow = reserved - inner;
    const int cut_coord =
        std::min(overflow, std::max(0, coord_w - ScaleDip(72)));
    coord_w -= cut_coord;
    overflow -= cut_coord;
    if (overflow > 0) {
      const int cut_scale =
          std::min(overflow, std::max(0, scale_w - ScaleDip(56)));
      scale_w -= cut_scale;
      overflow -= cut_scale;
    }
    if (overflow > 0) {
      const int cut_crs =
          std::min(overflow, std::max(0, crs_w - ScaleDip(88)));
      crs_w -= cut_crs;
    }
    reserved = scale_w + crs_w + coord_w + 3 * pad;
  }
  const int msg_w = std::max(ScaleDip(64), inner - reserved);
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
