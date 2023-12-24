// Copyright (c) 2023 The Mogu Authors.
// All rights reserved.

#ifndef BASE_THREAD_THREAD_MANAGER_H_
#define BASE_THREAD_THREAD_MANAGER_H_

#include <map>
#include <memory>

#include "base/base_export.h"
#include "base/memory/singleton.h"
#include "base/thread/framework_thread.h"

namespace base {
class MessageLoop;
class MessageLoopProxy;

class ThreadMap {
 public:
  SINGLETON_DEFINE(ThreadMap);
  static bool AquireAccess();

  bool RegisterThread(int self_identifier);
  bool UnregisterThread();
  int QueryThreadId(const FrameworkThread *thread);
  std::shared_ptr<MessageLoopProxy> GetMessageLoop(int identifier) const;
  FrameworkThread *QueryThreadInternal(int identifier) const;

 private:
  ThreadMap() {}
  Lock lock_;
  std::map<int, FrameworkThread *> threads_;
};

class BASE_EXPORT ThreadManager {
 public:
  static bool RegisterThread(int self_identifier);
  static bool UnregisterThread();

  static FrameworkThread *CurrentThread();
  template <typename T>
  static T *CurrentThreadT();
  static int QueryThreadId(const FrameworkThread *thread);

  static bool PostTask(const StdClosure &task);
  static bool PostTask(int identifier, const StdClosure &task);

  static bool PostDelayedTask(const StdClosure &task, TimeDelta delay);
  static bool PostDelayedTask(int identifier, const StdClosure &task,
                              TimeDelta delay);

  static const int TIMES_FOREVER = -1;
  static void PostRepeatedTask(const WeakCallback<StdClosure> &task,
                               const TimeDelta &delay,
                               int times = TIMES_FOREVER);
  static void PostRepeatedTask(int thread_id,
                               const WeakCallback<StdClosure> &task,
                               const TimeDelta &delay,
                               int times = TIMES_FOREVER);

  static bool PostNonNestableTask(const StdClosure &task);
  static bool PostNonNestableTask(int identifier, const StdClosure &task);

  static bool PostNonNestableDelayedTask(const StdClosure &task,
                                         TimeDelta delay);
  static bool PostNonNestableDelayedTask(int identifier, const StdClosure &task,
                                         TimeDelta delay);

  template <typename T1, typename T2>
  static bool Await(int identifier, const std::function<T1> &task,
                    const std::function<T2> &reply) {
    std::shared_ptr<MessageLoopProxy> message_loop =
        ThreadMap::GetInstance()->GetMessageLoop(identifier);
    if (message_loop == NULL) return false;
    message_loop->PostTaskAndReply(task, reply);
    return true;
  }

 private:
  static void RunRepeatedly(const WeakCallback<StdClosure> &task,
                            const TimeDelta &delay, int times);
  static void RunRepeatedly2(int thread_id,
                             const WeakCallback<StdClosure> &task,
                             const TimeDelta &delay, int times);

  DISALLOW_COPY_AND_ASSIGN(ThreadManager);
};

template <typename T>
T *ThreadManager::CurrentThreadT() {
  return static_cast<T *>(CurrentThread());
}
}  // namespace base

#endif  // BASE_THREAD_THREAD_MANAGER_H_
