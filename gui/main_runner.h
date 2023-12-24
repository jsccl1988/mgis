// Copyright (c) 2023 The MGIS Authors.
// All rights reserved.


#ifndef GUI_MAIN_RUNNER_H
#define GUI_MAIN_RUNNER_H

#include <assert.h>

#include <memory>

#include "content/public/app/task_runner.h"
#include "gui/base/thread/thread.h"

namespace gui {
class MessageLoop;
class MainRunner : public base::RefCountedThreadSafe<MainRunner> {
 public:
  MainRunner();

 public:
  bool Init();
  bool Run();
  bool Stop();

 private:
  ~MainRunner();
  friend class base::RefCountedThreadSafe<MainRunner>;

  bool CreateThreads();
  bool DestroyThreads();

 protected:
  base::ScopedPtr<MessageLoop> ui_message_loop_;
  base::ScopedPtr<gui::Thread> worker_thread_;

  base::ScopedRefPtr<content::TaskRunner> task_runner_;

 private:
  DISALLOW_COPY_AND_ASSIGN(MainRunner);
};
}  // namespace gui

#endif  // GUI_MAIN_RUNNER_H
