#ifndef CONTENT_COMMON_MESSAGE_H
#define CONTENT_COMMON_MESSAGE_H

#include "content/content.h"

#define MESSAGE_INVALID (-1)  //无效消息

#define MESSAGE_SYS_MESSAGE (0x0000)  //系统消息
#define MESSAGE_2D_TOOL (0x1000)      //二维交互工具消息
#define MESSAGE_3D_TOOL (0x2000)      //三维交互工具消息
#define MESSAGE_CMD_BEGIN (0x3001)    // cmd消息
#define MESSAGE_CMD_END (0x4000)      // cmd消息
#define MESSAGE_USER_BEGIN (0x4001)   //自定义消息
#define MESSAGE_USER_END (0x8000)     //自定义消息

#define MESSAGE_GET_SYS_2DVIEW (MESSAGE_SYS_MESSAGE + 1)
#define MESSAGE_GET_SYS_2DEDITVIEW (MESSAGE_SYS_MESSAGE + 2)
#define MESSAGE_GET_SYS_3DVIEW (MESSAGE_SYS_MESSAGE + 3)

namespace content {
struct FuntionItem {
  std::string name{"default"};
  long message{0};
};

enum FuntionItemRoute {
  FIM_2DVIEW = 1 << 0,
  FIM_3DVIEW = 1 << 1,
  FIM_3DEXVIEW = 1 << 2,
  FIM_MAPDOCCATALOG = 1 << 3,
  FIM_2DMFTOOLBAR = 1 << 4,
  FIM_3DMFTOOLBAR = 1 << 5,
  FIM_2DMFMENU = 1 << 6,
  FIM_3DMFMENU = 1 << 7,
  FIM_AUXMODULEBOX = 1 << 8,
  FIM_AUXMODULETREE = 1 << 9,
};

using FunctionItems = std::vector<FuntionItem>;
using View2DFunctionItems = FunctionItems;
using View3DFunctionItems = FunctionItems;
using View3DExFunctionItems = FunctionItems;
using MapDocCatalogFunctionItems = FunctionItems;
using ToolBar2DFunctionItems = FunctionItems;
using ToolBar3DFunctionItems = FunctionItems;
using Menu2DFunctionItems = FunctionItems;
using Menu3DFunctionItems = FunctionItems;
using AMBoxFunctionItems = FunctionItems;
using AMTreeFunctionItems = FunctionItems;

using Messages = std::vector<long>;
using Message2Ptr = std::map<long, void *>;
using Message2PtrPair = std::pair<long, void *>;
}  // namespace content
#endif  // CONTENT_COMMON_MESSAGE_H
