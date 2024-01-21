// Copyright (c) 2015 The Baidu Authors.
// All rights reserved.

#ifndef CONTENT_CONTROL_IMPL_APP_PROCESS_RUNNER_IMPL_H
#define CONTENT_CONTROL_IMPL_APP_PROCESS_RUNNER_IMPL_H

#include "content/control/window/map_app.h"
#include "content/public/app/process_runner.h"

namespace content {
class ProcessRunnerImpl : public ProcessRunner {
 public:
  ProcessRunnerImpl() {}
  virtual bool Init() {
    _Module.AddMessageLoop(&ui_message_loop_);
    return true;
  }
  virtual bool Run() {
    ui_message_loop_.Run();
    return true;
  }
  virtual bool Stop() {
    _Module.RemoveMessageLoop();
    _Module.Term();
    return true;
  }

 protected:
  friend class base::RefCountedThreadSafe<ProcessRunnerImpl>;
  virtual ~ProcessRunnerImpl() {}

 private:
  CMessageLoop ui_message_loop_;
};
}  // namespace content

#endif  // CONTENT_PUBLIC_APP_PROCESS_RUNNER_H
