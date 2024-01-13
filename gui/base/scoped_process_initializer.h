// Copyright (c) 2023 The MGIS Authors.
// All rights reserved.

#ifndef GUI_BASE_SCOPED_PROCESS_INITIALIZER_H_
#define GUI_BASE_SCOPED_PROCESS_INITIALIZER_H_

#include <stdio.h>
#include <windows.h>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif // WIN32_LEAN_AND_MEAN

#include <atlbase.h>
#include <atlapp.h>
#include <atlcrack.h>
#include <atlmisc.h>
#include <atlwin.h>

#include "base/logging.h"
#include "base/path/base_paths.h"
#include "base/util/string_util.h"

class ScopedProcessInitializer {
 public:
  ScopedProcessInitializer() {
    ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
    InitDLLSearchPath();
    CreateATLModuleIfNeeded();
    CreateLogModuleIfNeeded();
  }
  ~ScopedProcessInitializer() { CoUninitialize(); }

 private:
  void InitDLLSearchPath() {
    base::PathString module_dir;
    base::PathProvider(base::FILE_MODULE_DIR, &module_dir);
    module_dir += L"third_party\\bin";
    base::DirectoryAsDLLSearchPath(module_dir);
    LOG(INFO) << __FUNCTION__ << base::UTF16ToUTF8(module_dir).c_str();
  }

  void CreateATLModuleIfNeeded() {
    if (_pAtlModule == NULL) {
      // This creates the module and automatically updates _pAtlModule.
      static CComModule& module = *new CComModule();
    }
  }

  void CreateLogModuleIfNeeded() {}
};

#endif  // GUI_BASE_SCOPED_PROCESS_INITIALIZER_H_
