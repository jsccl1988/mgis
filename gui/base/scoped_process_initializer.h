// Copyright (c) 2023 The MGIS Authors.
// All rights reserved.

#ifndef GUI_BASE_SCOPED_PROCESS_INITIALIZER_H_
#define GUI_BASE_SCOPED_PROCESS_INITIALIZER_H_

#include <windows.h>
#include <stdio.h>

class ScopedProcessInitializer {
 public:
  ScopedProcessInitializer() {
    ::CoInitializeEx(NULL, COINIT_MULTITHREADED);

    CreateATLModuleIfNeeded();
    CreateLogModuleIfNeeded();
  }
  ~ScopedProcessInitializer() { CoUninitialize(); }

 private:
  void CreateATLModuleIfNeeded() {
    if (_pAtlModule == NULL) {
      // This creates the module and automatically updates _pAtlModule.
      static CComModule& module = *new CComModule();
    }
  }

  void CreateLogModuleIfNeeded() {}
};

#endif  // GUI_BASE_SCOPED_PROCESS_INITIALIZER_H_
