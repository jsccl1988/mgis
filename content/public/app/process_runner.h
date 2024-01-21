// Copyright (c) 2015 The Baidu Authors.
// All rights reserved.

#ifndef CONTENT_PUBLIC_APP_PROCESS_RUNNER_H
#define CONTENT_PUBLIC_APP_PROCESS_RUNNER_H

#include "base/memory/ref_counted.h"
#include "base/memory/weak_ptr.h"

namespace content {
class ProcessRunner : public base::RefCountedThreadSafe<ProcessRunner> {
 public:
  virtual bool Init() = 0;
  virtual bool Run() = 0;
  virtual bool Stop() = 0;

 protected:
  friend class base::RefCountedThreadSafe<ProcessRunner>;
  virtual ~ProcessRunner() {}
};
}  // namespace content

#endif  // CONTENT_PUBLIC_APP_PROCESS_RUNNER_H
