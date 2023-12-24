// Copyright (c) 2023 The Mogu Authors.
// All rights reserved.

#include "lock.h"

#if defined(OS_WIN)

namespace base {

Lock::Lock() {
  // The second parameter is the spin count, for short-held locks it avoid the
  // contending thread from going to sleep which helps performance greatly.
  ::InitializeCriticalSectionAndSpinCount(&os_lock_, 2000);
}

Lock::~Lock() { ::DeleteCriticalSection(&os_lock_); }

bool Lock::Try() {
  if (::TryEnterCriticalSection(&os_lock_)) {
    return true;
  }
  return false;
}

void Lock::Acquire() { ::EnterCriticalSection(&os_lock_); }

void Lock::Release() { ::LeaveCriticalSection(&os_lock_); }

}  // namespace base

#endif  // OS_WIN
