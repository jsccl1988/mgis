// Copyright (c) 2024 The MGIS Authors.
// All rights reserved.
#include "content/control/tool/tool_factory.h"

#include "content/control/tool/flash_tool.h"
// #include "content/control/tool/input_line_tool.h"
// #include "content/control/tool/input_point_tool.h"
// #include "content/control/tool/input_surface_tool.h"
#include "content/control/tool/navigate_tool.h"
// #include "content/control/tool/select_tool.h"

namespace content {
int ToolFactory::CreateTool(Tool*& tool, ToolType type) {
  int ret = ERR_FAILURE;
  if (!tool) {
    switch (type) {
      case Navigate:
        tool = new NavigateTool();
        ret = ERR_NONE;
        break;
      // case Select:
      //   tool = new SelectTool();
      //   ret = ERR_NONE;
      //   break;
      case Flash:
        tool = new FlashTool();
        ret = ERR_NONE;
        break;
      // case InputPoint:
      //   tool = new InputPointTool();
      //   ret = ERR_NONE;
      //   break;
      // case InputLine:
      //   tool = new InputLineTool();
      //   ret = ERR_NONE;
      //   break;
      // case InputSurface:
      //   tool = new InputSurfaceTool();
      //   ret = ERR_NONE;
      //   break;
      default:
        break;
    }
  }

  return ret;
}

int ToolFactory::DestoryTool(Tool*& tool) {
  SAFE_DELETE(tool);
  return ERR_NONE;
}
}  // namespace content