// Copyright (c) 2023 The MGIS Authors.
// All rights reserved.

#ifndef GUI_BASE_THREAD_THREAD_NAME_H_
#define GUI_BASE_THREAD_THREAD_NAME_H_

#include <Windows.h>

namespace gui {
struct DebuggerThreadName {
#pragma pack(push, 8)
  struct ThreadNameInfo {
    DWORD dwType;
    LPCSTR szName;
    DWORD dwThreadID;
    DWORD dwFlags;
  };
#pragma pack(pop)

  static void SetThreadName(DWORD dwThreadID, LPCSTR pszThreadName) {
    if (!::IsDebuggerPresent()) return;

    if (dwThreadID == -1) {
      dwThreadID = ::GetCurrentThreadId();
    }

    struct ThreadNameInfo info = {0x1000, pszThreadName, dwThreadID, 0};

    __try {
      RaiseException(0x406D1388, 0, sizeof(info) / sizeof(ULONG_PTR),
                     reinterpret_cast<ULONG_PTR*>(&info));
    } __except (GetExceptionCode() == 0x406D1388 ? EXCEPTION_EXECUTE_HANDLER
                                                 : EXCEPTION_CONTINUE_SEARCH) {
      (0);  // This line fixes warning:C6322 (Empty _except block)
    }
  }
};
}  // namespace gui

#endif  // GUI_BASE_THREAD_THREAD_NAME_H_
