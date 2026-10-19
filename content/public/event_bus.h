// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#ifndef CONTENT_PUBLIC_EVENT_BUS_H_
#define CONTENT_PUBLIC_EVENT_BUS_H_

#include <string>
#include <vector>

#include "content/public/types.h"

namespace content {

// Published after a selection commit has already landed on the document.
struct SelectionChanged {
  SelectionChanged();

  std::string view_id;
  std::vector<FeatureId> feature_ids;
};

// Published after an extent commit has already landed.
// west/south/east/north are WGS84 degrees (EPSG:4979 horizontal).
struct ExtentChanged {
  ExtentChanged();

  std::string view_id;
  double west;
  double south;
  double east;
  double north;
};

// Session-scoped fan-out for facts that already happened.
// Construct one bus per session. This type has no process-wide instance.
// Pointer input does not travel on this bus.
class EventBus {
 public:
  // Observer pointers are not owned. Unregister before the observer dies.
  class Observer {
   public:
    virtual ~Observer();
    virtual void on_selection_changed(const SelectionChanged& event);
    virtual void on_extent_changed(const ExtentChanged& event);

   protected:
    Observer();

   private:
    Observer(const Observer&) = delete;
    Observer& operator=(const Observer&) = delete;
  };

  EventBus();
  ~EventBus();

  void add_observer(Observer* observer);
  void remove_observer(Observer* observer);

  void publish(const SelectionChanged& event);
  void publish(const ExtentChanged& event);

 private:
  EventBus(const EventBus&) = delete;
  EventBus& operator=(const EventBus&) = delete;

  std::vector<Observer*> observers_;
};

}  // namespace content

#endif  // CONTENT_PUBLIC_EVENT_BUS_H_
