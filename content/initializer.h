// Copyright (c) 2023 The MGIS Authors.
// All rights reserved.

#ifndef CONTENT_INITIALIZER_H
#define CONTENT_INITIALIZER_H

#include "base/logging.h"
#include "base/path/base_paths.h"
#include "base/util/string_util.h"
#include "content/control/window/map_app.h"


namespace content {
class Initializer {
 public:
  Initializer() {
    ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
    InitDLLSearchPath();
    CreateATLModuleIfNeeded();
    CreateLogModuleIfNeeded();
  }
  ~Initializer() { CoUninitialize(); }

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
}  // namespace content

#endif  // CONTENT_INITIALIZER_H
