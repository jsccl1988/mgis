// Copyright (c) 2023 The MGIS Authors.
// All rights reserved.

#pragma comment( \
    linker,      \
    "\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include "gui/base/scoped_process_initializer.h"
#include "gui/main_runner.h"

ScopedProcessInitializer initiaizer_;

int WINAPI _tWinMain(HINSTANCE instance, HINSTANCE /*hPrevInstance*/,
                     LPTSTR lpstrCmdLine, int show) {
  base::ScopedRefPtr<gui::MainRunner> runner = new gui::MainRunner;
  if (runner->Init()) {
    runner->Run();
    runner->Stop();
  }

  return 0;
}
