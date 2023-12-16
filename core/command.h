#ifndef GIS_CORE_COMMAND_H
#define GIS_CORE_COMMAND_H

#include <stack>

#include "core/core.h"
#include "core/core_export.h"

namespace core {
class GIS_CORE_EXPORT CommandReceiver {
 public:
  CommandReceiver() {}
  virtual ~CommandReceiver() {}

 public:
  virtual bool Action(bool bUndo) = 0;
};

class GIS_CORE_EXPORT Command {
 public:
  Command(CommandReceiver* receiver, bool auto_delete_receiver = true);
  virtual ~Command(void);

 public:
  void SetReceiver(CommandReceiver* receiver, bool auto_delete_receiver = true);

 public:
  virtual bool Execute(void);
  virtual bool Unexecute();

 protected:
  Command(const Command& rhs);
  Command& operator=(const Command& rhs);

 protected:
  CommandReceiver* receiver_;
  bool auto_delete_receiver_;
};

class GIS_CORE_EXPORT MacroCommand : public Command {
 public:
  MacroCommand();
  ~MacroCommand();

  virtual bool Execute();
  virtual bool Unexecute();

  void AddCommand(Command* pCommand);
  void DeleteCommand(Command* pCommand);

 private:
  MacroCommand(const MacroCommand& rhs);
  MacroCommand& operator=(const MacroCommand& rhs);

 private:
  std::vector<Command*> commands_;
};

class GIS_CORE_EXPORT CommandManager {
 public:
  CommandManager(void);
  virtual ~CommandManager();

 public:
  virtual bool CallCommand(Command* pCommand);
  virtual void ClearAllCommands();

  virtual void Undo();
  virtual void Redo();

  virtual bool CanUndo() const;
  virtual bool CanRedo() const;

 public:
  void PushUndoCommand(Command* pCommand);
  Command* PopUndoCommand();
  void PushRedoCommand(Command* pCommand);
  Command* PopRedoCommand();
  void DeleteUndoCommands();
  void DeleteRedoCommands();

 private:
  std::stack<Command*> undos_;
  std::stack<Command*> redos_;
};
}  // namespace core

#endif  // GIS_CORE_COMMAND_H
