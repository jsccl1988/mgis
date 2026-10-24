// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "app/mgis/horizon/tool_plugin_panel.h"

#include <algorithm>

#include "base/utf_string_conversions.h"
#include "content/public/browser/plugin_host.h"
#include "third_party/skia/include/core/SkColor.h"
#include "ui/base/win/dpi.h"
#include "ui/gfx/canvas.h"
#include "ui/views/controls/button/text_button.h"
#include "ui/views/controls/label.h"
#include "ui/views/controls/tabbed_pane/tabbed_pane.h"
#include "ui/views/controls/textfield/textfield.h"

namespace mgis {
namespace {

const char* kGroupLabels[] = {"Inspect", "Map", "Process", "Extend"};

const int kTagGroupBase = 1000;
const int kTagPluginBase = 2000;
const int kTagEnable = 3001;
const int kTagDisable = 3002;
const int kTagRun = 3003;

int ScaleDip(int dip) {
  const float scale = ui::win::GetDeviceScaleFactor();
  return static_cast<int>(dip * (scale > 0.0f ? scale : 1.0f) + 0.5f);
}

// Fills a tab page so the Atmosphere label is not left at 0x0.
class FillLabelPage : public views::View {
 public:
  explicit FillLabelPage(views::Label* label) : label_(label) {
    AddChildView(label_);
  }

  virtual void Layout() OVERRIDE {
    if (label_)
      label_->SetBounds(0, 0, width(), height());
  }

 private:
  views::Label* label_;
  DISALLOW_COPY_AND_ASSIGN(FillLabelPage);
};

bool PassesFilter(const std::string& id, const string16& filter) {
  if (filter.empty())
    return true;
  const string16 hay = UTF8ToUTF16(id);
  return hay.find(filter) != string16::npos;
}

}  // namespace

ToolPluginPanel::ToolPluginPanel()
    : host_(NULL),
      group_(kExtend),
      body_(NULL),
      placeholder_(NULL),
      extend_tabs_(NULL),
      plugins_page_(NULL),
      filter_(NULL),
      hint_(NULL),
      enable_btn_(NULL),
      disable_btn_(NULL),
      run_btn_(NULL),
      selected_index_(-1) {
  for (int i = 0; i < kGroupCount; ++i) {
    group_buttons_[i] =
        new views::TextButton(this, ASCIIToUTF16(kGroupLabels[i]));
    group_buttons_[i]->set_tag(kTagGroupBase + i);
    AddChildView(group_buttons_[i]);
  }

  body_ = new views::View();
  AddChildView(body_);

  placeholder_ = new views::Label(
      ASCIIToUTF16("Select a group. Extend hosts plugins."));
  placeholder_->SetHorizontalAlignment(gfx::ALIGN_LEFT);
  placeholder_->SetEnabledColor(SkColorSetRGB(0x5a, 0x6a, 0x7e));
  placeholder_->SetAutoColorReadabilityEnabled(false);
  body_->AddChildView(placeholder_);

  plugins_page_ = new views::View();
  filter_ = new views::Textfield();
  filter_->SetController(this);
  filter_->SetText(ASCIIToUTF16(""));
  plugins_page_->AddChildView(filter_);

  enable_btn_ = new views::TextButton(this, ASCIIToUTF16("Enable"));
  enable_btn_->set_tag(kTagEnable);
  disable_btn_ = new views::TextButton(this, ASCIIToUTF16("Disable"));
  disable_btn_->set_tag(kTagDisable);
  run_btn_ = new views::TextButton(this, ASCIIToUTF16("Run"));
  run_btn_->set_tag(kTagRun);
  plugins_page_->AddChildView(enable_btn_);
  plugins_page_->AddChildView(disable_btn_);
  plugins_page_->AddChildView(run_btn_);

  hint_ = new views::Label(
      ASCIIToUTF16("Select a plugin. Its commands open here."));
  hint_->SetHorizontalAlignment(gfx::ALIGN_LEFT);
  hint_->SetEnabledColor(SkColorSetRGB(0x5a, 0x6a, 0x7e));
  hint_->SetAutoColorReadabilityEnabled(false);
  plugins_page_->AddChildView(hint_);

  views::Label* atm_label =
      new views::Label(ASCIIToUTF16("(Atmosphere panel placeholder)"));
  atm_label->SetHorizontalAlignment(gfx::ALIGN_LEFT);
  atm_label->SetEnabledColor(SkColorSetRGB(0x5a, 0x6a, 0x7e));
  atm_label->SetAutoColorReadabilityEnabled(false);
  views::View* atmosphere = new FillLabelPage(atm_label);

  extend_tabs_ = new views::TabbedPane();
  extend_tabs_->AddTab(ASCIIToUTF16("Atmosphere"), atmosphere);
  extend_tabs_->AddTab(ASCIIToUTF16("Plugins"), plugins_page_);
  extend_tabs_->SelectTabAt(1);
  body_->AddChildView(extend_tabs_);

  SelectGroup(kExtend);
}

ToolPluginPanel::~ToolPluginPanel() {
  ClearPluginButtons();
}

void ToolPluginPanel::SetPluginHost(content::PluginHost* host) {
  host_ = host;
  RefreshCommands();
}

void ToolPluginPanel::ClearPluginButtons() {
  for (size_t i = 0; i < plugin_buttons_.size(); ++i) {
    if (plugin_buttons_[i] && plugins_page_)
      plugins_page_->RemoveChildView(plugin_buttons_[i]);
    delete plugin_buttons_[i];
  }
  plugin_buttons_.clear();
}

void ToolPluginPanel::RefreshCommands() {
  command_ids_.clear();
  command_enabled_.clear();
  selected_index_ = -1;
  if (host_)
    command_ids_ = host_->command_ids();
  command_enabled_.assign(command_ids_.size(), true);
  RebuildPluginList();
}

void ToolPluginPanel::RebuildPluginList() {
  ClearPluginButtons();
  for (size_t i = 0; i < command_ids_.size(); ++i) {
    if (!PassesFilter(command_ids_[i], filter_text_))
      continue;
    const char* state = command_enabled_[i] ? "on " : "off ";
    views::TextButton* button = new views::TextButton(
        this, ASCIIToUTF16(std::string(state) + command_ids_[i]));
    button->set_tag(kTagPluginBase + static_cast<int>(i));
    plugins_page_->AddChildView(button);
    plugin_buttons_.push_back(button);
  }
  if (hint_) {
    if (command_ids_.empty())
      hint_->SetText(ASCIIToUTF16("(no plugins)"));
    else if (selected_index_ < 0)
      hint_->SetText(
          ASCIIToUTF16("Select a plugin. Its commands open here."));
  }
  Layout();
  SchedulePaint();
}

void ToolPluginPanel::SelectGroup(Group group) {
  group_ = group;
  const bool extend = group_ == kExtend;
  if (placeholder_)
    placeholder_->SetVisible(!extend);
  if (extend_tabs_)
    extend_tabs_->SetVisible(extend);
  Layout();
  SchedulePaint();
}

void ToolPluginPanel::SetSelectedEnabled(bool enabled) {
  if (selected_index_ < 0 ||
      static_cast<size_t>(selected_index_) >= command_enabled_.size())
    return;
  command_enabled_[static_cast<size_t>(selected_index_)] = enabled;
  RebuildPluginList();
}

void ToolPluginPanel::RunSelected() {
  if (!host_ || selected_index_ < 0 ||
      static_cast<size_t>(selected_index_) >= command_ids_.size())
    return;
  if (!command_enabled_[static_cast<size_t>(selected_index_)]) {
    if (hint_)
      hint_->SetText(ASCIIToUTF16("Plugin disabled. Enable it first."));
    return;
  }
  std::string error;
  host_->run_processing(command_ids_[static_cast<size_t>(selected_index_)],
                        &error);
  if (hint_) {
    if (error.empty())
      hint_->SetText(UTF8ToUTF16("Ran: " +
                                 command_ids_[static_cast<size_t>(
                                     selected_index_)]));
    else
      hint_->SetText(UTF8ToUTF16("Error: " + error));
  }
}

void ToolPluginPanel::ButtonPressed(views::Button* sender,
                                    const ui::Event& /*event*/) {
  if (!sender)
    return;
  const int tag = sender->tag();
  if (tag >= kTagGroupBase && tag < kTagGroupBase + kGroupCount) {
    SelectGroup(static_cast<Group>(tag - kTagGroupBase));
    return;
  }
  if (tag >= kTagPluginBase) {
    selected_index_ = tag - kTagPluginBase;
    if (hint_ && selected_index_ >= 0 &&
        static_cast<size_t>(selected_index_) < command_ids_.size()) {
      hint_->SetText(UTF8ToUTF16("Selected: " +
                                 command_ids_[static_cast<size_t>(
                                     selected_index_)]));
    }
    return;
  }
  if (tag == kTagEnable) {
    SetSelectedEnabled(true);
    return;
  }
  if (tag == kTagDisable) {
    SetSelectedEnabled(false);
    return;
  }
  if (tag == kTagRun)
    RunSelected();
}

void ToolPluginPanel::ContentsChanged(views::Textfield* sender,
                                      const string16& new_contents) {
  if (sender != filter_)
    return;
  filter_text_ = new_contents;
  RebuildPluginList();
}

bool ToolPluginPanel::HandleKeyEvent(views::Textfield* /*sender*/,
                                     const ui::KeyEvent& /*key_event*/) {
  return false;
}

void ToolPluginPanel::OnPaint(gfx::Canvas* canvas) {
  canvas->FillRect(GetLocalBounds(), SkColorSetRGB(0xfa, 0xfb, 0xfc));
  View::OnPaint(canvas);
}

void ToolPluginPanel::Layout() {
  const int pad = ScaleDip(6);
  const int group_h = ScaleDip(26);
  const int gap = ScaleDip(2);
  int x = pad;
  const int group_w =
      std::max(ScaleDip(48),
               (std::max(0, width() - 2 * pad) / kGroupCount) - gap);
  for (int i = 0; i < kGroupCount; ++i) {
    if (group_buttons_[i])
      group_buttons_[i]->SetBounds(x, pad, group_w, group_h);
    x += group_w + gap;
  }

  const int body_y = pad + group_h + pad;
  const int body_h = std::max(0, height() - body_y - pad);
  if (body_)
    body_->SetBounds(pad, body_y, std::max(0, width() - 2 * pad), body_h);

  if (!body_)
    return;
  const int bw = body_->width();
  const int bh = body_->height();
  if (placeholder_)
    placeholder_->SetBounds(0, 0, bw, ScaleDip(40));
  if (extend_tabs_)
    extend_tabs_->SetBounds(0, 0, bw, bh);

  if (!plugins_page_)
    return;
  const int pw = plugins_page_->width() > 0 ? plugins_page_->width() : bw;
  int y = pad;
  const int row_h = ScaleDip(22);
  if (filter_) {
    filter_->SetBounds(pad, y, std::max(0, pw - 2 * pad), row_h);
    y += ScaleDip(26);
  }
  for (size_t i = 0; i < plugin_buttons_.size(); ++i) {
    if (!plugin_buttons_[i])
      continue;
    plugin_buttons_[i]->SetBounds(pad, y, std::max(0, pw - 2 * pad), row_h);
    y += ScaleDip(24);
  }
  const int btn_w = ScaleDip(64);
  const int run_w = ScaleDip(80);
  const int action_h = ScaleDip(24);
  if (enable_btn_)
    enable_btn_->SetBounds(pad, y, btn_w, action_h);
  if (disable_btn_)
    disable_btn_->SetBounds(pad + btn_w + ScaleDip(4), y, btn_w, action_h);
  if (run_btn_)
    run_btn_->SetBounds(pad + 2 * (btn_w + ScaleDip(4)), y, run_w, action_h);
  y += ScaleDip(30);
  if (hint_)
    hint_->SetBounds(pad, y, std::max(0, pw - 2 * pad), ScaleDip(40));
}

}  // namespace mgis
