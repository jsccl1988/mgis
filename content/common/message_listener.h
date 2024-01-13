// Copyright (c) 2024 The mgis Authors.
// All rights reserved.

#ifndef CONTENT_COMMON_LISTENER_H
#define CONTENT_COMMON_LISTENER_H

#include <map>
#include <string>
#include <vector>

#include "base/memory/singleton.h"
#include "base/synchronization/lock.h"
#include "content/content.h"
#include "content/content_export.h"
#include "content/common/message.h"

namespace content {
class CONTENT_EXPORT MessageListener {
 public:
  struct Message {
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

  virtual int Notify(long message, MessageListener::Message& param) = 0;

 public:
  const char* GetName() const;
  void SetName(const char* name);

  bool AppendFunctionItems(const char* function, long message,
                           long style = FIM_2DVIEW | FIM_3DVIEW);
  FunctionItems GetFunctionItems(FuntionItemRoute style);

  Messages GetMessages(void) { return messages_; }

 protected:
  bool AppendFunctionItems(const char* function, long message,
                           FunctionItems& FunctionItems);
  bool AppendMessage(long message);

 protected:
  std::string name_;

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

#define MESSAGE_LISTENER_BROADCAST ((content::MessageListener*)0xFFFF)
#define MESSAGE_LISTENER_INVALID ((content::MessageListener*)0x0000)

#define MESSAGE_KEY(message, hWnd) (MAKELONG(message, hWnd))
#define MESSAGE_KEY_HWND(msgKey) (HIWORD(msgKey))
#define MESSAGE_KEY_LMESSAGE(msgKey) (LOWORD(msgKey))

using MessageListeners = std::vector<MessageListener*>;
using Name2MessageListener = std::map<std::string, MessageListener>;

class CONTENT_EXPORT MessageCenter {
 public:
  virtual ~MessageCenter(void);

 public:
  static MessageCenter* GetSingletonPtr(void);
  static void DestoryInstance(void);

 public:
  long Notify(MessageListener* message_listener, long message,
              MessageListener::Message& param);

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

  int GetListenerSize(void) const { return message_listeners_.size(); }
  MessageListener* GetListener(int index);
  const MessageListener* GetListener(int index) const;

  long RegisterListenerMessage(MessageListener* message_listener);
  long UnRegisterListenerMessage(MessageListener* message_listener);

 protected:
  MessageListeners message_listeners_;
  MessageListener* current_message_listener_;
  Message2Ptr message2message_listeners_;

 private:
  MessageCenter(void);
  static MessageCenter* singleton_;
};

long PostListenerMessage(MessageListener* message_listener, long message,
                         MessageListener::Message& param) {
  MessageCenter* listener_manager = MessageCenter::GetSingletonPtr();
  return listener_manager->Notify(message_listener, message, param);
}

#define POST_MESSAGE_LISTENER_MESSAGE(listener, message, param) \
  {                                                             \
    content::MessageCenter* listener_manager =                  \
        content::MessageCenter::GetSingletonPtr();              \
    listener_manager->Notify(listener, message, param);         \
  }
}  // namespace content
#endif  // CONTENT_COMMON_LISTENER_H