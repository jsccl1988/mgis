// Copyright (c) 2023 The MGIS Authors.
// All rights reserved.

#include <shellscalingapi.h>

#include "base/logging.h"
#include "base/util/string_util.h"
#include "gui/app/main_runner.h"
#include "gui/app/resource.h"
#include "gui/base/scoped_process_initializer.h"


// Win8.1 supports monitor-specific DPI scaling.
bool SetProcessDpiAwarenessWrapper(PROCESS_DPI_AWARENESS value) {
  typedef HRESULT(WINAPI * SetProcessDpiAwarenessPtr)(PROCESS_DPI_AWARENESS);
  SetProcessDpiAwarenessPtr set_process_dpi_awareness_func =
      reinterpret_cast<SetProcessDpiAwarenessPtr>(GetProcAddress(
          GetModuleHandleA("user32.dll"), "SetProcessDpiAwarenessInternal"));
  if (set_process_dpi_awareness_func) {
    HRESULT hr = set_process_dpi_awareness_func(value);
    if (SUCCEEDED(hr)) {
      LOG(INFO) << "SetProcessDpiAwareness succeeded.";
      return true;
    } else if (hr == E_ACCESSDENIED) {
      LOG(ERROR) << "Access denied error from SetProcessDpiAwareness.\
      Function called twice, or manifest was used.";
    }
  }
  return false;
}

// This function works for Windows Vista through Win8. Win8.1 must use
// SetProcessDpiAwareness[Wrapper].
BOOL SetProcessDPIAwareWrapper() {
  typedef BOOL(WINAPI * SetProcessDPIAwarePtr)(VOID);
  SetProcessDPIAwarePtr set_process_dpi_aware_func =
      reinterpret_cast<SetProcessDPIAwarePtr>(
          GetProcAddress(GetModuleHandleA("user32.dll"), "SetProcessDPIAware"));
  return set_process_dpi_aware_func && set_process_dpi_aware_func();
}

void EnableHighDPISupport() {
  if (!SetProcessDpiAwarenessWrapper(PROCESS_SYSTEM_DPI_AWARE)) {
    BOOL success = SetProcessDPIAwareWrapper();
    if (success) {
      LOG(INFO) << "SetProcessDPIAwareWrapper succeeded.";
    }
  }
}

int WINAPI _tWinMain(HINSTANCE instance, HINSTANCE /*hPrevInstance*/,
                     LPTSTR lpstrCmdLine, int show) {
  ScopedProcessInitializer process_initializer;
  EnableHighDPISupport();
  base::ScopedRefPtr<gui::MainRunner> runner = new gui::MainRunner();
  if (runner->Init()) {
    runner->Run();
    runner->Stop();
  }

  return 0;
}
