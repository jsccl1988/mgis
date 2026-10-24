// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

// Product PE entry (mgis.exe). Dispatches workbench / harness / layout check.

#include <tchar.h>
#include <windows.h>
#include <atlbase.h>

#include "app/mgis/app/cmdline/launch_switches.h"
#include "app/mgis/harness/harness.h"
#include "app/mgis/layout/check_layout.h"
#include "base/at_exit.h"
#include "base/command_line.h"
#include "base/i18n/icu_util.h"
#include "base/message_loop.h"
#include "base/process_util.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/base/ui_base_paths.h"
#include "ui/base/win/dpi.h"
#include "ui/base/win/scoped_ole_initializer.h"

CComModule _Module;

int APIENTRY _tWinMain(HINSTANCE hInstance,
                       HINSTANCE hPrevInstance,
                       LPTSTR lpCmdLine,
                       int nCmdShow) {
  UNREFERENCED_PARAMETER(hInstance);
  UNREFERENCED_PARAMETER(hPrevInstance);
  UNREFERENCED_PARAMETER(lpCmdLine);
  UNREFERENCED_PARAMETER(nCmdShow);

  CommandLine::Init(0, NULL);
  if (CommandLine::ForCurrentProcess()->HasSwitch(mgis::switches::kCheckLayout))
    return mgis::CheckLayout();

  // Before any HWND / GetDeviceCaps: opt out of OS bitmap scaling.
  ui::EnableHighDPISupport();

  ui::ScopedOleInitializer ole_initializer;

  base::EnableTerminationOnHeapCorruption();
  base::AtExitManager exit_manager;

  icu_util::Initialize();
  ui::RegisterPathProvider();
  ui::ResourceBundle::InitSharedInstanceWithLocale("zh-CN", NULL);

  MessageLoopForUI main_message_loop;
  int exit_code = 0;
  if (CommandLine::ForCurrentProcess()->HasSwitch(
          mgis::switches::kHarnessChrome)) {
    exit_code = mgis::RunChromeHarness();
  } else if (CommandLine::ForCurrentProcess()->HasSwitch(
                 mgis::switches::kRenderCheck)) {
    exit_code = mgis::RunRenderCheck();
  } else {
    mgis::ShowMainWindow();
    MessageLoopForUI::current()->Run();
  }

  ui::ResourceBundle::CleanupSharedInstance();
  return exit_code;
}
