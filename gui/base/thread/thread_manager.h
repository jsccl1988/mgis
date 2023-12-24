// Copyright (c) 2023 The MGIS Authors.
// All rights reserved.

#ifndef GUI_BASE_THREAD_THREAD_MANAGER_H_
#define GUI_BASE_THREAD_THREAD_MANAGER_H_

#include <map>

#include "gui/base/thread/thread.h"

namespace gui {
class MainRunner;
class ThreadManager {
 public:
  enum ID {
    UI,
    WORKER,
    ID_COUNT,
  };

  ThreadManager();
  virtual ~ThreadManager();

 private:
  friend class MainRunner;
  friend class MessageLoop;
  static bool SetMessageLoop(ID identifier, MessageLoop* message_loop);

  static void RemoveMessageLoop(ID identifier);
  static void RemoveMessageLoop(ThreadId thread_id);
  static void RemoveAllMessageLoop();

 public:
  static bool CurrentlyOn(ID identifier);

  static bool GetMessageLoop(ID identifier, MessageLoop** message_loop);

  static bool PostTask(ID identifier, const std::function<void(void)>& handler,
                       int64 delay = 0);

 private:
  static base::Lock callers_lock_;
  static std::map<ID, MessageLoop*> id_2_message_loop_;
};
}  // namespace gui
#endif  // GUI_BASE_THREAD_THREAD_MANAGER_H_
