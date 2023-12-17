// Copyright (c) 2023 The Mogu Authors.
// All rights reserved.

#include "base/thread/thread_local.h"
#if defined(OS_POSIX)
#include <assert.h>
#include <pthread.h>

namespace base {

namespace internal {

// static
void ThreadLocalPlatform::AllocateSlot(SlotType &slot) {
  int error = pthread_key_create(&slot, NULL);
  assert(error == 0);
}

// static
void ThreadLocalPlatform::FreeSlot(SlotType &slot) {
  int error = pthread_key_delete(slot);
  assert(error == 0);
}

// static
void *ThreadLocalPlatform::GetValueFromSlot(SlotType &slot) {
  return pthread_getspecific(slot);
}

// static
void ThreadLocalPlatform::SetValueInSlot(SlotType &slot, void *value) {
  int error = pthread_setspecific(slot, value);
  assert(error == 0);
}

}  // namespace internal

}  // namespace base

#endif  // OS_POSIX
