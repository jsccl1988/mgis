// Copyright (c) 2023 The Mogu Authors.
// All rights reserved.

#ifndef BASE_PLATFORM_H_
#define BASE_PLATFORM_H_

#include "base/compiler_specific.h"
#include "base/macros.h"

#if defined(OS_WIN)
#include <windows.h>

#include <atlcomcli.h>
#include <atlstr.h>
using namespace ATL;
#endif  // OS_WIN

#if defined(OS_WIN)
typedef HANDLE ProcessHandle;
typedef DWORD ProcessId;
typedef DWORD ThreadId;
#elif defined(OS_POSIX)
typedef pid_t ThreadId;
#endif

#endif  // BASE_PLATFORM_H_
