// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "tool/workspace.h"

#include <cstring>

namespace tool {
namespace {

bool IsSelectionTool(const char* id) {
  return std::strcmp(id, "selection.point") == 0 ||
         std::strcmp(id, "selection.rect") == 0;
}

bool IsSelectionId(const char* id) {
  return id && IsSelectionTool(id);
}

// Sets in_input for the duration of on_input, including early returns.
class InputScope {
 public:
  explicit InputScope(bool* flag) : flag_(flag) { *flag_ = true; }
  ~InputScope() { *flag_ = false; }

 private:
  bool* flag_;
  InputScope(const InputScope&) = delete;
  InputScope& operator=(const InputScope&) = delete;
};

// Selection and edit gestures. Records draft/result only. No EditSink.
class DraftingInteraction : public Interaction {
 public:
  explicit DraftingInteraction(const char* id)
      : id_(id),
        in_input_(false),
        has_draft_(false),
        has_result_(false),
        acknowledged_(false) {}

  const char* id() const override { return id_; }
  bool has_draft() const override { return has_draft_; }
  bool has_result() const override { return has_result_; }
  bool in_input() const override { return in_input_; }
  bool result_acknowledged() const override { return acknowledged_; }
  void acknowledge_result() override { acknowledged_ = true; }

  bool on_input(const InputEvent& event) override {
    InputScope scope(&in_input_);
    if (event.kind != kMouse)
      return false;
    if (event.action == kMousePress) {
      has_draft_ = true;
      has_result_ = false;
      acknowledged_ = false;
      return true;
    }
    if (event.action == kMouseMove)
      return has_draft_;
    if (event.action == kMouseRelease && has_draft_) {
      has_result_ = true;
      return true;
    }
    return false;
  }

 private:
  const char* id_;
  bool in_input_;
  bool has_draft_;
  bool has_result_;
  bool acknowledged_;
};

// Navigation tool. Consumes the pointer and leaves the wheel to wheel.zoom.
class PointerInteraction : public Interaction {
 public:
  explicit PointerInteraction(const char* id) : id_(id) {}
  const char* id() const override { return id_; }
  bool on_input(const InputEvent& event) override {
    return event.kind == kMouse;
  }

 private:
  const char* id_;
};

class IdFactory : public InteractionFactory {
 public:
  enum Kind { kDraft, kPointer };

  IdFactory(Kind kind, const char* id) : kind_(kind), id_(id) {}

  std::unique_ptr<Interaction> Create() const override {
    if (kind_ == kDraft)
      return std::unique_ptr<Interaction>(new DraftingInteraction(id_));
    return std::unique_ptr<Interaction>(new PointerInteraction(id_));
  }

 private:
  Kind kind_;
  const char* id_;
};

class ActivateToolCommand : public CommandHandler {
 public:
  ActivateToolCommand(InteractionStack* stack,
                      const char* id,
                      bool* selection_flag)
      : stack_(stack), id_(id), selection_flag_(selection_flag) {}

  bool execute() override {
    if (!stack_->activate(id_))
      return false;
    if (selection_flag_ && IsSelectionTool(id_))
      *selection_flag_ = true;
    return true;
  }

 private:
  InteractionStack* stack_;
  const char* id_;
  bool* selection_flag_;
};

class ClearSelectionCommand : public CommandHandler {
 public:
  ClearSelectionCommand(bool* selection_flag, EditSink** sink)
      : selection_flag_(selection_flag), sink_(sink) {}

  bool execute() override {
    if (selection_flag_)
      *selection_flag_ = true;
    if (sink_ && *sink_)
      (*sink_)->clear_selection();
    return true;
  }

 private:
  bool* selection_flag_;
  EditSink** sink_;
};

class SucceedCommand : public CommandHandler {
 public:
  bool execute() override { return true; }
};

bool AddFactory(InteractionRegistry* registry,
                const char* id,
                IdFactory::Kind kind) {
  return registry->add(id, std::unique_ptr<InteractionFactory>(
                               new IdFactory(kind, id)));
}

bool AddActivate(CommandCatalog* catalog,
                 InteractionStack* stack,
                 const char* id,
                 bool* selection_flag) {
  return catalog->add(id, std::unique_ptr<CommandHandler>(new ActivateToolCommand(
                              stack, id, selection_flag)));
}

}  // namespace

Workspace::Workspace(const std::string& view_id)
    : view_id_(view_id),
      edit_sink_(nullptr),
      selection_requested_(false),
      dispatcher_(&catalog_),
      stack_(&registry_, view_id) {
  wheel_zoom_.reset(new WheelZoom());
  hover_cursor_.reset(new HoverCursor());
  router_.add_always_on(wheel_zoom_.get());
  router_.add_always_on(hover_cursor_.get());
  router_.set_stack(&stack_);
  RegisterBuiltins();
}

Workspace::~Workspace() {}

bool Workspace::execute(const std::string& command_id) {
  return dispatcher_.execute(command_id);
}

bool Workspace::dispatch(const InputEvent& event) {
  const bool consumed = router_.dispatch(event);
  CommitSelectionIfReady();
  return consumed;
}

Interaction* Workspace::always_on(const char* id) const {
  if (!id)
    return nullptr;
  if (std::strcmp(id, "wheel.zoom") == 0)
    return wheel_zoom_.get();
  if (std::strcmp(id, "hover.cursor") == 0)
    return hover_cursor_.get();
  return nullptr;
}

void Workspace::CommitSelectionIfReady() {
  Interaction* current = stack_.current();
  if (!current || !current->has_result() || current->result_acknowledged())
    return;
  if (!IsSelectionId(current->id()))
    return;
  current->acknowledge_result();
  if (edit_sink_)
    edit_sink_->commit_selection();
}

void Workspace::RegisterBuiltins() {
  const char* k_draft[] = {
      "selection.point",
      "selection.rect",
      "edit.append.point",
      "edit.append.linestring",
      "edit.append.polygon",
  };
  const char* k_pointer[] = {
      "view.pan",
      "view.zoom_in",
      "view.zoom_out",
      "view3d.trackball",
      "view3d.fps",
  };

  for (size_t i = 0; i < sizeof(k_draft) / sizeof(k_draft[0]); ++i)
    AddFactory(&registry_, k_draft[i], IdFactory::kDraft);
  for (size_t i = 0; i < sizeof(k_pointer) / sizeof(k_pointer[0]); ++i)
    AddFactory(&registry_, k_pointer[i], IdFactory::kPointer);

  for (size_t i = 0; i < sizeof(k_draft) / sizeof(k_draft[0]); ++i) {
    bool* flag = IsSelectionTool(k_draft[i]) ? &selection_requested_ : nullptr;
    AddActivate(&catalog_, &stack_, k_draft[i], flag);
  }
  for (size_t i = 0; i < sizeof(k_pointer) / sizeof(k_pointer[0]); ++i)
    AddActivate(&catalog_, &stack_, k_pointer[i], nullptr);

  catalog_.add("view.full",
               std::unique_ptr<CommandHandler>(new SucceedCommand()));
  catalog_.add("view.refresh",
               std::unique_ptr<CommandHandler>(new SucceedCommand()));
  catalog_.add("selection.clear",
               std::unique_ptr<CommandHandler>(
                   new ClearSelectionCommand(&selection_requested_, &edit_sink_)));
}

}  // namespace tool
