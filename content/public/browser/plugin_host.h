// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#ifndef CONTENT_PUBLIC_BROWSER_PLUGIN_HOST_H_
#define CONTENT_PUBLIC_BROWSER_PLUGIN_HOST_H_

#include <string>
#include <vector>

#include "plugin/api/mview_plugin_api.h"

namespace content {

class ToolSession;

// In-process plugin surface. Plugins contribute ids and run algorithms here.
// This type has no pipe client, no MapLibre map, and no UE types. It does not
// LoadLibrary and does not include Views.
class PluginHost {
 public:
  // Optional runner bound when a command is contributed through the loader.
  typedef int (*CommandRunFn)(void* user_data, char* error, int error_cap);

  PluginHost();
  ~PluginHost();

  // Unowned. When set, interaction factories are installed into every
  // ToolSession workspace and can be activated there.
  void set_tool_session(ToolSession* session);
  ToolSession* tool_session() const { return tool_session_; }

  // Records a command id. |run| may be NULL. Returns false when |command_id|
  // is empty or already contributed.
  bool ContributeCommand(const std::string& command_id,
                         CommandRunFn run,
                         void* user_data);

  // Records a command id with no runner (C ABI contribute_command).
  bool contribute_command_id(const std::string& command_id);

  // Records an interaction-factory id and installs a Workspace factory when
  // a tool session is attached. Returns false when |factory_id| is empty,
  // |create| is NULL, or the id is already contributed.
  bool ContributeInteractionFactory(const std::string& factory_id,
                                    MviewPluginCreateInteractionFn create,
                                    void* user_data);

  bool contribute_interaction_factory_id(const std::string& factory_id);

  bool ContributeMenuItem(const std::string& menu_id,
                          const std::string& label_utf8,
                          const std::string& command_id);

  bool ContributeDockPanel(const std::string& panel_id,
                           const std::string& title_utf8);

  // Runs a contributed command or records a bare algorithm id. |error| may be
  // NULL. Returns false when |algorithm_id| is empty or a bound runner fails.
  bool run_processing(const std::string& algorithm_id, std::string* error);

  // Activates a contributed interaction factory on |view_id|.
  bool ActivateInteraction(const std::string& factory_id,
                           const std::string& view_id);

  bool has_command_id(const std::string& command_id) const;
  bool has_interaction_factory_id(const std::string& factory_id) const;
  const std::string& last_algorithm_id() const;

  const std::vector<std::string>& command_ids() const { return command_ids_; }
  const std::vector<std::string>& interaction_factory_ids() const {
    return interaction_factory_ids_;
  }

  struct MenuItem {
    std::string menu_id;
    std::string label_utf8;
    std::string command_id;
  };
  struct DockPanel {
    std::string panel_id;
    std::string title_utf8;
  };

  const std::vector<MenuItem>& menu_items() const { return menu_items_; }
  const std::vector<DockPanel>& dock_panels() const { return dock_panels_; }

 private:
  PluginHost(const PluginHost&) = delete;
  PluginHost& operator=(const PluginHost&) = delete;

  struct CommandEntry {
    std::string id;
    CommandRunFn run;
    void* user_data;
  };

  struct FactoryEntry {
    std::string id;
    MviewPluginCreateInteractionFn create;
    void* user_data;
  };

  void InstallFactoryLocked(const FactoryEntry& entry);

  ToolSession* tool_session_;
  std::vector<std::string> command_ids_;
  std::vector<CommandEntry> commands_;
  std::vector<std::string> interaction_factory_ids_;
  std::vector<FactoryEntry> factories_;
  std::vector<MenuItem> menu_items_;
  std::vector<DockPanel> dock_panels_;
  std::string last_algorithm_id_;
};

}  // namespace content

#endif  // CONTENT_PUBLIC_BROWSER_PLUGIN_HOST_H_
