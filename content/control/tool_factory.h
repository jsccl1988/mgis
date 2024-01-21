// Copyright (c) 2024 The mgis Authors.
// All rights reserved.
#ifndef CONTENT_CONTROL_TOOL_FACTORY_H
#define CONTENT_CONTROL_TOOL_FACTORY_H

#include "content/common/tool.h"

namespace content {
class CONTENT_EXPORT ToolFactory {
 public:
  enum ToolType {
    Navigate,
    Flash,
    Select,
    InputPoint,
    InputLine,
    InputSurface,
  };

  static int CreateTool(Tool*& tool, ToolType type);
  static int DestoryTool(Tool*& tool);

 private:
  ToolFactory(void) {}
  ~ToolFactory(void) {}
};
}  // namespace content

#endif  // CONTENT_CONTROL_TOOL_FACTORY_H