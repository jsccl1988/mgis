// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "app/mgis/tool_plugin_panel.h"

#include <algorithm>

#include "base/utf_string_conversions.h"
#include "third_party/skia/include/core/SkColor.h"
#include "ui/gfx/canvas.h"
#include "ui/views/controls/label.h"

namespace mgis {

ToolPluginPanel::ToolPluginPanel() : title_(NULL), body_(NULL) {
  title_ = new views::Label(WideToUTF16(L"Tools / Plugins"));
  title_->SetHorizontalAlignment(gfx::ALIGN_LEFT);
  title_->SetAutoColorReadabilityEnabled(false);
  body_ = new views::Label(WideToUTF16(L"(empty shell)"));
  body_->SetHorizontalAlignment(gfx::ALIGN_LEFT);
  body_->SetEnabledColor(SkColorSetRGB(0x5a, 0x6a, 0x7e));
  body_->SetAutoColorReadabilityEnabled(false);
  AddChildView(title_);
  AddChildView(body_);
}

ToolPluginPanel::~ToolPluginPanel() {}

void ToolPluginPanel::OnPaint(gfx::Canvas* canvas) {
  canvas->FillRect(GetLocalBounds(), SkColorSetRGB(0xfa, 0xfb, 0xfc));
  View::OnPaint(canvas);
}

void ToolPluginPanel::Layout() {
  const int pad = 8;
  if (title_)
    title_->SetBounds(pad, pad, std::max(0, width() - 2 * pad), 20);
  if (body_)
    body_->SetBounds(pad, pad + 24, std::max(0, width() - 2 * pad), 40);
}

}  // namespace mgis
