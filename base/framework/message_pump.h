// Copyright (c) 2023 The Mogu Authors.
// All rights reserved.

#ifndef BASE_FRAMEWORK_MESSAGE_PUMP_H_
#define BASE_FRAMEWORK_MESSAGE_PUMP_H_

#include "base/base_config.h"
#include "base/base_export.h"

namespace base {
class TimeTicks;
class BASE_EXPORT MessagePump {
 public:
  class BASE_EXPORT Delegate {
   public:
    virtual ~Delegate() {}
    virtual bool DoWork() = 0;
    virtual bool DoDelayedWork(TimeTicks* next_delayed_message_time) = 0;
    virtual bool DoIdleWork() = 0;
  };

  MessagePump();
  virtual ~MessagePump();

  virtual void Run(Delegate* delegate) = 0;
  virtual void Quit() = 0;
  virtual void ScheduleWork() = 0;
  virtual void ScheduleDelayedWork(const TimeTicks& delay_message_time) = 0;
};
}  // namespace base
#endif  // BASE_FRAMEWORK_MESSAGE_PUMP_H_
