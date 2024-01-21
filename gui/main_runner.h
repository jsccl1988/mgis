// Copyright (c) 2024 The MGIS Authors.
// All rights reserved.

#ifndef GUI_MAIN_RUNNER_H
#define GUI_MAIN_RUNNER_H

#include <assert.h>

#include <memory>

#include "content/public/app/task_runner.h"
#include "content/public/map_view.h"
#include "gui/base/thread/thread.h"

namespace gui {
class MainRunner : public base::RefCountedThreadSafe<MainRunner>,
                   public base::SupportsWeakPtr<MainRunner> {
 public:
  MainRunner();

 public:
  bool Init();
  bool Run();
  bool Stop();

 private:
  bool CreateThreads();
  bool DestroyThreads();

 protected:
  base::ScopedPtr<gui::Thread> worker_thread_;
  base::ScopedRefPtr<content::TaskRunner> task_runner_;

 private:
  friend class base::RefCountedThreadSafe<MainRunner>;
  ~MainRunner();
};
}  // namespace gui

#endif  // GUI_MAIN_RUNNER_H
