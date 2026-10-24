// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#ifndef APP_MGIS_HORIZON_TOOL_PLUGIN_PANEL_H_
#define APP_MGIS_HORIZON_TOOL_PLUGIN_PANEL_H_

#include <string>
#include <vector>

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/string16.h"
#include "ui/base/events/event.h"
#include "ui/views/controls/button/button.h"
#include "ui/views/controls/textfield/textfield_controller.h"
#include "ui/views/view.h"

namespace content {
class PluginHost;
}

namespace views {
class Label;
class TabbedPane;
class TextButton;
class Textfield;
}

namespace mgis {

// Right rail: Inspect|Map|Process|Extend group bar; Extend hosts Plugins.
class ToolPluginPanel : public views::View,
                        public views::ButtonListener,
                        public views::TextfieldController {
 public:
  enum Group {
    kInspect = 0,
    kMap,
    kProcess,
    kExtend,
    kGroupCount
  };

  ToolPluginPanel();
  virtual ~ToolPluginPanel();

  // Unowned. NULL clears the list.
  void SetPluginHost(content::PluginHost* host);
  void RefreshCommands();

  virtual void Layout() OVERRIDE;
  virtual void OnPaint(gfx::Canvas* canvas) OVERRIDE;
  virtual void ButtonPressed(views::Button* sender,
                             const ui::Event& event) OVERRIDE;

  // TextfieldController:
  virtual void ContentsChanged(views::Textfield* sender,
                               const string16& new_contents) OVERRIDE;
  virtual bool HandleKeyEvent(views::Textfield* sender,
                              const ui::KeyEvent& key_event) OVERRIDE;

 private:
  void SelectGroup(Group group);
  void RebuildPluginList();
  void ClearPluginButtons();
  void RunSelected();
  void SetSelectedEnabled(bool enabled);

  content::PluginHost* host_;
  Group group_;
  views::TextButton* group_buttons_[kGroupCount];
  views::View* body_;
  views::Label* placeholder_;
  views::TabbedPane* extend_tabs_;
  views::View* plugins_page_;
  views::Textfield* filter_;
  views::Label* hint_;
  views::TextButton* enable_btn_;
  views::TextButton* disable_btn_;
  views::TextButton* run_btn_;
  std::vector<views::TextButton*> plugin_buttons_;
  std::vector<std::string> command_ids_;
  std::vector<bool> command_enabled_;
  int selected_index_;
  string16 filter_text_;

  DISALLOW_COPY_AND_ASSIGN(ToolPluginPanel);
};

}  // namespace mgis

#endif  // APP_MGIS_HORIZON_TOOL_PLUGIN_PANEL_H_
