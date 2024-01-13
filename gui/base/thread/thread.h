// Copyright (c) 2023 The MGIS Authors.
// All rights reserved.

#ifndef GUI_BASE_THREAD_THREAD_H_
#define GUI_BASE_THREAD_THREAD_H_

#include <math.h>

#include <list>
#include <map>
#include <queue>
#include <string>

#include "base/basictypes.h"
#include "base/platform.h"
#include "base/synchronization/lock.h"
#include "gui/base/thread/message_loop.h"
#include "gui/base/thread/thread_local_pointer.h"
#include "gui/base/thread/thread_name.h"

namespace gui {
class Thread {
 public:
  struct Options {
   public:
    uint32_t stack_size_;
    bool use_com_;
    uint32_t coinitex_flag_;

    const char* thread_name_;
    bool couninitex_hack_;

    Options()
        : stack_size_(0),
          use_com_(false),
          coinitex_flag_((uint32_t)-1),
          thread_name_(NULL),
          couninitex_hack_(false) {}

    Options(uint32_t stack_size, bool use_com = false,
            uint32_t coinitex_flag = -1, LPCSTR thread_name = NULL)
        : stack_size_(stack_size),
          use_com_(use_com),
          coinitex_flag_(coinitex_flag),
          thread_name_(thread_name),
          couninitex_hack_(false) {}
  };

  struct StartupData {
   public:
    const Thread::Options& options_;
    HANDLE started_event_;
    explicit StartupData(const Options& opt) : options_(opt) {
      started_event_ = ::CreateEvent(NULL, FALSE, FALSE, NULL);
    }
  };

 public:
  explicit Thread(LPCSTR default_name = NULL);
  virtual ~Thread(void);

 public:
  bool Start();
  bool StartWithOptions(const Options& options);

  bool Stop();
  bool StopSoon();

  MessageLoop* GetMessageLoop() const { return message_loop_; }
  HANDLE GetThreadHandle() { return thread_; }
  ThreadId GetThreadID() const { return thread_id_; }

  bool IsRunning() const { return thread_id_ != 0; }

  void ThreadMain();

 protected:
  void Run(MessageLoop* message_loop);

 private:
  bool ThreadWasStarted() const { return NULL != startup_data_; }

 private:
  bool stopping_;
  StartupData* startup_data_;

  MessageLoop* message_loop_;

  ThreadId thread_id_;
  HANDLE thread_;

  char name_[64];
};
}  // namespace gui

#endif  // GUI_BASE_THREAD_THREAD_H_
