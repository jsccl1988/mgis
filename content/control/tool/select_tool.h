// Copyright (c) 2024 The MGIS Authors.
// All rights reserved.
#ifndef CONTENT_CONTROL_TOOL_SELECT_TOOL_H
#define CONTENT_CONTROL_TOOL_SELECT_TOOL_H

#include "content/common/tool.h"

namespace content {
class SelectTool : public Tool {
 public:
  SelectTool();
  virtual ~SelectTool();

  int Init(HWND hwnd, H2DRENDERDEVICE render_device,
           DelegateCommit delegate_commit = NULL, void *to_follow = NULL);
  int AuxDraw();
  int Timer();

 public:
  int KeyDown(uint32_t nChar, uint32_t nRepCnt, uint32_t nFlags);

 public:
  int Notify(MessageListener::Message &message);

 protected:
  void OnRetDelegate(int nRetType);
  void OnSetSelMode(void);

 protected:
  Point origin_point_;
  Point current_point_;
  Point prev_point_;
  bool captured_;
  int step_;

 protected:
  eSelectMode select_mode_;
  int m_nLayerFeaType;

  OGRLayer *result_layer_;
  GQueryDesc m_gQDes;
  PQueryDesc m_pQDes;
  double select_margin_;
};
}  // namespace content

#endif  // CONTENT_CONTROL_TOOL_SELECT_TOOL_H
