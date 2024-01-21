// Copyright (c) 2023 The MGIS Authors.
// All rights reserved.

#ifndef GUI_BASE_LOCK_H
#define GUI_BASE_LOCK_H

#include <assert.h>
#include <windows.h>

namespace gui {
class Lock {
 public:
  typedef CRITICAL_SECTION OSLockType;

  Lock() { ::InitializeCriticalSectionAndSpinCount(&os_lock_, 2000); }
  ~Lock() { ::DeleteCriticalSection(&os_lock_); }

  bool Try() {
    if (::TryEnterCriticalSection(&os_lock_)) {
      return true;
    }
    return false;
  }
  void Acquire() { ::EnterCriticalSection(&os_lock_); }
  void Release() { ::LeaveCriticalSection(&os_lock_); }

 private:
  OSLockType os_lock_;
};

class AutoLock {
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

class AutoUnlock {
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
}  // namespace gui
#endif  // GUI_BASE_LOCK_H
