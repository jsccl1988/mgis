// Copyright (c) 2023 The Mogu Authors.
// All rights reserved.

#ifndef BASE_FRAMEWORK_MESSAGE_LOOP_H_
#define BASE_FRAMEWORK_MESSAGE_LOOP_H_

#include <memory>
#include <queue>

#include "base/base_types.h"
#include "base/framework/default_message_pump.h"
#include "base/framework/message_loop_proxy.h"
#include "base/framework/observer_list.h"
#if defined(OS_WIN)
#include "base/framework/win_io_message_pump.h"
#include "base/framework/win_ui_message_pump.h"
#elif defined(OS_POSIX)
#include "base/framework/libevent_message_pump.h"
#endif  // OS_WIN
#include "base/callback/callback.h"
#include "base/synchronization/lock.h"
#include "base/time/time.h"

namespace base {
class UIMessageLoop;
class IOMessageLoop;
class UvMessageLoop;
class MessageLoopProxy;

#if defined(OS_WIN)
typedef WinIOMessagePump IOMessagePump;
typedef WinUIMessagePump UIMessagePump;
typedef WinMessagePump::Dispatcher Dispatcher;
#elif defined(OS_POSIX)
typedef LibeventMessagePump IOMessagePump;
#else
#error Not support currently!
#endif

class BASE_EXPORT MessageLoop : public MessagePump::Delegate {
 public:
  enum Type {
    kDefaultMessageLoop,
    kUIMessageLoop,
    kIOMessageLoop,
    kCustomMessageLoop,
  };

  explicit MessageLoop();
  virtual ~MessageLoop();
  static MessageLoop *current();

#if defined(OS_WIN)
  UIMessageLoop *ToUIMessageLoop();
  IOMessageLoop *ToIOMessageLoop();
#elif defined(OS_POSIX)
  IOMessageLoop *ToIOMessageLoop();
#endif

  MessagePump *pump() { return pump_.get(); }

  Type type() const { return type_; }

  void Run();
#if defined(OS_WIN)
  void RunWithDispatcher(Dispatcher *dispatcher);
#endif

  void RunAllPending();

  void Quit();
  void QuitNow();

  void PostTask(const StdClosure &task);
  void PostDelayedTask(const StdClosure &task, TimeDelta delay);
  void PostNonNestableTask(const StdClosure &task);
  void PostNonNestableDelayedTask(const StdClosure &task, TimeDelta delay);

  void SetNestableTasksAllowed(bool allowed);
  bool IsNestableTasksAllowed() const { return nestable_tasks_allowed_; }
  bool IsNested() const {
    if (state_) return state_->run_depth > 1;
    return false;
  }

  std::shared_ptr<MessageLoopProxy> message_loop_proxy() {
    return message_loop_proxy_;
  }

  class BASE_EXPORT DestructionObserver {
   public:
    virtual void PreDestroyCurrentMessageLoop() = 0;
    virtual ~DestructionObserver() {}
  };

  void AddDestructionObserver(DestructionObserver *observer);
  void RemoveDestructionObserver(DestructionObserver *observer);

  class BASE_EXPORT TaskObserver {
   public:
    virtual void PreProcessTask() = 0;
    virtual void PostProcessTask() = 0;

   protected:
    virtual ~TaskObserver();
  };

  void AddTaskObserver(TaskObserver *observer);
  void RemoveTaskObserver(TaskObserver *observer);

#if defined(OS_WIN)
  bool os_modal_loop() const { return os_modal_loop_; }
  void set_os_modal_loop(bool os_modal_loop) { os_modal_loop_ = os_modal_loop; }
#endif  // OS_WIN

 protected:
  struct RunState {
    int run_depth;
    bool quit_received;
#if defined(OS_WIN)
    Dispatcher *dispatcher;
#endif
  };

  class BASE_EXPORT AutoRunState : RunState {
   public:
    explicit AutoRunState(MessageLoop *loop);
    ~AutoRunState();

   private:
    MessageLoop *loop_;
    RunState *previous_state_;
  };

  class ScopedNestableTaskAllower {
   public:
    explicit ScopedNestableTaskAllower(MessageLoop *loop)
        : loop_(loop), old_state_(loop_->IsNestableTasksAllowed()) {
      loop_->SetNestableTasksAllowed(true);
    }
    ~ScopedNestableTaskAllower() { loop_->SetNestableTasksAllowed(old_state_); }

   private:
    MessageLoop *loop_;
    bool old_state_;
  };

  struct PendingTask {
    PendingTask(const StdClosure &task);
    PendingTask(const StdClosure &task, base::TimeTicks delayed_run_time,
                bool nestable);

    ~PendingTask();

    bool operator<(const PendingTask &other) const;
    base::TimeTicks delayed_run_time;
    int sequence_num;
    bool nestable;

    void Run() {
      if (std_task) {
        std_task();
      } else {
        assert(false);
      }
    }

   private:
    StdClosure std_task;
  };

  class TaskQueue : public std::queue<PendingTask> {
   public:
    void Swap(TaskQueue *queue) { c.swap(queue->c); }
  };

  typedef std::priority_queue<PendingTask> DelayedTaskQueue;

  virtual bool DoWork();
  virtual bool DoDelayedWork(base::TimeTicks *next_delayed_work_time);
  virtual bool DoIdleWork();

  void RunInternal();

  virtual void AddToIncomingQueue(const PendingTask &task);
  void AddToDelayedWorkQueue(const PendingTask &task);
  void ReloadWorkQueue();
  bool DeferOrRunPendingTask(const PendingTask &task);
  void RunTask(const PendingTask &task);
  bool ProcessNextDelayedNonNestableTask();
  bool DeletePendingTasks();

  void PreDestruct();
  void PreProcessTask();
  void PostPrecessTask();

  static TimeTicks EvalDelayedRuntime(int64_t delay_ms);

  Type type_;
  RunState *state_;
  std::shared_ptr<MessagePump> pump_;

#if defined(OS_WIN)
  bool os_modal_loop_;
#endif  // OS_WIN

  bool nestable_tasks_allowed_;
  TaskQueue incoming_queue_;
  Lock incoming_queue_lock_;

  TaskQueue work_queue_;
  TaskQueue deferred_non_nestable_work_queue_;
  DelayedTaskQueue delayed_work_queue_;
  int next_delayed_task_sequence_num_;
  TimeTicks recent_tick_;

  ObserverList<TaskObserver> task_observers_;
  ObserverList<DestructionObserver> destruction_observers_;
  std::shared_ptr<MessageLoopProxy> message_loop_proxy_;

  DISALLOW_COPY_AND_ASSIGN(MessageLoop);
};

#if defined(OS_WIN)

class BASE_EXPORT UIMessageLoop : public MessageLoop {
 public:
#if defined(OS_WIN)
  typedef WinUIMessagePump::UIObserver UIObserver;
#else
#error Not support currently!
#endif

  UIMessageLoop();

  static UIMessageLoop *current() {
    return reinterpret_cast<UIMessageLoop *>(MessageLoop::current());
  }

  void AddUIObserver(UIObserver *observer);
  void RemoveUIObserver(UIObserver *observer);
};

#endif  // OS_WIN

class BASE_EXPORT IOMessageLoop : public MessageLoop {
 public:
#if defined(OS_WIN)
  typedef WinIOMessagePump::IOHandler IOHandler;
  typedef WinIOMessagePump::IOContext IOContext;
  typedef WinIOMessagePump::IOObserver IOObserver;
#elif defined(OS_POSIX)
  typedef LibeventMessagePump::Watcher Watcher;
  typedef LibeventMessagePump::FileDescriptorWatcher FileDescriptorWatcher;
  typedef LibeventMessagePump::IOObserver IOObserver;

  enum Mode{WATCH_READ = LibeventMessagePump::WATCH_READ,
            WATCH_WRITE = LibeventMessagePump::WATCH_WRITE,
            WATCH_READ_WRITE = LibeventMessagePump::WATCH_READ_WRITE};
#else
#error Not support currently!
#endif

  IOMessageLoop();

  static IOMessageLoop *current() {
    return reinterpret_cast<IOMessageLoop *>(MessageLoop::current());
  }

  void AddIOObserver(IOObserver *observer);
  void RemoveIOObserver(IOObserver *observer);

#if defined(OS_WIN)
  void RegisterIOHandler(HANDLE file_handle, IOHandler *handler);
  bool WaitForIOCompletion(DWORD timeout, IOHandler *handler);

#elif defined(OS_POSIX)
  bool WatchFileDescriptor(int fd, bool persistent, Mode mode,
                           FileDescriptorWatcher *controller,
                           Watcher *delegate);
#endif
};
}  // namespace base

#endif  // BASE_FRAMEWORK_MESSAGE_LOOP_H_
