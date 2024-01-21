// Copyright (c) 2024 The mgis Authors.
// All rights reserved.

#ifndef GUI_MAIN_RUNNER_H
#define GUI_MAIN_RUNNER_H

#include <assert.h>

#include <memory>

#include "content/public/app/task_runner.h"
#include "content/public/map_box/map_view.h"
#include "gui/base/thread/thread.h"


// (TODO) will be removed
#include "content/control/map_box/map_app.h"

namespace gui {
class MainRunner : public base::RefCountedThreadSafe<MainRunner>,
                   public base::SupportsWeakPtr<MainRunner> {
 public:
  MainRunner();
  ~MainRunner();

 public:
  bool Init();
  bool Run();
  bool Stop();

 private:
  bool CreateThreads();
  bool DestroyThreads();

 protected:
  base::ScopedRefPtr<content::MapView> map_view_;

  CMessageLoop ui_message_loop_;
  base::ScopedPtr<gui::Thread> worker_thread_;
  base::ScopedRefPtr<content::TaskRunner> task_runner_;

 private:
  DISALLOW_COPY_AND_ASSIGN(MainRunner);
};
}  // namespace gui

#endif  // GUI_MAIN_RUNNER_H
