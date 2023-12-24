#include "content/core/message_listener.h"

#include <algorithm>

#include "base/logging.h"

namespace content {
MessageListener::MessageListener() {}
MessageListener::~MessageListener() {}

const char *MessageListener::GetName() const { return name_.c_str(); }
void MessageListener::SetName(const char *name) { name_ = name; }

bool MessageListener::AppendFunctionItems(const char *function, long message,
                                          long style) {
  bool ret = true;
  if (style & FIM_2DVIEW) {
    ret &= AppendFunctionItems(function, message, view2d_function_items_);
  }

  if (style & FIM_3DVIEW) {
    ret &= AppendFunctionItems(function, message, view3d_function_items_);
  }

  if (style & FIM_3DEXVIEW) {
    ret &= AppendFunctionItems(function, message, view3dex_function_items_);
  }

  if (style & FIM_MAPDOCCATALOG) {
    ret &=
        AppendFunctionItems(function, message, map_doc_catalog_function_items_);
  }

  if (style & FIM_2DMFTOOLBAR) {
    ret &= AppendFunctionItems(function, message, toolbar2d_function_items_);
  }

  if (style & FIM_3DMFTOOLBAR) {
    ret &= AppendFunctionItems(function, message, toolbar3d_function_items_);
  }

  if (style & FIM_2DMFMENU) {
    ret &= AppendFunctionItems(function, message, menu2d_function_items_);
  }

  if (style & FIM_3DMFMENU) {
    ret &= AppendFunctionItems(function, message, menu3d_function_items_);
  }

  if (style & FIM_AUXMODULEBOX) {
    ret &= AppendFunctionItems(function, message, am_box_function_items_);
  }

  if (style & FIM_AUXMODULETREE) {
    ret &= AppendFunctionItems(function, message, am_tree_function_items_);
  }

  return ret;
}

FunctionItems MessageListener::GetFunctionItems(FuntionItemRoute style) {
  FunctionItems temp_function_items;
  switch (style) {
    case FIM_2DVIEW: {
      temp_function_items = view2d_function_items_;
    } break;
    case FIM_3DVIEW: {
      temp_function_items = view3d_function_items_;
    } break;
    case FIM_3DEXVIEW: {
      temp_function_items = view3dex_function_items_;
    } break;
    case FIM_MAPDOCCATALOG: {
      temp_function_items = map_doc_catalog_function_items_;
    } break;
    case FIM_2DMFTOOLBAR: {
      temp_function_items = toolbar2d_function_items_;
    } break;
    case FIM_3DMFTOOLBAR: {
      temp_function_items = toolbar3d_function_items_;
    } break;
    case FIM_2DMFMENU: {
      temp_function_items = menu2d_function_items_;
    } break;
    case FIM_3DMFMENU: {
      temp_function_items = menu3d_function_items_;
    } break;
    case FIM_AUXMODULEBOX: {
      temp_function_items = am_box_function_items_;
    } break;
    case FIM_AUXMODULETREE: {
      temp_function_items = am_tree_function_items_;
    } break;
  }

  return temp_function_items;
}

bool MessageListener::AppendFunctionItems(const char *function, long message,
                                          FunctionItems &function_items) {
  FunctionItems::iterator iter = function_items.begin();
  while (iter != function_items.end()) {
    if ((*iter).message == message) {
      return false;
    }
    ++iter;
  }

  FuntionItem funcItem;

  funcItem.name = function;
  funcItem.message = message;

  function_items.push_back(funcItem);

  return true;
}

bool MessageListener::AppendMessage(long message) {
  Messages::iterator iter = find(messages_.begin(), messages_.end(), message);
  if (iter != messages_.end()) return false;

  messages_.push_back(message);

  return true;
}

int MessageListener::Register() {
  MessageCenter *listener_manager = MessageCenter::GetSingletonPtr();
  return listener_manager->RegisterListener(this);
}

int MessageListener::RegisterMessage() {
  MessageCenter *listener_manager = MessageCenter::GetSingletonPtr();
  listener_manager->RegisterListenerMessage(this);

  return ERR_NONE;
}

int MessageListener::UnRegister() {
  MessageCenter *listener_manager = MessageCenter::GetSingletonPtr();
  return listener_manager->RemoveListener(this);
}

int MessageListener::UnRegisterMessage() {
  MessageCenter *listener_manager = MessageCenter::GetSingletonPtr();
  listener_manager->UnRegisterListenerMessage(this);

  return ERR_NONE;
}

int MessageListener::SetActive() {
  MessageCenter *listener_manager = MessageCenter::GetSingletonPtr();
  listener_manager->SetActiveListener(this);

  return ERR_NONE;
}

MessageCenter *MessageCenter::singleton_ = nullptr;
MessageCenter *MessageCenter::GetSingletonPtr(void) {
  if (singleton_ == nullptr) {
    singleton_ = new MessageCenter();
  }

  return singleton_;
}

void MessageCenter::DestoryInstance(void) { SAFE_DELETE(singleton_); }

MessageCenter::MessageCenter(void) { current_message_listener_ = nullptr; }
MessageCenter::~MessageCenter(void) { RemoveAllListener(); }

long MessageCenter::Notify(MessageListener *message_listener, long message,
                           MessageListener::Message &param) {
  if (message_listener == MESSAGE_LISTENER_INVALID) {
    return ERR_INVALID_PARAM;
  }

  if (message_listener == MESSAGE_LISTENER_BROADCAST) {
    Message2Ptr::iterator mapIter;
    mapIter = message2message_listeners_.find(message);
    if (mapIter != message2message_listeners_.end()) {
      MessageListener *pFindListener = (MessageListener *)mapIter->second;
      if (pFindListener) {
        pFindListener->Notify(message, param);
      }
    } else {
      MessageListeners done_listeners;
      MessageListeners ::iterator it = message_listeners_.begin();
      while (it != message_listeners_.end()) {
        if (find(done_listeners.begin(), done_listeners.end(), *it) ==
            done_listeners.end()) {
          param.modify = false;
          done_listeners.push_back(*it);
          (*it++)->Notify(message, param);
          if (param.modify) {
            it = message_listeners_.begin();
            continue;
          }
        } else
          ++it;
      }
    }
  } else {
    message_listener->Notify(message, param);
  }

  return ERR_NONE;
}

long MessageCenter::RegisterListener(MessageListener *message_listener) {
  MessageListeners::iterator i = message_listeners_.begin();

  while (i != message_listeners_.end()) {
    if ((*i) == message_listener) {
      return false;
    }
    ++i;
  }

  message_listeners_.push_back(message_listener);
  return ERR_NONE;
}

long MessageCenter::RemoveListener(MessageListener *message_listener) {
  MessageListeners::iterator i = message_listeners_.begin();
  while (i != message_listeners_.end()) {
    if ((*i) == message_listener) {
      message_listeners_.erase(i);
      break;
    }
    ++i;
  }

  return true;
}

long MessageCenter::RemoveAllListener(void) {
  message_listeners_.clear();

  return ERR_NONE;
}

MessageListener *MessageCenter::GetListener(int index) {
  if (index < 0 || index >= message_listeners_.size()) return nullptr;

  return message_listeners_.at(index);
}

const MessageListener *MessageCenter::GetListener(int index) const {
  if (index < 0 || index >= message_listeners_.size()) return nullptr;

  return message_listeners_.at(index);
}

long MessageCenter::RegisterListenerMessage(MessageListener *message_listener) {
  if (message_listener == nullptr) return ERR_INVALID_PARAM;

  Messages Messages = message_listener->GetMessages();

  Message2Ptr::iterator mapIter = message2message_listeners_.begin();
  Messages::iterator iter = Messages.begin();
  while (iter != Messages.end()) {
    if (message2message_listeners_.find((*iter)) ==
        message2message_listeners_.end()) {
      message2message_listeners_.insert(
          Message2PtrPair((*iter), (void *)message_listener));
    }
    ++iter;
  }

  return ERR_NONE;
}

long MessageCenter::UnRegisterListenerMessage(
    MessageListener *message_listener) {
  if (message_listener == nullptr) return ERR_INVALID_PARAM;

  Messages Messages = message_listener->GetMessages();

  Message2Ptr::iterator mapIter = message2message_listeners_.begin();
  Messages::iterator iter = Messages.begin();

  while (iter != Messages.end()) {
    if ((mapIter = message2message_listeners_.find((*iter))) !=
        message2message_listeners_.end()) {
      message2message_listeners_.erase(mapIter);
    }
    ++iter;
  }

  return ERR_NONE;
}
}  // namespace content