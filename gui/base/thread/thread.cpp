// Copyright (c) 2023 The MGIS Authors.
// All rights reserved.

#include "gui/base/thread/thread.h"

#include <assert.h>

#include <list>
#include <map>
#include <string>

namespace gui {
unsigned int __stdcall ThreadFunc(void* closure) {
  Thread* thread = static_cast<Thread*>(closure);

  thread->ThreadMain();

  return 0;
}

static void ThreadQuitFunc() { MessageLoop::Current()->Quit(); }

Thread::Thread(LPCSTR default_name)
    : stopping_(NULL),
      thread_id_(0),
      thread_(HANDLE(INVALID_HANDLE_VALUE)),
      message_loop_(NULL),
      startup_data_(NULL) {
  if (default_name) {
    strncpy_s(name_, default_name, _TRUNCATE);
  } else {
    name_[0] = '\0';
  }
}

Thread::~Thread(void) { Stop(); }

bool Thread::Start() { return StartWithOptions(Options()); }

bool Thread::StartWithOptions(const Options& options) {
  StartupData startup_data(options);
  startup_data_ = &startup_data;

  if (options.thread_name_) {
    strncpy_s(name_, options.thread_name_, _TRUNCATE);
  }

  uint32_t flags = 0;

  uint32_t threadId = 0;
  thread_ = (HANDLE)_beginthreadex(NULL, options.stack_size_, ThreadFunc, this,
                                   flags, &threadId);

  thread_id_ = threadId;
  if (0 == thread_) {
    DWORD error = GetLastError();
    startup_data_ = NULL;
    return false;
  }

  // Wait for the thread to start and initialize message_loop_
  WaitForSingleObject(startup_data_->started_event_, INFINITE);

  const char* name = name_;
  if (name[0]) {
    DebuggerThreadName::SetThreadName(thread_id_, name);
  }

  return true;
}

bool Thread::Stop() {
  if (!ThreadWasStarted()) return false;

  if (!StopSoon()) return false;

  WaitForSingleObject(thread_, INFINITE);

  CloseHandle(thread_);
  thread_ = HANDLE(INVALID_HANDLE_VALUE);

  // The thread no longer needs to be joined.
  startup_data_ = NULL;

  stopping_ = false;

  return true;
}

bool Thread::StopSoon() {
  if (stopping_ || !message_loop_) return false;

  stopping_ = true;
  message_loop_->PostTask(std::bind(ThreadQuitFunc));

  return true;
}

void Thread::Run(MessageLoop* message_loop) { message_loop->Run(); }

void Thread::ThreadMain() {
  thread_id_ = GetCurrentThreadId();
  message_loop_ = new MessageLoop(thread_id_);

  bool use_com = startup_data_->options_.use_com_;
  DWORD coinitex_flag = startup_data_->options_.coinitex_flag_;
  bool couninitex_hack = startup_data_->options_.couninitex_hack_;

  ::SetEvent(startup_data_->started_event_);

  if (use_com) {
    if (-1 != coinitex_flag) {
      ::CoInitializeEx(NULL, coinitex_flag);
    } else {
      ::OleInitialize(NULL);
    }
  }

  Run(message_loop_);

  if (use_com) {
    if (-1 != coinitex_flag) {
      ::CoUninitialize();
    } else {
      ::OleUninitialize();
    }

    if (couninitex_hack) {
      int i = 0;
      while (++i <= 2) {
        ::CoUninitialize();
      }
    }
  }

  delete message_loop_;
  message_loop_ = NULL;
}
}  // namespace gui
