// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "app/mgis/horizon/map_toolbar.h"

#include <algorithm>
#include <string>

#include "base/utf_string_conversions.h"
#include "third_party/skia/include/core/SkColor.h"
#include "ui/base/win/dpi.h"
#include "ui/gfx/canvas.h"
#include "ui/views/controls/button/text_button.h"

namespace mgis {
namespace {

struct ToolDef {
  const char* label;
  const char* tool_id;
};

const ToolDef kTools[] = {
    {"Sel pt", "selection.point"},
    {"Sel rect", "selection.point"},
    {"Sel poly", "selection.point"},
    {"Clear", "view.pan"},
    {"Select", "selection.point"},
    {"Point", "edit.append.point"},
    {"Line", "edit.append.point"},
    {"Polygon", "edit.append.point"},
    {"Vertex", "edit.append.point"},
    {"Undo", "view.pan"},
    {"Redo", "view.pan"},
    {"Cancel", "view.pan"},
};

}  // namespace

MapToolbar::MapToolbar() : delegate_(NULL) {
  for (size_t i = 0; i < arraysize(kTools); ++i) {
    ToolEntry entry = {kTools[i].label, kTools[i].tool_id};
    AddToolButton(entry, static_cast<int>(i));
  }
}

MapToolbar::~MapToolbar() {}

void MapToolbar::AddToolButton(const ToolEntry& entry, int tag) {
  views::TextButton* button =
      new views::TextButton(this, ASCIIToUTF16(entry.label));
  button->set_tag(tag);
  AddChildView(button);
  buttons_.push_back(button);
  tool_ids_.push_back(entry.tool_id);
}

void MapToolbar::ButtonPressed(views::Button* sender,
                               const ui::Event& /*event*/) {
  if (!delegate_ || !sender)
    return;
  const int tag = sender->tag();
  if (tag < 0 || static_cast<size_t>(tag) >= tool_ids_.size())
    return;
  delegate_->OnMapToolSelected(tool_ids_[static_cast<size_t>(tag)]);
}

void MapToolbar::OnPaint(gfx::Canvas* canvas) {
  canvas->FillRect(GetLocalBounds(), SkColorSetRGB(0xf0, 0xf3, 0xf7));
  View::OnPaint(canvas);
}

void MapToolbar::Layout() {
  const float scale = ui::win::GetDeviceScaleFactor();
  const float s = scale > 0.0f ? scale : 1.0f;
  const int pad = static_cast<int>(4 * s + 0.5f);
  const int gap = static_cast<int>(4 * s + 0.5f);
  const int min_w = static_cast<int>(56 * s + 0.5f);
  const int pad_w = static_cast<int>(12 * s + 0.5f);
  const int h = std::max(0, height() - 2 * pad);
  int x = pad;
  for (size_t i = 0; i < buttons_.size(); ++i) {
    if (!buttons_[i])
      continue;
    const int w =
        std::max(min_w, buttons_[i]->GetPreferredSize().width() + pad_w);
    buttons_[i]->SetBounds(x, pad, w, h);
    x += w + gap;
  }
}

}  // namespace mgis
