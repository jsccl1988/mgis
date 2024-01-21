
// Copyright (c) 2024 The MGIS Authors.
// All rights reserved.
#ifndef CONTENT_CONTROL_TOOL_FLASH_TOOL_H
#define CONTENT_CONTROL_TOOL_FLASH_TOOL_H

#include "content/common/tool.h"
#include "content/control/tool/common.h"

namespace content {
class FlashTool : public Tool {
 public:
  FlashTool();
  virtual ~FlashTool();
  int Init(HWND hwnd, H2DRENDERDEVICE render_device,
           DelegateCommit delegate_commit = NULL, void *to_follow = NULL);
  int AuxDraw();
  int Timer();

 public:
  int Notify(MessageListener::Message &message);

 protected:
  OGRLayer *result_layer_;

  std::string flash_style1_;
  std::string flash_style2_;
  std::string flash_style_;
  eFlashMode flash_mode_;

  bool is_flash_;
  bool is_style1_;
  double scale_delta_;
};
}  // namespace content

#endif  // CONTENT_CONTROL_TOOL_FLASH_TOOL_H
