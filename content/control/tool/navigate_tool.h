// Copyright (c) 2024 The mgis Authors.
// All rights reserved.

#ifndef CONTENT_CONTROL_TOOL_NAVIGATE_TOOL_H
#define CONTENT_CONTROL_TOOL_NAVIGATE_TOOL_H

#include "content/common/tool.h"
#include "content/control/tool/common.h"

namespace content {
enum eCursorType {
  CT_LoupePlus,
  CT_LoupeMinus,
  CT_Move,
  CT_Identify
};

class CONTENT_EXPORT NavigateTool : public Tool {
 public:
  NavigateTool();
  virtual ~NavigateTool();

  int Init(HWND hwnd, H2DRENDERDEVICE render_device,
           DelegateCommit delegate_commit = NULL, void *to_follow = NULL);
  int AuxDraw();
  int Timer();

 public:
  int Notify(MessageListener::Message &message);

  int SetCursor(void);
  int LButtonDown(uint32_t nFlags, Point point);
  int MouseMove(uint32_t nFlags, Point point);
  int LButtonUp(uint32_t nFlags, Point point);
  int RButtonDown(uint32_t nFlags, Point point);
  int MouseWheel(uint32_t nFlags, int16_t zDelta, Point point);

 protected:
  void ZoomMove(int16_t mouse_status, Point point);
  void ZoomIn(int16_t mouse_status, Point point);
  void ZoomOut(int16_t mouse_status, Point point);
  void ZoomRestore();
  void ZoomRefresh();

 protected:
  eViewMode view_model_;
  uint16_t flashed_;

 protected:
  Point origin_point_;
  Point current_point_;
  Point prev_point_;
  BOOL captured_;
  HCURSOR cursors_[4];
};
}  // namespace content

#endif  // CONTENT_CONTROL_TOOL_NAVIGATE_TOOL_H
