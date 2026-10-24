// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#ifndef APP_MGIS_HORIZON_MAP_TOOLBAR_H_
#define APP_MGIS_HORIZON_MAP_TOOLBAR_H_

#include <string>
#include <vector>

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "ui/base/events/event.h"
#include "ui/views/controls/button/button.h"
#include "ui/views/view.h"

namespace views {
class TextButton;
}

namespace mgis {

// Top map tool strip: selection / draw / undo. Forwards tool ids to the host.
class MapToolbar : public views::View, public views::ButtonListener {
 public:
  class Delegate {
   public:
    virtual void OnMapToolSelected(const std::string& tool_id) = 0;

   protected:
    virtual ~Delegate() {}
  };

  MapToolbar();
  virtual ~MapToolbar();

  void set_delegate(Delegate* delegate) { delegate_ = delegate; }

  virtual void Layout() OVERRIDE;
  virtual void OnPaint(gfx::Canvas* canvas) OVERRIDE;
  virtual void OnThemeChanged() OVERRIDE;
  virtual void ButtonPressed(views::Button* sender,
                             const ui::Event& event) OVERRIDE;

 private:
  struct ToolEntry {
    const char* label;
    const char* tool_id;
  };

  void AddToolButton(const ToolEntry& entry, int tag);

  Delegate* delegate_;
  std::vector<views::TextButton*> buttons_;
  std::vector<std::string> tool_ids_;

  DISALLOW_COPY_AND_ASSIGN(MapToolbar);
};

}  // namespace mgis

#endif  // APP_MGIS_HORIZON_MAP_TOOLBAR_H_
