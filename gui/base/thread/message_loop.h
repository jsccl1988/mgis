// Copyright (c) 2023 The MGIS Authors.
// All rights reserved.

#ifndef GUI_BASE_THREAD_MESSAGE_LOOP_H_
#define GUI_BASE_THREAD_MESSAGE_LOOP_H_

#include <assert.h>

#include <functional>

#include "base/platform.h"

namespace gui {
class MessageLoop {
 public:
  MessageLoop(ThreadId thread_id);
  ~MessageLoop();

 public:
  void PostTask(const std::function<void(void)>& handler,
                unsigned int delay = 0);

  void Quit();

  void Run();

 public:
  static MessageLoop* Current();
  ThreadId GetThreadID() const { return thread_id_; }

 private:
  void InitMessageWnd();
  HWND message_wnd_;

  static LRESULT CALLBACK WndProcThunk(HWND hwnd, UINT message, WPARAM wparam,
                                       LPARAM lparam);

  ThreadId thread_id_;
};
}  // namespace gui

#endif  // GUI_BASE_THREAD_MESSAGE_LOOP_H_
