// Copyright (c) 2024 The MGIS Authors.
// All rights reserved.

#ifndef CONTENT_COMMON_MESSAGE_H
#define CONTENT_COMMON_MESSAGE_H

#include <map>
#include <string>
#include <vector>

#include "base/memory/singleton.h"
#include "base/synchronization/lock.h"
#include "content/content.h"
#include "content/content_export.h"

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
  base::NameString name{L"default"};
  long message_id{0};
};

enum FuntionItemGroup {
  FIG_2DVIEW = 1 << 0,
  FIG_3DVIEW = 1 << 1,
  FIG_3DEXVIEW = 1 << 2,
  FIG_MAPDOCCATALOG = 1 << 3,
  FIG_2DMFTOOLBAR = 1 << 4,
  FIG_3DMFTOOLBAR = 1 << 5,
  FIG_2DMFMENU = 1 << 6,
  FIG_3DMFMENU = 1 << 7,
  FIG_AUXMODULEBOX = 1 << 8,
  FIG_AUXMODULETREE = 1 << 9,
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
using Message2Ptr = std::map<long, void*>;
using Message2PtrPair = std::pair<long, void*>;

class MessageListener {
 public:
  struct Message {
    long id{0};
    HWND source_window{nullptr};
    WPARAM wparam{NULL};
    LPARAM lparam{NULL};
    void* to_follow{nullptr};
    bool modify{false};

    operator LPARAM() { return LPARAM(this); }
    operator WPARAM() { return WPARAM(this); }
  };

  MessageListener(void);
  virtual ~MessageListener(void);

 public:
  virtual int Register(void);
  virtual int RegisterMessage(void);

  virtual int UnRegister(void);
  virtual int UnRegisterMessage(void);

  virtual int SetActive();

  virtual int Notify(MessageListener::Message& message) = 0;

 public:
  const base::NameChar* GetName() const;
  void SetName(const base::NameChar* name);

  bool AppendFunctionItems(const base::NameChar* function, long message_id,
                           long group = FIG_2DVIEW | FIG_3DVIEW);
  FunctionItems GetFunctionItems(FuntionItemGroup group);

  Messages GetMessages(void) { return messages_; }

 protected:
  bool AppendFunctionItems(const base::NameChar* function, long message_id,
                           FunctionItems& FunctionItems);
  bool AppendMessage(long message_id);

 protected:
  base::NameString name_;

  View2DFunctionItems view2d_function_items_;
  View3DFunctionItems view3d_function_items_;
  View3DExFunctionItems view3dex_function_items_;
  MapDocCatalogFunctionItems map_doc_catalog_function_items_;
  ToolBar2DFunctionItems toolbar2d_function_items_;
  ToolBar3DFunctionItems toolbar3d_function_items_;
  Menu2DFunctionItems menu2d_function_items_;
  Menu3DFunctionItems menu3d_function_items_;
  AMBoxFunctionItems am_box_function_items_;
  AMTreeFunctionItems am_tree_function_items_;
  Messages messages_;
};

#define MESSAGE_LISTENER_BROADCAST ((MessageListener*)0xFFFF)
#define MESSAGE_LISTENER_INVALID ((MessageListener*)0x0000)

#define MESSAGE_KEY(message_id, hwnd) (MAKELONG(message_id, hwnd))
#define MESSAGE_KEY_HWND(key) (HIWORD(key))
#define MESSAGE_KEY_LMESSAGE(key) (LOWORD(key))

using MessageListeners = std::vector<MessageListener*>;
using Name2MessageListener = std::map<std::string, MessageListener>;

class MessageCenter : public base::Singleton<MessageCenter> {
 public:
  long Notify(MessageListener* message_listener,
              MessageListener::Message& message);

  long RegisterListener(MessageListener* message_listener);
  long RemoveListener(MessageListener* message_listener);
  long RemoveAllListener(void);

  void SetActiveListener(MessageListener* message_listener) {
    current_message_listener_ = message_listener;
  }
  MessageListener* GetActiveListener(void) { return current_message_listener_; }
  const MessageListener* GetActiveListener(void) const {
    return current_message_listener_;
  }

  size_t GetListenerSize(void) const { return message_listeners_.size(); }
  MessageListener* GetListener(int index);
  const MessageListener* GetListener(int index) const;

  long RegisterListenerMessage(MessageListener* message_listener);
  long UnRegisterListenerMessage(MessageListener* message_listener);

 protected:
  MessageListeners message_listeners_;
  MessageListener* current_message_listener_;
  Message2Ptr message2message_listeners_;
};

HMENU CreateListenerMenu(MessageListener* message_listener,
                         FuntionItemGroup group);

void AppendListenerMenu(HMENU ownwer_menu, MessageListener* message_listener,
                        FuntionItemGroup group, bool insert_seperator);

long PostListenerMessage(MessageListener* message_listener,
                         MessageListener::Message& message);
}  // namespace content

#define POST_MESSAGE_LISTENER_MESSAGE(listener, message) \
  {                                                      \
    content::MessageCenter& listener_manager =           \
        content::MessageCenter::GetSingleton();          \
    listener_manager.get()->Notify(listener, message);   \
  }

#endif  // CONTENT_COMMON_MESSAGE_H