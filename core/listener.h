#ifndef GIS_CORE_LISTENER_H
#define GIS_CORE_LISTENER_H

#include <string>

#include "core/core.h"
#include "core/core_export.h"

namespace core {
class GIS_CORE_EXPORT Listener {
 public:
  Listener(void);
  virtual ~Listener(void);

 public:
  virtual int Register(void);
  virtual int RegisterMessage(void);

  virtual int UnRegister(void);
  virtual int UnRegisterMessage(void);

  virtual int SetActive();

  virtual int Notify(long lMessage, ListenerMessage& param) = 0;

 public:
  const char* GetName() const;
  void SetName(const char* szName);

  bool AppendFuncItems(const char* szFunc, long lFuncMessage,
                       long lStyle = FIM_2DVIEW | FIM_3DVIEW);
  FunctionItems GetFuncItems(FuncItemStyle style);

  Messages GetMessages(void) { return m_vMessages; }

 protected:
  bool AppendFuncItems(const char* szFunc, long lFuncMessage,
                       FunctionItems& FunctionItems);
  bool AppendMessage(long lFuncMessage);

 protected:
  char m_szListenerName[SMT_GROUP_NAME_LENGTH];

  View2DFunctionItems m_v2DViewFuncItems;
  View3DFunctionItems m_v3DViewFuncItems;
  View3DExFunctionItems m_v3DExViewFuncItems;
  MapDocCatalogFunctionItems m_vMDCatalogFuncItems;
  ToolBar2DFunctionItems m_v2DToolBarFuncItems;
  ToolBar3DFunctionItems m_v3DToolBarFuncItems;
  Menu2DFunctionItems m_v2DMMenuFuncItems;
  Menu3DFunctionItems m_v3DMMenuFuncItems;
  AMBoxFunctionItems m_vAMBoxFuncItems;
  AMTreeFunctionItems m_vAMTreeFuncItems;
  Messages m_vMessages;
};

typedef std::vector<Listener*> vListenerPtrs;
typedef std::map<std::string, Listener> ListenerMap;

class GIS_CORE_EXPORT ListenerManager {
 public:
  virtual ~ListenerManager(void);

 public:
  static ListenerManager* GetSingletonPtr(void);
  static void DestoryInstance(void);

 public:
  long Notify(Listener* listener, long lMessage, ListenerMessage& param);

  long RegisterListener(Listener* listener);
  long RemoveListener(Listener* listener);
  long RemoveAllListener(void);

  void SetActiveListener(Listener* listener) { current_listener_ = listener; }
  Listener* GetActiveListener(void) { return current_listener_; }
  const Listener* GetActiveListener(void) const { return current_listener_; }

  int GetListenerCount(void) const { return listeners_.size(); }
  Listener* GetListener(int index);
  const Listener* GetListener(int index) const;

  long RegisterListenerMessage(Listener* listener);
  long UnRegisterListenerMessage(Listener* listener);

 protected:
  vListenerPtrs listeners_;
  Listener* current_listener_;
  Message2Ptr message2listeners_;

 private:
  ListenerManager(void);
  static ListenerManager* singleton_;
};
}  // namespace core
#endif  // GIS_CORE_LISTENER_H