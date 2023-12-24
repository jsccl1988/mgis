// Copyright (c) 2023 The MGIS Authors.
// All rights reserved.

#ifndef GUI_BASE_THREAD_THREAD_LOCAL_POINTER_H_
#define GUI_BASE_THREAD_THREAD_LOCAL_POINTER_H_

#include "base/platform.h"

namespace gui {
template <class Type>
class ThreadLocalPointer {
 public:
  ThreadLocalPointer() : slot_(0) { slot_ = TlsAlloc(); }
  ~ThreadLocalPointer() { TlsFree(slot_); }

  Type* Get() { return static_cast<Type*>(TlsGetValue(slot_)); }
  VOID Set(Type* ptr) { TlsSetValue(slot_, ptr); }

 private:
  DWORD slot_;
};
}  // namespace gui

#endif  // GUI_BASE_THREAD_LOCAL_POINTER_H_
