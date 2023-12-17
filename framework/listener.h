#ifndef FRAMEWORK_EXPORT_LISTENER_H
#define FRAMEWORK_EXPORT_LISTENER_H

#include <string>

#include "framework/framework.h"
#include "framework/core_export.h"
#include "framework/message.h"

namespace framework {
class FRAMEWORK_EXPORT_EXPORT Listener {
 public:
  Listener(void);
  virtual ~Listener(void);

 public:
  virtual int Register(void);
  virtual int RegisterMessage(void);

  virtual int UnRegister(void);
  virtual int UnRegisterMessage(void);

  virtual int SetActive();

  virtual int Notify(long message, ListenerMessage& param) = 0;

 public:
  const char* GetName() const;
  void SetName(const char* name);

  bool AppendFuncItems(const char* function, long message,
                       long lStyle = FIM_2DVIEW | FIM_3DVIEW);
  function_items GetFuncItems(FuncItemStyle style);

  Messages GetMessages(void) { return messages_; }

 protected:
  bool AppendFuncItems(const char* function, long message,
                       function_items& function_items);
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

typedef std::vector<Listener*> Listeners;
typedef std::map<std::string, Listener> Name2Listener;

class FRAMEWORK_EXPORT_EXPORT ListenerManager {
 public:
  virtual ~ListenerManager(void);

 public:
  static ListenerManager* GetSingletonPtr(void);
  static void DestoryInstance(void);

 public:
  long Notify(Listener* listener, long message, ListenerMessage& param);

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
  Listeners listeners_;
  Listener* current_listener_;
  Message2Ptr message2listeners_;

 private:
  ListenerManager(void);
  static ListenerManager* singleton_;
};
}  // namespace framework
#endif  // FRAMEWORK_EXPORT_LISTENER_H