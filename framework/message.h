#ifndef FRAMEWORK_EXPORT_MESSAGE_H
#define FRAMEWORK_EXPORT_MESSAGE_H

#include "framework/framework.h"

#define MESSAGE_INVALID (-1)  //无效消息

#define MESSAGE_SYS_MESSAGE (0x0000)  //系统消息
#define MESSAGE_2D_TOOL (0x1000)      //二维交互工具消息
#define MESSAGE_3D_TOOL (0x2000)      //三维交互工具消息
#define MESSAGE_CMD_BEGIN (0x3001)    // cmd消息
#define MESSAGE_CMD_END (0x4000)      // cmd消息
#define MESSAGE_USER_BEGIN (0x4001)   //自定义消息
#define MESSAGE_USER_END (0x8000)     //自定义消息

#define LISTENER_MESSAGE_BROADCAST ((framework::Listener *)0xFFFF)
#define LISTENER_MESSAGE_INVALID ((framework::Listener *)0x0000)

#define MESSAGE_GET_SYS_2DVIEW (MESSAGE_SYS_MESSAGE + 1)
#define MESSAGE_GET_SYS_2DEDITVIEW (MESSAGE_SYS_MESSAGE + 2)
#define MESSAGE_GET_SYS_3DVIEW (MESSAGE_SYS_MESSAGE + 3)

namespace framework {

struct FuntionItem {
  std::string name{"default"};
  long message{0};
};

enum FuncItemStyle {
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

struct ListenerMessage {
  HWND source_window{nullptr};
  WPARAM wparam{NULL};
  LPARAM lparam{NULL};
  void *to_follow{nullptr};
  bool modify{false};

  operator LPARAM() { return LPARAM(this); }
  operator WPARAM() { return WPARAM(this); }
};

#define MESSAGE_KEY(message, hWnd) (MAKELONG(message, hWnd))
#define MESSAGE_KEY_HWND(msgKey) (HIWORD(msgKey))
#define MESSAGE_KEY_LMESSAGE(msgKey) (LOWORD(msgKey))

using function_items = std::vector<FuntionItem>;
using View2DFunctionItems = function_items;
using View3DFunctionItems = function_items;
using View3DExFunctionItems = function_items;
using MapDocCatalogFunctionItems = function_items;
using ToolBar2DFunctionItems = function_items;
using ToolBar3DFunctionItems = function_items;
using Menu2DFunctionItems = function_items;
using Menu3DFunctionItems = function_items;
using AMBoxFunctionItems = function_items;
using AMTreeFunctionItems = function_items;

using Messages = std::vector<long>;
using Message2Ptr = std::map<long, void *>;
using Message2PtrPair = std::pair<long, void *>;
}  // namespace framework

#define POST_LISTENER_MESSAGE(listener, message, param) \
  {                                                     \
    framework::ListenerManager *listener_manager =           \
        framework::ListenerManager::GetSingletonPtr();       \
    listener_manager->Notify(listener, message, param); \
  }
#endif  // FRAMEWORK_EXPORT_MESSAGE_H
