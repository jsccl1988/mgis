// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#ifndef TOOL_COMMAND_H_
#define TOOL_COMMAND_H_

#include <map>
#include <memory>
#include <string>

namespace tool {

// One shot of work for a single command id. Commands do not see the input
// chain; interactions do.
class CommandHandler {
 public:
  virtual ~CommandHandler() {}
  virtual bool execute() = 0;

 private:
  CommandHandler(const CommandHandler&) = delete;
  CommandHandler& operator=(const CommandHandler&) = delete;

 protected:
  CommandHandler() {}
};

// Maps each command id to one handler. An empty or duplicate id is rejected
// so dispatch cannot fan a miss out across every registered handler.
class CommandCatalog {
 public:
  CommandCatalog() {}
  ~CommandCatalog() {}

  bool add(const std::string& id, std::unique_ptr<CommandHandler> handler);
  CommandHandler* find(const std::string& id);
  const CommandHandler* find(const std::string& id) const;
  bool contains(const std::string& id) const;

 private:
  CommandCatalog(const CommandCatalog&) = delete;
  CommandCatalog& operator=(const CommandCatalog&) = delete;

  std::map<std::string, std::unique_ptr<CommandHandler> > handlers_;
};

// Invokes the single handler registered for an id. A missing id returns
// false and does not call any handler.
class CommandDispatcher {
 public:
  explicit CommandDispatcher(CommandCatalog* catalog);
  ~CommandDispatcher() {}

  bool execute(const std::string& id);

 private:
  CommandDispatcher(const CommandDispatcher&) = delete;
  CommandDispatcher& operator=(const CommandDispatcher&) = delete;

  CommandCatalog* catalog_;
};

}  // namespace tool

#endif  // TOOL_COMMAND_H_
