// Copyright (c) 2023 The MGIS Authors.
// All rights reserved.

#pragma comment( \
    linker,      \
    "\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include "base/logging.h"
#include "base/util/string_util.h"
#include "gui/app/window.h"
#include "gui/main_runner.h"

int WINAPI _tWinMain(HINSTANCE instance, HINSTANCE /*hPrevInstance*/,
                     LPTSTR lpstrCmdLine, int show) {
  gui::MainRunner runner;
  if (runner.Init()) {
    runner.Run();
    runner.Stop();
  }

  return 0;
}
