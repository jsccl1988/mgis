#include "framework/listener.h"

#include <algorithm>

#include "base/logging.h"
#include "framework/message.h"

namespace framework {
using base::Err;
Listener::Listener() {}
Listener::~Listener() {}

const char *Listener::GetName() const { return name_.c_str(); }
void Listener::SetName(const char *name) { name_ = name; }

bool Listener::AppendFuncItems(const char *function, long message, long style) {
  bool ret = true;
  if (style & FIM_2DVIEW) {
    ret &= AppendFuncItems(function, message, view2d_function_items_);
  }

  if (style & FIM_3DVIEW) {
    ret &= AppendFuncItems(function, message, view3d_function_items_);
  }

  if (style & FIM_3DEXVIEW) {
    ret &= AppendFuncItems(function, message, view3dex_function_items_);
  }

  if (style & FIM_MAPDOCCATALOG) {
    ret &= AppendFuncItems(function, message, map_doc_catalog_function_items_);
  }

  if (style & FIM_2DMFTOOLBAR) {
    ret &= AppendFuncItems(function, message, toolbar2d_function_items_);
  }

  if (style & FIM_3DMFTOOLBAR) {
    ret &= AppendFuncItems(function, message, toolbar3d_function_items_);
  }

  if (style & FIM_2DMFMENU) {
    ret &= AppendFuncItems(function, message, menu2d_function_items_);
  }

  if (style & FIM_3DMFMENU) {
    ret &= AppendFuncItems(function, message, menu3d_function_items_);
  }

  if (style & FIM_AUXMODULEBOX) {
    ret &= AppendFuncItems(function, message, am_box_function_items_);
  }

  if (style & FIM_AUXMODULETREE) {
    ret &= AppendFuncItems(function, message, am_tree_function_items_);
  }

  return ret;
}

function_items Listener::GetFuncItems(FuncItemStyle style) {
  function_items temp_function_items;
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

bool Listener::AppendFuncItems(const char *function, long message,
                               function_items &function_items) {
  function_items::iterator iter = function_items.begin();
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

bool Listener::AppendMessage(long message) {
  Messages::iterator iter = find(messages_.begin(), messages_.end(), message);
  if (iter != messages_.end()) return false;

  messages_.push_back(message);

  return true;
}

int Listener::Register() {
  ListenerManager *listener_manager = ListenerManager::GetSingletonPtr();
  return listener_manager->RegisterListener(this);
}

int Listener::RegisterMessage() {
  ListenerManager *listener_manager = ListenerManager::GetSingletonPtr();
  listener_manager->RegisterListenerMessage(this);

  return base::ERR_NONE;
}

int Listener::UnRegister() {
  ListenerManager *listener_manager = ListenerManager::GetSingletonPtr();
  return listener_manager->RemoveListener(this);
}

int Listener::UnRegisterMessage() {
  ListenerManager *listener_manager = ListenerManager::GetSingletonPtr();
  listener_manager->UnRegisterListenerMessage(this);

  return base::ERR_NONE;
}

int Listener::SetActive() {
  ListenerManager *listener_manager = ListenerManager::GetSingletonPtr();
  listener_manager->SetActiveListener(this);

  return base::ERR_NONE;
}

ListenerManager *ListenerManager::singleton_ = nullptr;
ListenerManager *ListenerManager::GetSingletonPtr(void) {
  if (singleton_ == nullptr) {
    singleton_ = new ListenerManager();
  }

  return singleton_;
}

void ListenerManager::DestoryInstance(void) { SAFE_DELETE(singleton_); }

ListenerManager::ListenerManager(void) { current_listener_ = nullptr; }
ListenerManager::~ListenerManager(void) { RemoveAllListener(); }

long ListenerManager::Notify(Listener *listener, long message,
                             ListenerMessage &param) {
  if (listener == LISTENER_MESSAGE_INVALID) {
    return base::ERR_INVALID_PARAM;
  }

  if (listener == LISTENER_MESSAGE_BROADCAST) {
    Message2Ptr::iterator mapIter;
    mapIter = message2listeners_.find(message);
    if (mapIter != message2listeners_.end()) {
      Listener *pFindListener = (Listener *)mapIter->second;
      if (pFindListener) {
        pFindListener->Notify(message, param);
      }
    } else {
      Listeners done_listeners;
      Listeners ::iterator it = listeners_.begin();
      while (it != listeners_.end()) {
        if (find(done_listeners.begin(), done_listeners.end(), *it) ==
            done_listeners.end()) {
          param.modify = false;
          done_listeners.push_back(*it);
          (*it++)->Notify(message, param);
          if (param.modify) {
            it = listeners_.begin();
            continue;
          }
        } else
          ++it;
      }
    }
  } else {
    listener->Notify(message, param);
  }

  return base::ERR_NONE;
}

long ListenerManager::RegisterListener(Listener *listener) {
  Listeners::iterator i = listeners_.begin();

  while (i != listeners_.end()) {
    if ((*i) == listener) {
      return false;
    }
    ++i;
  }

  listeners_.push_back(listener);
  return base::ERR_NONE;
}

long ListenerManager::RemoveListener(Listener *listener) {
  Listeners::iterator i = listeners_.begin();
  while (i != listeners_.end()) {
    if ((*i) == listener) {
      listeners_.erase(i);
      break;
    }
    ++i;
  }

  return true;
}

long ListenerManager::RemoveAllListener(void) {
  listeners_.clear();

  return base::ERR_NONE;
}

Listener *ListenerManager::GetListener(int index) {
  if (index < 0 || index >= listeners_.size()) return nullptr;

  return listeners_.at(index);
}

const Listener *ListenerManager::GetListener(int index) const {
  if (index < 0 || index >= listeners_.size()) return nullptr;

  return listeners_.at(index);
}

long ListenerManager::RegisterListenerMessage(Listener *listener) {
  if (listener == nullptr) return base::ERR_INVALID_PARAM;

  Messages Messages = listener->GetMessages();

  Message2Ptr::iterator mapIter = message2listeners_.begin();
  Messages::iterator iter = Messages.begin();
  while (iter != Messages.end()) {
    if (message2listeners_.find((*iter)) == message2listeners_.end()) {
      message2listeners_.insert(Message2PtrPair((*iter), (void *)listener));
    }
    ++iter;
  }

  return base::ERR_NONE;
}

long ListenerManager::UnRegisterListenerMessage(Listener *listener) {
  if (listener == nullptr) return base::ERR_INVALID_PARAM;

  Messages Messages = listener->GetMessages();

  Message2Ptr::iterator mapIter = message2listeners_.begin();
  Messages::iterator iter = Messages.begin();

  while (iter != Messages.end()) {
    if ((mapIter = message2listeners_.find((*iter))) !=
        message2listeners_.end()) {
      message2listeners_.erase(mapIter);
    }
    ++iter;
  }

  return base::ERR_NONE;
}
}  // namespace framework