#ifndef FRAMEWORK_EXPORT_COMMAND_H
#define FRAMEWORK_EXPORT_COMMAND_H

#include <stack>
#include <vector>

#include "framework/framework.h"
#include "framework/framework_export.h"

namespace framework {
class FRAMEWORK_EXPORT CommandReceiver {
 public:
  CommandReceiver() {}
  virtual ~CommandReceiver() {}

 public:
  virtual bool Action(bool is_undo) = 0;
};

class FRAMEWORK_EXPORT Command {
 public:
  Command(CommandReceiver* receiver, bool auto_delete_receiver = true);
  virtual ~Command(void);

 public:
  void SetReceiver(CommandReceiver* receiver, bool auto_delete_receiver = true);

 public:
  virtual bool Execute(void);
  virtual bool UnExecute();

 protected:
  Command(const Command& rhs);
  Command& operator=(const Command& rhs);

 protected:
  CommandReceiver* receiver_;
  bool auto_delete_receiver_;
};

class FRAMEWORK_EXPORT MacroCommand : public Command {
 public:
  MacroCommand();
  ~MacroCommand();

  virtual bool Execute();
  virtual bool UnExecute();

  void AddCommand(Command* command);
  void DeleteCommand(Command* command);

 private:
  MacroCommand(const MacroCommand& rhs);
  MacroCommand& operator=(const MacroCommand& rhs);

 private:
  std::vector<Command*> commands_;
};

class FRAMEWORK_EXPORT CommandManager {
 public:
  CommandManager(void);
  virtual ~CommandManager();

 public:
  virtual bool CallCommand(Command* command);
  virtual void ClearAllCommands();

  virtual void Undo();
  virtual void Redo();

  virtual bool CanUndo() const;
  virtual bool CanRedo() const;

 public:
  void PushUndoCommand(Command* command);
  Command* PopUndoCommand();
  void PushRedoCommand(Command* command);
  Command* PopRedoCommand();
  void DeleteUndoCommands();
  void DeleteRedoCommands();

 private:
  std::stack<Command*> undos_;
  std::stack<Command*> redos_;
};
}  // namespace framework

#endif  // FRAMEWORK_EXPORT_COMMAND_H
