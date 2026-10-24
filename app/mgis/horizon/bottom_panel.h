// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#ifndef APP_MGIS_HORIZON_BOTTOM_PANEL_H_
#define APP_MGIS_HORIZON_BOTTOM_PANEL_H_

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

// Bottom rail: Output / Console / Trace / Memory / Agent.
class BottomPanel : public views::View {
 public:
  BottomPanel();
  virtual ~BottomPanel();

  // |channel| 0 Output+Console, 1 Trace, 2 Agent, 3 Memory (ignored content).
  void ShowChannel(int channel, const std::vector<std::string>& utf8_lines);

  virtual void Layout() OVERRIDE;

 private:
  class LogPage;

  views::TabbedPane* tabs_;
  LogPage* output_;
  LogPage* console_;
  LogPage* trace_;
  LogPage* memory_;
  LogPage* agent_;

  DISALLOW_COPY_AND_ASSIGN(BottomPanel);
};

}  // namespace mgis

#endif  // APP_MGIS_HORIZON_BOTTOM_PANEL_H_
