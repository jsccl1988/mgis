#ifndef CONTENT_COMMON_COMMAND_H
#define CONTENT_COMMON_COMMAND_H

#include <stack>
#include <vector>

#include "content/content.h"
#include "content/content_export.h"

namespace content {
class CONTENT_EXPORT CommandReceiver {
 public:
  CommandReceiver() {}
  virtual ~CommandReceiver() {}

 public:
  virtual bool Action(bool is_undo) = 0;
};

class CONTENT_EXPORT Command {
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

class CONTENT_EXPORT MacroCommand : public Command {
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

class CONTENT_EXPORT CommandManager {
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
}  // namespace content

#endif  // CONTENT_COMMON_COMMAND_H
