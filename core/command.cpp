#include "core/command.h"

#include <algorithm>

#include "base/logging.h"

namespace core {
Command::Command(CommandReceiver* receiver, bool auto_delete_receiver)
    : receiver_(receiver), auto_delete_receiver_(auto_delete_receiver) {}

Command::~Command(void) {
  if (auto_delete_receiver_) {
    delete receiver_
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

bool Command::Unexecute() {
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
  vector<Command*>::iterator iter = commands_.begin();

  while (iter != commands_.end()) {
    if (*iter) {
      delete *iter;
    }

    ++iter;
  }

  commands_.clear();
}

bool MacroCommand::Execute() {
  vector<Command*>::iterator iter = commands_.begin();

  while (iter != commands_.end()) {
    if (!(*iter)->Execute()) {
      return false;
    }

    ++iter;
  }

  return true;
}

bool MacroCommand::Unexecute() {
  vector<Command*>::iterator iter = commands_.begin();

  while (iter != commands_.end()) {
    if (!(*iter)->Unexecute()) {
      return false;
    }

    ++iter;
  }

  return true;
}

void MacroCommand::AddCommand(Command* pCommand) {
  if (pCommand) commands_.push_back(pCommand);
}

void MacroCommand::DeleteCommand(Command* pCommand) {
  if (pCommand)
    commands_.erase(std::remove(commands_.begin(), commands_.end(), pCommand));
}

//////////////////////////////////////////////////////////////////////////
// CommandManager
CommandManager::CommandManager(void) {}
CommandManager::~CommandManager() {}

bool CommandManager::CallCommand(Command* pCommand) {
  if (pCommand) {
    if (pCommand->Execute()) {
      PushUndoCommand(pCommand);
      DeleteRedoCommands();

      return true;
    } else {
      delete pCommand;
    }
  }

  return false;
}

void CommandManager::ClearAllCommands() {
  DeleteUndoCommands();
  DeleteRedoCommands();
}

void CommandManager::Undo() {
  Command* pCommand = PopUndoCommand();
  if (pCommand) {
    if (pCommand->Unexecute()) {
      PushRedoCommand(pCommand);
    } else {
      delete pCommand;
    }
  }
}

void CommandManager::Redo() {
  Command* pCommand = PopRedoCommand();
  if (pCommand) {
    if (pCommand->Execute()) {
      PushUndoCommand(pCommand);
    } else {
      delete pCommand;
    }
  }
}

bool CommandManager::CanUndo() const { return !undos_.empty(); }
bool CommandManager::CanRedo() const { return !redos_.empty(); }
void CommandManager::PushUndoCommand(Command* pCommand) {
  if (pCommand) {
    undos_.push(pCommand);
  }
}

Command* CommandManager::PopUndoCommand() {
  Command* pCommand = NULL;
  if (!undos_.empty()) {
    pCommand = undos_.top();
    undos_.pop();
  }

  return pCommand;
}

void CommandManager::PushRedoCommand(Command* pCommand) {
  if (pCommand) {
    redos_.push(pCommand);
  }
}

Command* CommandManager::PopRedoCommand() {
  Command* pCommand = NULL;
  if (!redos_.empty()) {
    pCommand = redos_.top();
    redos_.pop();
  }

  return pCommand;
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
}  // namespace core