// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#ifndef TOOL_WORKSPACE_H_
#define TOOL_WORKSPACE_H_

#include <memory>
#include <string>

#include "tool/command.h"
#include "tool/interaction.h"

namespace tool {

// Test double for the edit session a later patch will own. Workspace calls
// these after an interaction yields. Interactions do not hold this pointer.
class EditSink {
 public:
  virtual ~EditSink() {}
  virtual void commit_selection() = 0;
  virtual void clear_selection() = 0;

 private:
  EditSink(const EditSink&) = delete;
  EditSink& operator=(const EditSink&) = delete;

 protected:
  EditSink() {}
};

// Per-view command catalog, exclusive-tool stack, and input router.
// Installs wheel.zoom and hover.cursor. Those ids are not commands.
class Workspace {
 public:
  explicit Workspace(const std::string& view_id);
  ~Workspace();

  const std::string& view_id() const { return view_id_; }

  void set_edit_sink(EditSink* sink) { edit_sink_ = sink; }
  EditSink* edit_sink() const { return edit_sink_; }

  CommandCatalog& catalog() { return catalog_; }
  const CommandCatalog& catalog() const { return catalog_; }
  InteractionRegistry& registry() { return registry_; }
  InteractionStack& stack() { return stack_; }
  InputRouter& router() { return router_; }

  bool execute(const std::string& command_id);
  bool dispatch(const InputEvent& event);

  // True after selection.point, selection.rect, or selection.clear was
  // accepted. The interaction itself does not commit that request.
  bool selection_requested() const { return selection_requested_; }

  Interaction* always_on(const char* id) const;

 private:
  void RegisterBuiltins();
  void CommitSelectionIfReady();

  std::string view_id_;
  EditSink* edit_sink_;
  bool selection_requested_;
  std::unique_ptr<Interaction> wheel_zoom_;
  std::unique_ptr<Interaction> hover_cursor_;
  CommandCatalog catalog_;
  CommandDispatcher dispatcher_;
  InteractionRegistry registry_;
  InteractionStack stack_;
  InputRouter router_;

  Workspace(const Workspace&) = delete;
  Workspace& operator=(const Workspace&) = delete;
};

}  // namespace tool

#endif  // TOOL_WORKSPACE_H_
