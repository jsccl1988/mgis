// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#ifndef APP_MGIS_BOTTOM_PANEL_H_
#define APP_MGIS_BOTTOM_PANEL_H_

#include <string>
#include <vector>

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "ui/views/view.h"

namespace views {
class Label;
class TabbedPane;
}

namespace mgis {

// Bottom rail: console / trace / agent transcripts.
class BottomPanel : public views::View {
 public:
  BottomPanel();
  virtual ~BottomPanel();

  // |channel| 0 console, 1 trace, 2 agent. Shows the tail of |utf8_lines|.
  void ShowChannel(int channel, const std::vector<std::string>& utf8_lines);

  virtual void Layout() OVERRIDE;

 private:
  class LogPage;

  views::TabbedPane* tabs_;
  LogPage* console_;
  LogPage* trace_;
  LogPage* agent_;

  DISALLOW_COPY_AND_ASSIGN(BottomPanel);
};

}  // namespace mgis

#endif  // APP_MGIS_BOTTOM_PANEL_H_
