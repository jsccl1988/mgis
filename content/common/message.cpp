// Copyright (c) 2024 The MGIS Authors.
// All rights reserved.

#include "content/common/message.h"

#include <algorithm>

#include "base/logging.h"

namespace content {
MessageListener::MessageListener() {}
MessageListener::~MessageListener() {}

const base::NameChar *MessageListener::GetName() const { return name_.c_str(); }
void MessageListener::SetName(const base::NameChar *name) { name_ = name; }

bool MessageListener::AppendFunctionItems(const base::NameChar *function,
                                          long message_id, long group) {
  bool ret = true;
  if (group & FIG_2DVIEW) {
    ret &= AppendFunctionItems(function, message_id, view2d_function_items_);
  }

  if (group & FIG_3DVIEW) {
    ret &= AppendFunctionItems(function, message_id, view3d_function_items_);
  }

  if (group & FIG_3DEXVIEW) {
    ret &= AppendFunctionItems(function, message_id, view3dex_function_items_);
  }

  if (group & FIG_MAPDOCCATALOG) {
    ret &= AppendFunctionItems(function, message_id,
                               map_doc_catalog_function_items_);
  }

  if (group & FIG_2DMFTOOLBAR) {
    ret &= AppendFunctionItems(function, message_id, toolbar2d_function_items_);
  }

  if (group & FIG_3DMFTOOLBAR) {
    ret &= AppendFunctionItems(function, message_id, toolbar3d_function_items_);
  }

  if (group & FIG_2DMFMENU) {
    ret &= AppendFunctionItems(function, message_id, menu2d_function_items_);
  }

  if (group & FIG_3DMFMENU) {
    ret &= AppendFunctionItems(function, message_id, menu3d_function_items_);
  }

  if (group & FIG_AUXMODULEBOX) {
    ret &= AppendFunctionItems(function, message_id, am_box_function_items_);
  }

  if (group & FIG_AUXMODULETREE) {
    ret &= AppendFunctionItems(function, message_id, am_tree_function_items_);
  }

  return ret;
}

FunctionItems MessageListener::GetFunctionItems(FuntionItemGroup group) {
  FunctionItems temp_function_items;
  switch (group) {
    case FIG_2DVIEW: {
      temp_function_items = view2d_function_items_;
    } break;
    case FIG_3DVIEW: {
      temp_function_items = view3d_function_items_;
    } break;
    case FIG_3DEXVIEW: {
      temp_function_items = view3dex_function_items_;
    } break;
    case FIG_MAPDOCCATALOG: {
      temp_function_items = map_doc_catalog_function_items_;
    } break;
    case FIG_2DMFTOOLBAR: {
      temp_function_items = toolbar2d_function_items_;
    } break;
    case FIG_3DMFTOOLBAR: {
      temp_function_items = toolbar3d_function_items_;
    } break;
    case FIG_2DMFMENU: {
      temp_function_items = menu2d_function_items_;
    } break;
    case FIG_3DMFMENU: {
      temp_function_items = menu3d_function_items_;
    } break;
    case FIG_AUXMODULEBOX: {
      temp_function_items = am_box_function_items_;
    } break;
    case FIG_AUXMODULETREE: {
      temp_function_items = am_tree_function_items_;
    } break;
  }

  return temp_function_items;
}

bool MessageListener::AppendFunctionItems(const base::NameChar *function,
                                          long message_id,
                                          FunctionItems &function_items) {
  FunctionItems::iterator iter = function_items.begin();
  while (iter != function_items.end()) {
    if ((*iter).message_id == message_id) {
      return false;
    }
    ++iter;
  }

  FuntionItem funcItem;

  funcItem.name = function;
  funcItem.message_id = message_id;

  function_items.push_back(funcItem);

  return true;
}

bool MessageListener::AppendMessage(long message_id) {
  Messages::iterator iter =
      find(messages_.begin(), messages_.end(), message_id);
  if (iter != messages_.end()) return false;

  messages_.push_back(message_id);

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

long MessageCenter::Notify(MessageListener *message_listener,
                           MessageListener::Message &message) {
  if (message_listener == MESSAGE_LISTENER_INVALID) {
    return ERR_INVALID_PARAM;
  }

  if (message_listener == MESSAGE_LISTENER_BROADCAST) {
    Message2Ptr::iterator mapIter;
    mapIter = message2message_listeners_.find(message.id);
    if (mapIter != message2message_listeners_.end()) {
      MessageListener *pFindListener = (MessageListener *)mapIter->second;
      if (pFindListener) {
        pFindListener->Notify(message);
      }
    } else {
      MessageListeners done_listeners;
      MessageListeners ::iterator it = message_listeners_.begin();
      while (it != message_listeners_.end()) {
        if (find(done_listeners.begin(), done_listeners.end(), *it) ==
            done_listeners.end()) {
          message.modify = false;
          done_listeners.push_back(*it);
          (*it++)->Notify(message);
          if (message.modify) {
            it = message_listeners_.begin();
            continue;
          }
        } else
          ++it;
      }
    }
  } else {
    message_listener->Notify(message);
  }

  return ERR_NONE;
}

long MessageCenter::RegisterListener(MessageListener *message_listener) {
  MessageListeners::iterator iter = message_listeners_.begin();

  while (iter != message_listeners_.end()) {
    if ((*iter) == message_listener) {
      return false;
    }
    ++iter;
  }

  message_listeners_.push_back(message_listener);
  return ERR_NONE;
}

long MessageCenter::RemoveListener(MessageListener *message_listener) {
  MessageListeners::iterator iter = message_listeners_.begin();
  while (iter != message_listeners_.end()) {
    if ((*iter) == message_listener) {
      message_listeners_.erase(iter);
      break;
    }
    ++iter;
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
  if (message_listener == nullptr) {
    return ERR_INVALID_PARAM;
  }

  Messages messages = message_listener->GetMessages();

  Message2Ptr::iterator mapIter = message2message_listeners_.begin();
  Messages::iterator iter = messages.begin();

  while (iter != messages.end()) {
    if ((mapIter = message2message_listeners_.find((*iter))) !=
        message2message_listeners_.end()) {
      message2message_listeners_.erase(mapIter);
    }
    ++iter;
  }

  return ERR_NONE;
}

HMENU CreateListenerMenu(MessageListener *message_listener,
                         FuntionItemGroup group) {
  if (message_listener == nullptr) {
    return NULL;
  }

  HMENU hMenu = ::CreatePopupMenu();
  AppendListenerMenu(hMenu, message_listener, group, false);

  return hMenu;
}

void AppendListenerMenu(HMENU hmenu, MessageListener *message_listener,
                        FuntionItemGroup group, bool insert_seperator) {
  if (NULL == hmenu || NULL == message_listener) {
    return;
  }

  if (insert_seperator) {
    ::AppendMenu(hmenu, MF_SEPARATOR, NULL, NULL);
  }

  FunctionItems items = message_listener->GetFunctionItems(group);
  FunctionItems::iterator iter = items.begin();
  while (iter != items.end()) {
    ::AppendMenu(hmenu, MF_STRING, (*iter).message_id, (*iter).name.c_str());
    ++iter;
  }
}

long PostListenerMessage(MessageListener *message_listener,
                         MessageListener::Message &message) {
  MessageCenter *listener_center = MessageCenter::GetSingletonPtr();
  return listener_center->Notify(message_listener, message);
}
}  // namespace content
