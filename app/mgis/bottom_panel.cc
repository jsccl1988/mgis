// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "app/mgis/bottom_panel.h"

#include <algorithm>

#include "base/utf_string_conversions.h"
#include "third_party/skia/include/core/SkColor.h"
#include "ui/views/controls/label.h"
#include "ui/views/controls/tabbed_pane/tabbed_pane.h"

namespace mgis {
namespace {

const int kLogLines = 8;

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

  virtual void Layout() OVERRIDE {
    const int row = std::max(1, height() / kLogLines);
    for (int i = 0; i < kLogLines; ++i)
      lines_[i]->SetBounds(8, i * row, std::max(0, width() - 16), row);
  }

 private:
  views::Label* lines_[kLogLines];

  DISALLOW_COPY_AND_ASSIGN(LogPage);
};

BottomPanel::BottomPanel()
    : tabs_(NULL), console_(NULL), trace_(NULL), agent_(NULL) {
  console_ = new LogPage();
  trace_ = new LogPage();
  agent_ = new LogPage();
  tabs_ = new views::TabbedPane();
  tabs_->AddTab(ASCIIToUTF16("console"), console_);
  tabs_->AddTab(ASCIIToUTF16("trace"), trace_);
  tabs_->AddTab(ASCIIToUTF16("agent"), agent_);
  AddChildView(tabs_);
}

BottomPanel::~BottomPanel() {}

void BottomPanel::ShowChannel(int channel,
                              const std::vector<std::string>& utf8_lines) {
  LogPage* page = console_;
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
