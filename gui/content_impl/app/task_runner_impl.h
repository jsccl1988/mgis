// Copyright (c) 2015 The Baidu Authors.
// All rights reserved.

#ifndef DEMO_WEB_RENDER_APP_TASK_RUNNER_IMPL_H_
#define DEMO_WEB_RENDER_APP_TASK_RUNNER_IMPL_H_

#include <assert.h>

#include "content/public/app/task_runner.h"
#include "base/platform.h"

namespace gui {
class TaskRunnerImpl : public content::TaskRunner {
 public:
  explicit TaskRunnerImpl();
  virtual ~TaskRunnerImpl();

 public:
  int PostTask(content::Task* task, int64 delay = 0) override;
};
}  // namespace gui

#endif  // DEMO_WEB_RENDER_APP_TASK_RUNNER_IMPL_H_
