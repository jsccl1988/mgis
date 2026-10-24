// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "app/mgis/horizon/map_toolbar.h"

#include <algorithm>
#include <string>

#include "base/utf_string_conversions.h"
#include "ui/base/win/dpi.h"
#include "ui/gfx/canvas.h"
#include "ui/views/controls/button/text_button.h"
#include "ui/views/theme/theme.h"

namespace mgis {
namespace {

struct ToolDef {
  const char* label;
  const char* tool_id;
};

// Keep the strip short so labels stay readable at 1100 DIP and HiDPI.
const ToolDef kTools[] = {
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
  canvas->FillRect(GetLocalBounds(), views::Theme::Current().panel_header);
  View::OnPaint(canvas);
}

void MapToolbar::OnThemeChanged() {
  SchedulePaint();
}

void MapToolbar::Layout() {
  const float scale = ui::win::GetDeviceScaleFactor();
  const float s = scale > 0.0f ? scale : 1.0f;
  const int pad = static_cast<int>(4 * s + 0.5f);
  int gap = static_cast<int>(4 * s + 0.5f);
  const int h = std::max(0, height() - 2 * pad);
  const int avail = std::max(0, width() - 2 * pad);

  std::vector<int> widths(buttons_.size(), 0);
  int total = 0;
  int visible = 0;
  for (size_t i = 0; i < buttons_.size(); ++i) {
    if (!buttons_[i])
      continue;
    // Prefer natural text width; avoid a large min width that forces overlap.
    widths[i] = std::max(static_cast<int>(48 * s + 0.5f),
                         buttons_[i]->GetPreferredSize().width() +
                             static_cast<int>(8 * s + 0.5f));
    total += widths[i];
    ++visible;
  }
  if (visible > 1)
    total += gap * (visible - 1);

  if (total > avail && visible > 0) {
    // Shrink gaps first, then scale button widths so glyphs are not clipped.
    const int gaps = visible - 1;
    while (total > avail && gap > 1) {
      --gap;
      total = 0;
      for (size_t i = 0; i < widths.size(); ++i)
        total += widths[i];
      total += gap * gaps;
    }
    if (total > avail) {
      const double factor =
          static_cast<double>(avail - gap * gaps) /
          static_cast<double>(std::max(1, total - gap * gaps));
      total = 0;
      for (size_t i = 0; i < widths.size(); ++i) {
        if (!buttons_[i])
          continue;
        widths[i] = std::max(static_cast<int>(36 * s + 0.5f),
                             static_cast<int>(widths[i] * factor + 0.5));
        total += widths[i];
      }
      total += gap * gaps;
    }
  }

  int x = pad;
  for (size_t i = 0; i < buttons_.size(); ++i) {
    if (!buttons_[i])
      continue;
    buttons_[i]->SetBounds(x, pad, widths[i], h);
    x += widths[i] + gap;
  }
}

}  // namespace mgis
