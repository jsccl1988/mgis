// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#ifndef TOOL_INTERACTION_H_
#define TOOL_INTERACTION_H_

#include <memory>
#include <map>
#include <string>
#include <vector>

#include "tool/input_event.h"

namespace tool {

// Pointer behavior for one tool. on_input returns true when the event is
// consumed. Draft and result flags describe a gesture; an interaction does
// not write a selection table.
class Interaction {
 public:
  virtual ~Interaction() {}
  virtual const char* id() const = 0;
  virtual bool on_input(const InputEvent& event) = 0;

  virtual bool has_draft() const { return false; }
  virtual bool has_result() const { return false; }
  virtual bool in_input() const { return false; }
  virtual bool result_acknowledged() const { return false; }
  virtual void acknowledge_result() {}

 private:
  Interaction(const Interaction&) = delete;
  Interaction& operator=(const Interaction&) = delete;

 protected:
  Interaction() {}
};

// Builds an Interaction for a registered id. Plugins add factories here.
class InteractionFactory {
 public:
  virtual ~InteractionFactory() {}
  virtual std::unique_ptr<Interaction> Create() const = 0;

 private:
  InteractionFactory(const InteractionFactory&) = delete;
  InteractionFactory& operator=(const InteractionFactory&) = delete;

 protected:
  InteractionFactory() {}
};

// Id-to-factory table for the interactions a view can activate.
class InteractionRegistry {
 public:
  InteractionRegistry() {}
  ~InteractionRegistry() {}

  bool add(const std::string& id, std::unique_ptr<InteractionFactory> factory);
  std::unique_ptr<Interaction> create(const std::string& id) const;

 private:
  InteractionRegistry(const InteractionRegistry&) = delete;
  InteractionRegistry& operator=(const InteractionRegistry&) = delete;

  std::map<std::string, std::unique_ptr<InteractionFactory> > factories_;
};

// Exclusive-tool stack for one view. activate replaces the current tool.
// push keeps the previous tool. An empty stack means no exclusive tool.
// view3d.trackball and view3d.fps activate on view0, view1, and scene.
class InteractionStack {
 public:
  InteractionStack(InteractionRegistry* registry, const std::string& view_id);
  ~InteractionStack() {}

  bool activate(const std::string& id);
  bool push(const std::string& id);
  bool pop();
  Interaction* current() const;

  const std::string& view_id() const { return view_id_; }

 private:
  InteractionStack(const InteractionStack&) = delete;
  InteractionStack& operator=(const InteractionStack&) = delete;

  bool Allows(const std::string& id) const;
  std::unique_ptr<Interaction> CreateIfAllowed(const std::string& id) const;

  InteractionRegistry* registry_;
  std::string view_id_;
  std::vector<std::unique_ptr<Interaction> > stack_;
};

// Delivers one event to always-on interactions in insertion order, then to
// the stack's current tool. The first true result stops the chain.
class InputRouter {
 public:
  InputRouter();
  ~InputRouter() {}

  void add_always_on(Interaction* interaction);
  void set_stack(InteractionStack* stack);
  bool dispatch(const InputEvent& event);

 private:
  InputRouter(const InputRouter&) = delete;
  InputRouter& operator=(const InputRouter&) = delete;

  std::vector<Interaction*> always_on_;
  InteractionStack* stack_;
};

// Always-on wheel consumer. Returns true only for wheel events.
class WheelZoom : public Interaction {
 public:
  WheelZoom() {}
  const char* id() const override { return "wheel.zoom"; }
  bool on_input(const InputEvent& event) override;
};

// Always-on hover. Returns false for move so the exclusive tool still sees it.
class HoverCursor : public Interaction {
 public:
  HoverCursor() {}
  const char* id() const override { return "hover.cursor"; }
  bool on_input(const InputEvent& event) override;
};

}  // namespace tool

#endif  // TOOL_INTERACTION_H_
