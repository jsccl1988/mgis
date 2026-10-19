// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "tool/command.h"

namespace tool {

bool CommandCatalog::add(const std::string& id,
                         std::unique_ptr<CommandHandler> handler) {
  if (id.empty() || !handler)
    return false;
  if (handlers_.find(id) != handlers_.end())
    return false;
  handlers_[id] = std::move(handler);
  return true;
}

CommandHandler* CommandCatalog::find(const std::string& id) {
  std::map<std::string, std::unique_ptr<CommandHandler> >::iterator it =
      handlers_.find(id);
  if (it == handlers_.end())
    return nullptr;
  return it->second.get();
}

const CommandHandler* CommandCatalog::find(const std::string& id) const {
  std::map<std::string, std::unique_ptr<CommandHandler> >::const_iterator it =
      handlers_.find(id);
  if (it == handlers_.end())
    return nullptr;
  return it->second.get();
}

bool CommandCatalog::contains(const std::string& id) const {
  return handlers_.find(id) != handlers_.end();
}

CommandDispatcher::CommandDispatcher(CommandCatalog* catalog)
    : catalog_(catalog) {}

bool CommandDispatcher::execute(const std::string& id) {
  if (!catalog_ || id.empty())
    return false;
  CommandHandler* handler = catalog_->find(id);
  if (!handler)
    return false;
  return handler->execute();
}

}  // namespace tool
