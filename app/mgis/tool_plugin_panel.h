// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#ifndef APP_MGIS_TOOL_PLUGIN_PANEL_H_
#define APP_MGIS_TOOL_PLUGIN_PANEL_H_

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "ui/views/view.h"

namespace views {
class Label;
}

namespace mgis {

// Right-rail shell for tools / plugins (empty in the first knife).
class ToolPluginPanel : public views::View {
 public:
  ToolPluginPanel();
  virtual ~ToolPluginPanel();

  virtual void Layout() OVERRIDE;
  virtual void OnPaint(gfx::Canvas* canvas) OVERRIDE;

 private:
  views::Label* title_;
  views::Label* body_;

  DISALLOW_COPY_AND_ASSIGN(ToolPluginPanel);
};

}  // namespace mgis

#endif  // APP_MGIS_TOOL_PLUGIN_PANEL_H_
