// Copyright (c) 2024 The mgis Authors.
// All rights reserved.
#ifndef CONTENT_CONTROL_TOOL_INPUT_POINT_TOOL_H
#define CONTENT_CONTROL_TOOL_INPUT_POINT_TOOL_H

#include "content/common/tool.h"
#include "content/control/tool/common.h"

namespace content {
class InputPointTool : public Tool {
 public:
  InputPointTool();
  virtual ~InputPointTool();

  int Init(HWND hwnd, H2DRENDERDEVICE render_device,
           DelegateCommit delegate_commit = NULL, void *to_follow = NULL);
  int AuxDraw();

 public:
  int Notify(MessageListener::Message &message);

  int LButtonDown(uint32_t nFlags, Point point);
  int MouseMove(uint32_t nFlags, Point point);
  int LButtonUp(uint32_t nFlags, Point point);
  int RButtonDown(uint32_t nFlags, Point point);
  int MouseWheel(uint32_t nFlags, int16_t zDelta, Point point);

 private:
  void AppendImage(int16_t mouse_status, Point point);
  void AppendText(int16_t mouse_status, Point point);
  void AppendDot(int16_t mouse_status, Point point);

  void EndAppendPoint();

 protected:
  OGRGeometry *geometry_;
  uint16_t append_type_;
  float angle_;

 private:
  Point origin_point_;
  Point current_point_;
  Point prev_point_;
  BOOL is_drag_;
  bool is_delay_;
};
}  // namespace content

#endif  //CONTENT_CONTROL_TOOL_INPUT_POINT_TOOL_H
