// Copyright (c) 2023 The Mogu Authors.
// All rights reserved.

#include "lock.h"

#if defined(OS_POSIX)

#include <errno.h>

namespace base {

NLock::NLock() {
  // In release, go with the default lock attributes.
  pthread_mutex_init(&os_lock_, NULL);
}

NLock::~NLock() { pthread_mutex_destroy(&os_lock_); }

bool NLock::Try() {
  int rv = pthread_mutex_trylock(&os_lock_);
  return rv == 0;
}

void NLock::Lock() { pthread_mutex_lock(&os_lock_); }

void NLock::Unlock() { pthread_mutex_unlock(&os_lock_); }

}  // namespace base

#endif  // OS_WIN