// Copyright (c) 2024 The MGIS Authors.
// All rights reserved.
#ifndef CONTENT_CONTROL_TOOL_SELECT_TOOL_H
#define CONTENT_CONTROL_TOOL_SELECT_TOOL_H

#include "content/common/tool.h"
#include "content/control/tool/common.h"

namespace content {
class SelectTool : public Tool {
 public:
  SelectTool();
  virtual ~SelectTool() override;

  int Init(HWND hwnd, H2DRENDERDEVICE render_device,
           DelegateCommit delegate_commit = nullptr,
           void *to_follow = nullptr) override;
  int AuxDraw() override;
  int Timer() override;

 public:
  int KeyDown(uint32_t nChar, uint32_t nRepCnt, uint32_t flags) override;

 public:
  int Notify(MessageListener::Message &message) override;

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
  int layer_feature_type_;

  OGRGeometry *geometry_;
  OGRLayer *result_layer_;
  double select_margin_;
};
}  // namespace content

#endif  // CONTENT_CONTROL_TOOL_SELECT_TOOL_H
