#include "framework/command.h"

#include <algorithm>
#include <vector>

#include "base/logging.h"

namespace framework {
Command::Command(CommandReceiver* receiver, bool auto_delete_receiver)
    : receiver_(receiver), auto_delete_receiver_(auto_delete_receiver) {}

Command::~Command(void) {
  if (auto_delete_receiver_) {
    delete receiver_;
  }
}

void Command::SetReceiver(CommandReceiver* receiver,
                          bool auto_delete_receiver) {
  receiver_ = receiver;
  auto_delete_receiver_ = auto_delete_receiver;
}

bool Command::Execute(void) {
  if (receiver_) {
    return receiver_->Action(false);
  }

  return false;
}

bool Command::UnExecute() {
  if (receiver_) {
    return receiver_->Action(true);
  }

  return false;
}

Command::Command(const Command& rhs) {}
Command& Command::operator=(const Command& rhs) { return *this; }

//////////////////////////////////////////////////////////////////////////
// Command
MacroCommand::MacroCommand() : Command(NULL, false) {}
MacroCommand::~MacroCommand() {
  std::vector<Command*>::iterator iter = commands_.begin();

  while (iter != commands_.end()) {
    if (*iter) {
      delete *iter;
    }

    ++iter;
  }

  commands_.clear();
}

bool MacroCommand::Execute() {
  std::vector<Command*>::iterator iter = commands_.begin();

  while (iter != commands_.end()) {
    if (!(*iter)->Execute()) {
      return false;
    }

    ++iter;
  }

  return true;
}

bool MacroCommand::UnExecute() {
  std::vector<Command*>::iterator iter = commands_.begin();

  while (iter != commands_.end()) {
    if (!(*iter)->UnExecute()) {
      return false;
    }

    ++iter;
  }

  return true;
}

void MacroCommand::AddCommand(Command* command) {
  if (command) commands_.push_back(command);
}

void MacroCommand::DeleteCommand(Command* command) {
  if (command)
    commands_.erase(std::remove(commands_.begin(), commands_.end(), command));
}

//////////////////////////////////////////////////////////////////////////
// CommandManager
CommandManager::CommandManager(void) {}
CommandManager::~CommandManager() {}

bool CommandManager::CallCommand(Command* command) {
  if (command) {
    if (command->Execute()) {
      PushUndoCommand(command);
      DeleteRedoCommands();

      return true;
    } else {
      delete command;
    }
  }

  return false;
}

void CommandManager::ClearAllCommands() {
  DeleteUndoCommands();
  DeleteRedoCommands();
}

void CommandManager::Undo() {
  Command* command = PopUndoCommand();
  if (command) {
    if (command->UnExecute()) {
      PushRedoCommand(command);
    } else {
      delete command;
    }
  }
}

void CommandManager::Redo() {
  Command* command = PopRedoCommand();
  if (command) {
    if (command->Execute()) {
      PushUndoCommand(command);
    } else {
      delete command;
    }
  }
}

bool CommandManager::CanUndo() const { return !undos_.empty(); }
bool CommandManager::CanRedo() const { return !redos_.empty(); }
void CommandManager::PushUndoCommand(Command* command) {
  if (command) {
    undos_.push(command);
  }
}

Command* CommandManager::PopUndoCommand() {
  Command* command = NULL;
  if (!undos_.empty()) {
    command = undos_.top();
    undos_.pop();
  }

  return command;
}

void CommandManager::PushRedoCommand(Command* command) {
  if (command) {
    redos_.push(command);
  }
}

Command* CommandManager::PopRedoCommand() {
  Command* command = NULL;
  if (!redos_.empty()) {
    command = redos_.top();
    redos_.pop();
  }

  return command;
}

void CommandManager::DeleteUndoCommands() {
  while (!undos_.empty()) {
    delete undos_.top();
    undos_.pop();
  }
}

void CommandManager::DeleteRedoCommands() {
  while (!redos_.empty()) {
    delete redos_.top();
    redos_.pop();
  }
}
}  // namespace framework