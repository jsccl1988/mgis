// Copyright (c) 2024 The MGIS Authors.
// All rights reserved.
#ifndef CONTENT_CONTROL_TOOL_EDIT_TOOL_H
#define CONTENT_CONTROL_TOOL_EDIT_TOOL_H

#include "content/common/tool.h"
#include "content/control/tool/common.h"

namespace content {
class EditTool : public Tool {
 public:
  EditTool();
  virtual ~EditTool();

  int Init(HWND hwnd, H2DRENDERDEVICE render_device,
           DelegateCommit delegate_commit = nullptr, void *to_follow = nullptr);
  int AuxDraw();
  int Timer();

 public:
  int Notify(MessageListener::Message &message);

 protected:
  void OnRetDelegate(int nRetType);

 protected:
  void OnInputPointFeature(uint16_t type);
  void OnInputLineFeature(uint16_t type);
  void OnInputSurfaceFeature(uint16_t type);

 protected:
  void AppendPointFeature(uint16_t type);
  void AppendDotFeature();
  void AppendLineFeature(void);
  void AppendSurfaceFeature(void);

 protected:
  OGRGeometry *geometry_;
};
}  // namespace content

#endif  // CONTENT_CONTROL_TOOL_EDIT_TOOL_H
