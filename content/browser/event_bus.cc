// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "content/public/event_bus.h"

#include <cstddef>

namespace content {

SelectionChanged::SelectionChanged() {}

ExtentChanged::ExtentChanged()
    : west(0.0), south(0.0), east(0.0), north(0.0) {}

EventBus::Observer::Observer() {}

EventBus::Observer::~Observer() {}

void EventBus::Observer::on_selection_changed(const SelectionChanged& event) {
  (void)event;
}

void EventBus::Observer::on_extent_changed(const ExtentChanged& event) {
  (void)event;
}

EventBus::EventBus() {}

EventBus::~EventBus() {}

void EventBus::add_observer(Observer* observer) {
  if (!observer)
    return;
  for (size_t i = 0; i < observers_.size(); ++i) {
    if (observers_[i] == observer)
      return;
  }
  observers_.push_back(observer);
}

void EventBus::remove_observer(Observer* observer) {
  for (size_t i = 0; i < observers_.size(); ++i) {
    if (observers_[i] == observer) {
      observers_.erase(observers_.begin() + static_cast<std::ptrdiff_t>(i));
      return;
    }
  }
}

void EventBus::publish(const SelectionChanged& event) {
  const std::vector<Observer*> observers = observers_;
  for (size_t i = 0; i < observers.size(); ++i) {
    if (observers[i])
      observers[i]->on_selection_changed(event);
  }
}

void EventBus::publish(const ExtentChanged& event) {
  const std::vector<Observer*> observers = observers_;
  for (size_t i = 0; i < observers.size(); ++i) {
    if (observers[i])
      observers[i]->on_extent_changed(event);
  }
}

}  // namespace content
