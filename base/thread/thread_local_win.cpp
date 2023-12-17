// Copyright (c) 2023 The Mogu Authors.
// All rights reserved.

#include "base/thread/thread_local.h"
#if defined(OS_WIN)
#include <assert.h>
#include <windows.h>

namespace base {
namespace internal {

// static
void ThreadLocalPlatform::AllocateSlot(SlotType &slot) {
  slot = ::TlsAlloc();
  assert(slot != TLS_OUT_OF_INDEXES);
}

// static
void ThreadLocalPlatform::FreeSlot(SlotType &slot) {
  if (!::TlsFree(slot)) {
    assert(false);
  }
}

// static
void *ThreadLocalPlatform::GetValueFromSlot(SlotType &slot) {
  return ::TlsGetValue(slot);
}

// static
void ThreadLocalPlatform::SetValueInSlot(SlotType &slot, void *value) {
  if (!::TlsSetValue(slot, value)) {
    assert(false);
  }
}
}  // namespace internal
}  // namespace base

#endif  // OS_WIN
