// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "app/mgis/horizon/bottom_panel.h"

#include <algorithm>

#include "base/utf_string_conversions.h"
#include "third_party/skia/include/core/SkColor.h"
#include "ui/base/win/dpi.h"
#include "ui/gfx/canvas.h"
#include "ui/views/controls/label.h"
#include "ui/views/controls/tabbed_pane/tabbed_pane.h"

namespace mgis {
namespace {

const int kLogLines = 10;

int ScaleDip(int dip) {
  const float scale = ui::win::GetDeviceScaleFactor();
  return static_cast<int>(dip * (scale > 0.0f ? scale : 1.0f) + 0.5f);
}

}  // namespace

class BottomPanel::LogPage : public views::View {
 public:
  LogPage() {
    for (int i = 0; i < kLogLines; ++i) {
      lines_[i] = new views::Label();
      lines_[i]->SetHorizontalAlignment(gfx::ALIGN_LEFT);
      lines_[i]->SetEnabledColor(SkColorSetRGB(0x1a, 0x23, 0x30));
      lines_[i]->SetAutoColorReadabilityEnabled(false);
      AddChildView(lines_[i]);
    }
  }

  void ShowLines(const std::vector<std::string>& utf8_lines) {
    const int total = static_cast<int>(utf8_lines.size());
    const int start = std::max(0, total - kLogLines);
    for (int i = 0; i < kLogLines; ++i) {
      const int source = start + i;
      if (source < total)
        lines_[i]->SetText(UTF8ToUTF16(utf8_lines[source]));
      else
        lines_[i]->SetText(string16());
    }
  }

  virtual void OnPaint(gfx::Canvas* canvas) OVERRIDE {
    canvas->FillRect(GetLocalBounds(), SkColorSetRGB(0xfa, 0xfb, 0xfc));
    View::OnPaint(canvas);
  }

  virtual void Layout() OVERRIDE {
    const int inset = ScaleDip(8);
    const int row = std::max(ScaleDip(16), height() / kLogLines);
    for (int i = 0; i < kLogLines; ++i)
      lines_[i]->SetBounds(inset, i * row, std::max(0, width() - 2 * inset),
                           row);
  }

 private:
  views::Label* lines_[kLogLines];

  DISALLOW_COPY_AND_ASSIGN(LogPage);
};

BottomPanel::BottomPanel()
    : tabs_(NULL),
      output_(NULL),
      console_(NULL),
      trace_(NULL),
      memory_(NULL),
      agent_(NULL) {
  output_ = new LogPage();
  console_ = new LogPage();
  trace_ = new LogPage();
  memory_ = new LogPage();
  agent_ = new LogPage();
  std::vector<std::string> memory_placeholder;
  memory_placeholder.push_back("(Memory tracking not enabled)");
  memory_->ShowLines(memory_placeholder);

  tabs_ = new views::TabbedPane();
  tabs_->AddTab(ASCIIToUTF16("Output"), output_);
  tabs_->AddTab(ASCIIToUTF16("Console"), console_);
  tabs_->AddTab(ASCIIToUTF16("Trace"), trace_);
  tabs_->AddTab(ASCIIToUTF16("Memory"), memory_);
  tabs_->AddTab(ASCIIToUTF16("Agent"), agent_);
  AddChildView(tabs_);
}

BottomPanel::~BottomPanel() {}

void BottomPanel::ShowChannel(int channel,
                              const std::vector<std::string>& utf8_lines) {
  // Console channel feeds both Output and Console (SmartGIS-style).
  if (channel == 0) {
    if (output_)
      output_->ShowLines(utf8_lines);
    if (console_)
      console_->ShowLines(utf8_lines);
    return;
  }
  LogPage* page = NULL;
  if (channel == 1)
    page = trace_;
  else if (channel == 2)
    page = agent_;
  if (page)
    page->ShowLines(utf8_lines);
}

void BottomPanel::Layout() {
  if (tabs_)
    tabs_->SetBounds(0, 0, width(), height());
}

}  // namespace mgis
