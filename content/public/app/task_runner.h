// Copyright (c) 2023 The MGIS Authors.
// All rights reserved.

#ifndef CONTENT_PUBLIC_APP_TASK_RUNNER_H
#define CONTENT_PUBLIC_APP_TASK_RUNNER_H

#include "base/basictypes.h"
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"
#include "base/memory/weak_ptr.h"
#include "base/platform.h"

namespace content {
class Task : public base::RefCountedThreadSafe<Task> {
 public:
  Task() { name_ = NULL; }

  virtual void Run() = 0;
  virtual void Destroy() = 0;

 protected:
  friend class base::RefCountedThreadSafe<Task>;
  virtual ~Task() {}

  const wchar_t* name_;
};

class TaskRunner : public base::RefCountedThreadSafe<TaskRunner>,
                   public base::SupportsWeakPtr<TaskRunner> {
 public:
  virtual int PostTask(Task* task, int64_t delay = 0) = 0;

 protected:
  friend class base::RefCountedThreadSafe<TaskRunner>;
  virtual ~TaskRunner() {}
};
}  // namespace content

#endif  // CONTENT_PUBLIC_APP_TASK_RUNNER_H
