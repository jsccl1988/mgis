// Copyright (c) 2015 The Baidu Authors.
// All rights reserved.

#include "gui/content_impl/app/task_runner_impl.h"

#include "gui/base/thread/thread_manager.h"

namespace gui {
static void OnRunTask(content::Task* task) {
  base::ScopedRefPtr<content::Task> ref_task = task;
  if (ref_task) {
    ref_task->Run();
    ref_task->Destroy();
  }
}

TaskRunnerImpl::TaskRunnerImpl() {}

TaskRunnerImpl::~TaskRunnerImpl() {}

int TaskRunnerImpl::PostTask(content::Task* task, int64 delay) {
  if (!task) return -1;

  ThreadManager::PostTask(ThreadManager::UI, std::bind(&OnRunTask, task),
                          delay);

  return 0;
}
}  // namespace gui
