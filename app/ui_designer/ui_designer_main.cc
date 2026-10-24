// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <tchar.h>
#include <windows.h>
#include <cguid.h>
#include <initguid.h>
#include <oleacc.h>

#include <atlbase.h>

#include "base/at_exit.h"
#include "base/command_line.h"
#include "base/i18n/icu_util.h"
#include "base/message_loop.h"
#include "base/process_util.h"
#include "app/ui_designer/shell.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/base/resource/resource_bundle_win.h"
#include "ui/base/ui_base_paths.h"
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

  ui::ScopedOleInitializer ole_initer;
  CommandLine::Init(0, NULL);
  base::EnableTerminationOnHeapCorruption();
  base::AtExitManager exit_manager;
  icu_util::Initialize();
  ui::RegisterPathProvider();
  ui::ResourceBundle::InitSharedInstanceWithLocale("zh-CN", NULL);
  base::MessageLoopForUI main_message_loop;

  ui_designer::ShowUiDesigner();
  base::MessageLoopForUI::current()->Run();
  ui::ResourceBundle::CleanupSharedInstance();
  return 0;
}
