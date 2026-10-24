// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "content/public/browser/plugin_host.h"

#include <stdlib.h>

#include <memory>

#include "content/browser/tooling/tool_session.h"
#include "content/public/browser/types.h"
#include "tool/interaction.h"
#include "tool/workspace.h"

namespace content {
namespace {

bool ContainsId(const std::vector<std::string>& ids, const std::string& id) {
  for (size_t i = 0; i < ids.size(); ++i) {
    if (ids[i] == id)
      return true;
  }
  return false;
}

void SetError(std::string* error, const std::string& message) {
  if (error)
    *error = message;
}

void ClearError(std::string* error) {
  if (error)
    error->clear();
}

MviewPluginInputEvent ToPluginEvent(const tool::InputEvent& event) {
  MviewPluginInputEvent out;
  out.kind = 0;
  out.action = 0;
  out.x_px = event.x_px;
  out.y_px = event.y_px;
  out.wheel_dy = event.wheel_dy;
  out.modifiers = event.modifiers;
  if (event.kind == tool::kWheel) {
    out.kind = 1;
  } else if (event.kind == tool::kKey) {
    out.kind = 2;
  } else {
    out.kind = 0;
    if (event.action == tool::kMouseMove)
      out.action = 1;
    else if (event.action == tool::kMousePress)
      out.action = 2;
    else if (event.action == tool::kMouseRelease)
      out.action = 3;
  }
  return out;
}

// Adapts a C ABI interaction into tool::Interaction for Workspace stacks.
class BridgedInteraction : public tool::Interaction {
 public:
  explicit BridgedInteraction(MviewPluginInteraction* plugin) : plugin_(plugin) {}

  virtual ~BridgedInteraction() {
    if (!plugin_)
      return;
    if (plugin_->destroy)
      plugin_->destroy(plugin_->user);
    free(plugin_);
    plugin_ = NULL;
  }

  virtual const char* id() const {
    if (!plugin_ || !plugin_->id)
      return "";
    const char* value = plugin_->id(plugin_->user);
    return value ? value : "";
  }

  virtual bool on_input(const tool::InputEvent& event) {
    if (!plugin_ || !plugin_->on_input)
      return false;
    MviewPluginInputEvent pe = ToPluginEvent(event);
    return plugin_->on_input(plugin_->user, &pe) != 0;
  }

 private:
  MviewPluginInteraction* plugin_;
};

class BridgedFactory : public tool::InteractionFactory {
 public:
  BridgedFactory(MviewPluginCreateInteractionFn create, void* user_data)
      : create_(create), user_data_(user_data) {}

  virtual std::unique_ptr<tool::Interaction> Create() const {
    if (!create_)
      return std::unique_ptr<tool::Interaction>();
    MviewPluginInteraction* plugin = create_(user_data_);
    if (!plugin)
      return std::unique_ptr<tool::Interaction>();
    return std::unique_ptr<tool::Interaction>(new BridgedInteraction(plugin));
  }

 private:
  MviewPluginCreateInteractionFn create_;
  void* user_data_;
};

}  // namespace

PluginHost::PluginHost() : tool_session_(NULL) {}

PluginHost::~PluginHost() {}

void PluginHost::set_tool_session(ToolSession* session) {
  tool_session_ = session;
  for (size_t i = 0; i < factories_.size(); ++i)
    InstallFactoryLocked(factories_[i]);
}

bool PluginHost::ContributeCommand(const std::string& command_id,
                                   CommandRunFn run,
                                   void* user_data) {
  if (command_id.empty())
    return false;
  for (size_t i = 0; i < commands_.size(); ++i) {
    if (commands_[i].id != command_id)
      continue;
    if (commands_[i].run && run)
      return false;
    if (run) {
      commands_[i].run = run;
      commands_[i].user_data = user_data;
    }
    return true;
  }
  if (ContainsId(command_ids_, command_id)) {
    CommandEntry entry;
    entry.id = command_id;
    entry.run = run;
    entry.user_data = user_data;
    commands_.push_back(entry);
    return true;
  }
  command_ids_.push_back(command_id);
  CommandEntry entry;
  entry.id = command_id;
  entry.run = run;
  entry.user_data = user_data;
  commands_.push_back(entry);
  return true;
}

bool PluginHost::contribute_command_id(const std::string& command_id) {
  return ContributeCommand(command_id, NULL, NULL);
}

bool PluginHost::ContributeInteractionFactory(
    const std::string& factory_id,
    MviewPluginCreateInteractionFn create,
    void* user_data) {
  if (factory_id.empty() || !create ||
      ContainsId(interaction_factory_ids_, factory_id))
    return false;
  interaction_factory_ids_.push_back(factory_id);
  FactoryEntry entry;
  entry.id = factory_id;
  entry.create = create;
  entry.user_data = user_data;
  factories_.push_back(entry);
  InstallFactoryLocked(entry);
  return true;
}

bool PluginHost::contribute_interaction_factory_id(
    const std::string& factory_id) {
  if (factory_id.empty() ||
      ContainsId(interaction_factory_ids_, factory_id))
    return false;
  interaction_factory_ids_.push_back(factory_id);
  return true;
}

bool PluginHost::ContributeMenuItem(const std::string& menu_id,
                                    const std::string& label_utf8,
                                    const std::string& command_id) {
  if (menu_id.empty() || label_utf8.empty() || command_id.empty())
    return false;
  for (size_t i = 0; i < menu_items_.size(); ++i) {
    if (menu_items_[i].menu_id == menu_id)
      return false;
  }
  MenuItem item;
  item.menu_id = menu_id;
  item.label_utf8 = label_utf8;
  item.command_id = command_id;
  menu_items_.push_back(item);
  return true;
}

bool PluginHost::ContributeDockPanel(const std::string& panel_id,
                                     const std::string& title_utf8) {
  if (panel_id.empty() || title_utf8.empty())
    return false;
  for (size_t i = 0; i < dock_panels_.size(); ++i) {
    if (dock_panels_[i].panel_id == panel_id)
      return false;
  }
  DockPanel panel;
  panel.panel_id = panel_id;
  panel.title_utf8 = title_utf8;
  dock_panels_.push_back(panel);
  return true;
}

bool PluginHost::run_processing(const std::string& algorithm_id,
                                std::string* error) {
  if (algorithm_id.empty()) {
    SetError(error, "algorithm id is empty");
    return false;
  }
  for (size_t i = 0; i < commands_.size(); ++i) {
    if (commands_[i].id != algorithm_id || !commands_[i].run)
      continue;
    char buf[512];
    buf[0] = '\0';
    const int rc = commands_[i].run(commands_[i].user_data, buf, 512);
    if (rc != 0) {
      SetError(error, buf[0] ? std::string(buf) : "command runner failed");
      return false;
    }
    last_algorithm_id_ = algorithm_id;
    ClearError(error);
    return true;
  }
  last_algorithm_id_ = algorithm_id;
  ClearError(error);
  return true;
}

bool PluginHost::ActivateInteraction(const std::string& factory_id,
                                     const std::string& view_id) {
  if (!tool_session_ || factory_id.empty() || view_id.empty())
    return false;
  if (!has_interaction_factory_id(factory_id))
    return false;
  // ToolSession::activate routes known navigation tools as commands; plugin
  // factories must hit InteractionStack directly.
  tool::Workspace* workspace = tool_session_->WorkspaceFor(view_id);
  if (!workspace)
    return false;
  return workspace->stack().activate(factory_id);
}

bool PluginHost::has_command_id(const std::string& command_id) const {
  return ContainsId(command_ids_, command_id);
}

bool PluginHost::has_interaction_factory_id(
    const std::string& factory_id) const {
  return ContainsId(interaction_factory_ids_, factory_id);
}

const std::string& PluginHost::last_algorithm_id() const {
  return last_algorithm_id_;
}

void PluginHost::InstallFactoryLocked(const FactoryEntry& entry) {
  if (!tool_session_ || !entry.create)
    return;
  const char* kViews[] = {kViewIdMap, kViewIdView0, kViewIdView1, kViewIdScene};
  for (size_t i = 0; i < sizeof(kViews) / sizeof(kViews[0]); ++i) {
    tool::Workspace* workspace = tool_session_->WorkspaceFor(kViews[i]);
    if (!workspace)
      continue;
    workspace->registry().add(
        entry.id, std::unique_ptr<tool::InteractionFactory>(
                      new BridgedFactory(entry.create, entry.user_data)));
  }
}

}  // namespace content
