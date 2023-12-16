#include "core/listener.h"

#include <algorithm>

#include "base/logging.h"

namespace core {
Listener::Listener() {}
Listener::~Listener() {}

const char *Listener::GetName() const { return m_szListenerName; }

void Listener::SetName(const char *szName) {
  strcpy(m_szListenerName, szName);
}

bool Listener::AppendFuncItems(const char *szFunc, long lFuncMessage,
                                  long lStyle) {
  bool bRet = true;
  if (lStyle & FIM_2DVIEW) {
    bRet &= AppendFuncItems(szFunc, lFuncMessage, m_v2DViewFuncItems);
  }

  if (lStyle & FIM_3DVIEW) {
    bRet &= AppendFuncItems(szFunc, lFuncMessage, m_v3DViewFuncItems);
  }

  if (lStyle & FIM_3DEXVIEW) {
    bRet &= AppendFuncItems(szFunc, lFuncMessage, m_v3DExViewFuncItems);
  }

  if (lStyle & FIM_MAPDOCCATALOG) {
    bRet &= AppendFuncItems(szFunc, lFuncMessage, m_vMDCatalogFuncItems);
  }

  if (lStyle & FIM_2DMFTOOLBAR) {
    bRet &= AppendFuncItems(szFunc, lFuncMessage, m_v2DToolBarFuncItems);
  }

  if (lStyle & FIM_3DMFTOOLBAR) {
    bRet &= AppendFuncItems(szFunc, lFuncMessage, m_v3DToolBarFuncItems);
  }

  if (lStyle & FIM_2DMFMENU) {
    bRet &= AppendFuncItems(szFunc, lFuncMessage, m_v2DMMenuFuncItems);
  }

  if (lStyle & FIM_3DMFMENU) {
    bRet &= AppendFuncItems(szFunc, lFuncMessage, m_v3DMMenuFuncItems);
  }

  if (lStyle & FIM_AUXMODULEBOX) {
    bRet &= AppendFuncItems(szFunc, lFuncMessage, m_vAMBoxFuncItems);
  }

  if (lStyle & FIM_AUXMODULETREE) {
    bRet &= AppendFuncItems(szFunc, lFuncMessage, m_vAMTreeFuncItems);
  }

  return bRet;
}

FunctionItems Listener::GetFuncItems(FuncItemStyle style) {
  FunctionItems vTmpFuncItems;
  switch (style) {
    case FIM_2DVIEW: {
      vTmpFuncItems = m_v2DViewFuncItems;
    } break;
    case FIM_3DVIEW: {
      vTmpFuncItems = m_v3DViewFuncItems;
    } break;
    case FIM_3DEXVIEW: {
      vTmpFuncItems = m_v3DExViewFuncItems;
    } break;
    case FIM_MAPDOCCATALOG: {
      vTmpFuncItems = m_vMDCatalogFuncItems;
    } break;
    case FIM_2DMFTOOLBAR: {
      vTmpFuncItems = m_v2DToolBarFuncItems;
    } break;
    case FIM_3DMFTOOLBAR: {
      vTmpFuncItems = m_v3DToolBarFuncItems;
    } break;
    case FIM_2DMFMENU: {
      vTmpFuncItems = m_v2DMMenuFuncItems;
    } break;
    case FIM_3DMFMENU: {
      vTmpFuncItems = m_v3DMMenuFuncItems;
    } break;
    case FIM_AUXMODULEBOX: {
      vTmpFuncItems = m_vAMBoxFuncItems;
    } break;
    case FIM_AUXMODULETREE: {
      vTmpFuncItems = m_vAMTreeFuncItems;
    } break;
  }

  return vTmpFuncItems;
}

bool Listener::AppendFuncItems(const char *szFunc, long lFuncMessage,
                                  FunctionItems &FunctionItems) {
  FunctionItems::iterator iter = FunctionItems.begin();
  while (iter != FunctionItems.end()) {
    if ((*iter).lMessage == lFuncMessage) {
      return false;
    }
    ++iter;
  }

  FuntionItem funcItem;

  strcpy(funcItem.szName, szFunc);
  funcItem.lMessage = lFuncMessage;

  FunctionItems.push_back(funcItem);

  return true;
}

bool Listener::AppendMessage(long lFuncMessage) {
  Messages::iterator iter = find(m_vMessages.begin(), m_vMessages.end(), lFuncMessage);
  if (iter != m_vMessages.end()) return false;

  m_vMessages.push_back(lFuncMessage);

  return true;
}

int Listener::Register() {
  ListenerManager *listener_manager = ListenerManager::GetSingletonPtr();
  return listener_manager->RegisterListener(this);
}

int Listener::RegisterMessage() {
  ListenerManager *listener_manager = ListenerManager::GetSingletonPtr();
  listener_manager->RegisterListenerMessage(this);

  return SMT_ERR_NONE;
}

int Listener::UnRegister() {
  ListenerManager *listener_manager = ListenerManager::GetSingletonPtr();
  return listener_manager->RemoveListener(this);
}

int Listener::UnRegisterMessage() {
  ListenerManager *listener_manager = ListenerManager::GetSingletonPtr();
  listener_manager->UnRegisterListenerMessage(this);

  return SMT_ERR_NONE;
}

int Listener::SetActive() {
  ListenerManager *listener_manager = ListenerManager::GetSingletonPtr();
  listener_manager->SetActiveListener(this);

  return SMT_ERR_NONE;
}

ListenerManager *ListenerManager::singleton_ = NULL;
ListenerManager *ListenerManager::GetSingletonPtr(void) {
  CSLock cslock;
  ScopeCSLock scope(&cslock);

  if (singleton_ == NULL) {
    singleton_ = new ListenerManager();
  }

  return singleton_;
}

void ListenerManager::DestoryInstance(void) {
  SMT_SAFE_DELETE(singleton_);
}

ListenerManager::ListenerManager(void) { current_listener_ = NULL; }
ListenerManager::~ListenerManager(void) { RemoveAllListener(); }

long ListenerManager::Notify(Listener *listener, long lMessage,
                                ListenerMessage &param) {
  if (listener == SMT_LISTENER_MSG_INVALID) {
    return SMT_ERR_INVALID_PARAM;
  }

  if (listener == SMT_LISTENER_MSG_BROADCAST) {
    Message2Ptr::iterator mapIter;
    mapIter = message2listeners_.find(lMessage);
    if (mapIter != message2listeners_.end()) {
      Listener *pFindListener = (Listener *)mapIter->second;
      if (pFindListener) {
        pFindListener->Notify(lMessage, param);
      }
    } else {
      vListenerPtrs vDoneList;
      vListenerPtrs ::iterator it = listeners_.begin();
      while (it != listeners_.end()) {
        if (find(vDoneList.begin(), vDoneList.end(), *it) == vDoneList.end()) {
          param.bModify = false;
          vDoneList.push_back(*it);
          (*it++)->Notify(lMessage, param);
          if (param.bModify) {
            it = listeners_.begin();
            continue;
          }
        } else
          ++it;
      }
    }
  } else {
    listener->Notify(lMessage, param);
  }

  return SMT_ERR_NONE;
}

long ListenerManager::RegisterListener(Listener *listener) {
  vListenerPtrs::iterator i = listeners_.begin();

  while (i != listeners_.end()) {
    if ((*i) == listener) {
      return false;
    }
    ++i;
  }

  listeners_.push_back(listener);
  return SMT_ERR_NONE;
}

long ListenerManager::RemoveListener(Listener *listener) {
  vListenerPtrs::iterator i = listeners_.begin();
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

  return SMT_ERR_NONE;
}

Listener *ListenerManager::GetListener(int index) {
  if (index < 0 || index >= listeners_.size()) return NULL;

  return listeners_.at(index);
}

const Listener *ListenerManager::GetListener(int index) const {
  if (index < 0 || index >= listeners_.size()) return NULL;

  return listeners_.at(index);
}

long ListenerManager::RegisterListenerMessage(Listener *listener) {
  if (listener == NULL) return SMT_ERR_INVALID_PARAM;

  Messages Messages = listener->GetMessages();

  Message2Ptr::iterator mapIter = message2listeners_.begin();
  Messages::iterator iter = Messages.begin();
  while (iter != Messages.end()) {
    if (message2listeners_.find((*iter)) == message2listeners_.end()) {
      message2listeners_.insert(Message2PtrPair((*iter), (void *)listener));
    }
    ++iter;
  }

  return SMT_ERR_NONE;
}

long ListenerManager::UnRegisterListenerMessage(Listener *listener) {
  if (listener == NULL) return SMT_ERR_INVALID_PARAM;

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

  return SMT_ERR_NONE;
}
}  // namespace core