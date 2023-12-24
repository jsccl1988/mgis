// Copyright (c) 2023 The Mogu Authors.
// All rights reserved.

#ifndef BASE_SYNCHRONIZATION_LOCK_H
#define BASE_SYNCHRONIZATION_LOCK_H

#include "base/base_config.h"
#include "base/base_export.h"

#if defined(OS_WIN)
#include <windows.h>
#elif defined(OS_POSIX)
#include <pthread.h>
#endif
#include <assert.h>

namespace base {

class BASE_EXPORT Lock {
 public:
#if defined(OS_WIN)
  typedef CRITICAL_SECTION OSLockType;
#elif defined(OS_POSIX)
  typedef pthread_mutex_t OSLockType;
#endif

  Lock();
  ~Lock();

  // If the lock is not held, take it and return true.  If the lock is already
  // held by something else, immediately return false.
  bool Try();

  // Take the acquire, blocking until it is available if necessary.
  void Acquire();

  // Release the lock.  This must only be called by the lock's holder: after
  // a successful call to Try, or a call to Lock.
  void Release();

  // Return the native underlying lock.  Not supported for Windows builds.
#if !defined(OS_WIN)
  OSLockType *os_lock() { return &os_lock_; }
#endif

 private:
  OSLockType os_lock_;
};

class BASE_EXPORT AutoLock {
 public:
  AutoLock(Lock *lock) {
    assert(lock);
    lock_ = lock;
    lock_->Acquire();
  }

  ~AutoLock() {
    if (lock_) lock_->Release();
  }

 private:
  Lock *lock_;
};

class BASE_EXPORT AutoUnlock {
 public:
  AutoUnlock(Lock *lock) {
    assert(lock);
    lock_ = lock;
    lock_->Release();
  }

  ~AutoUnlock() {
    if (lock_) lock_->Acquire();
  }

 private:
  Lock *lock_;
};

}  // namespace base

#endif  // BASE_SYNCHRONIZATION_LOCK_H
