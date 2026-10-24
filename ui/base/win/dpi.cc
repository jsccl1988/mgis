// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/base/win/dpi.h"

#include <windows.h>

#include "base/win/scoped_hdc.h"
#include "base/win/registry.h"
#include "ui/gfx/display.h"
#include "ui/gfx/point_conversions.h"
#include "ui/gfx/rect_conversions.h"
#include "ui/gfx/size_conversions.h"

namespace {

int kDefaultDPIX = 96;
int kDefaultDPIY = 96;

int g_dpi_x = 0;
int g_dpi_y = 0;
bool g_dpi_initialized = false;

void ClearCachedDPI() {
  g_dpi_initialized = false;
  g_dpi_x = 0;
  g_dpi_y = 0;
}

float GetDeviceScaleFactorImpl() {
#if defined(ENABLE_HIDPI)
  // Use the continuous DPI scale for layout/compositing. Resource packs still
  // pick the nearest ScaleFactor via GetScaleFactorFromScale(); quantizing here
  // (e.g. 2.5 -> 1.8) left UI undersized on common 150%/250% displays.
  if (gfx::Display::HasForceDeviceScaleFactor())
    return gfx::Display::GetForcedDeviceScaleFactor();
  return ui::GetDPIScale();
#else
  return 1.0f;
#endif
}

BOOL IsProcessDPIAwareWrapper() {
  typedef BOOL(WINAPI *IsProcessDPIAwarePtr)(VOID);
  IsProcessDPIAwarePtr is_process_dpi_aware_func =
      reinterpret_cast<IsProcessDPIAwarePtr>(
          GetProcAddress(GetModuleHandleA("user32.dll"), "IsProcessDPIAware"));
  if (is_process_dpi_aware_func)
    return is_process_dpi_aware_func();
  return FALSE;
}

}  // namespace

namespace ui {

gfx::Size GetDPI() {
  if (!g_dpi_initialized) {
    g_dpi_initialized = true;
    g_dpi_x = kDefaultDPIX;
    g_dpi_y = kDefaultDPIY;
    // Effective DPI of the primary monitor (Shcore). GetDpiForSystem /
    // LOGPIXELSX often stay 96 while GetDpiForWindow reports 240 on a 250%
    // panel — that desync crushed NativeWidgetWin DIP layout.
    typedef HRESULT(WINAPI *GetDpiForMonitorFn)(HMONITOR, int, UINT*, UINT*);
    HMODULE shcore = LoadLibraryW(L"Shcore.dll");
    if (shcore) {
      GetDpiForMonitorFn get_dpi_for_monitor =
          reinterpret_cast<GetDpiForMonitorFn>(
              GetProcAddress(shcore, "GetDpiForMonitor"));
      if (get_dpi_for_monitor) {
        HMONITOR primary =
            MonitorFromWindow(NULL, MONITOR_DEFAULTTOPRIMARY);
        UINT dpi_x = 0;
        UINT dpi_y = 0;
        // MDT_EFFECTIVE_DPI == 0
        if (primary &&
            SUCCEEDED(get_dpi_for_monitor(primary, 0, &dpi_x, &dpi_y)) &&
            dpi_x > 0) {
          g_dpi_x = static_cast<int>(dpi_x);
          g_dpi_y = static_cast<int>(dpi_y > 0 ? dpi_y : dpi_x);
          FreeLibrary(shcore);
          return gfx::Size(g_dpi_x, g_dpi_y);
        }
      }
      FreeLibrary(shcore);
    }
    typedef UINT(WINAPI *GetDpiForSystemFn)(void);
    GetDpiForSystemFn get_dpi_for_system = reinterpret_cast<GetDpiForSystemFn>(
        GetProcAddress(GetModuleHandleW(L"user32.dll"), "GetDpiForSystem"));
    if (get_dpi_for_system) {
      const UINT dpi = get_dpi_for_system();
      if (dpi > 0) {
        g_dpi_x = static_cast<int>(dpi);
        g_dpi_y = static_cast<int>(dpi);
        return gfx::Size(g_dpi_x, g_dpi_y);
      }
    }
    base::win::ScopedGetDC screen_dc(NULL);
    g_dpi_x = GetDeviceCaps(screen_dc, LOGPIXELSX);
    g_dpi_y = GetDeviceCaps(screen_dc, LOGPIXELSY);
  }
  return gfx::Size(g_dpi_x, g_dpi_y);
}

float GetDPIScale() {
  return static_cast<float>(GetDPI().width()) /
      static_cast<float>(kDefaultDPIX);
}

bool IsInHighDPIMode() {
  gfx::Size dpi(GetDPI());
  return dpi.width() > kDefaultDPIX || dpi.height() > kDefaultDPIY;
}

void EnableHighDPISupport() {
  // Prefer per-monitor V2 so GetDpiForSystem / window DPI match the monitor
  // that hosts mview_shell (common 150%/250% laptop + external packs).
  typedef BOOL(WINAPI *SetProcessDpiAwarenessContextPtr)(HANDLE);
  SetProcessDpiAwarenessContextPtr set_ctx =
      reinterpret_cast<SetProcessDpiAwarenessContextPtr>(GetProcAddress(
          GetModuleHandleW(L"user32.dll"), "SetProcessDpiAwarenessContext"));
  bool aware = false;
  if (set_ctx) {
    // DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2 == -4
    aware = set_ctx(reinterpret_cast<HANDLE>(static_cast<intptr_t>(-4))) != 0;
  }
  if (!aware) {
    typedef BOOL(WINAPI *SetProcessDPIAwarePtr)(VOID);
    SetProcessDPIAwarePtr set_process_dpi_aware_func =
        reinterpret_cast<SetProcessDPIAwarePtr>(GetProcAddress(
            GetModuleHandleA("user32.dll"), "SetProcessDPIAware"));
    if (set_process_dpi_aware_func)
      set_process_dpi_aware_func();
  }
  // Drop any LOGPIXELS* captured while Windows was virtualizing this process
  // to 96 DPI.
  ClearCachedDPI();
}

namespace win {

float GetDeviceScaleFactor() {
  if (gfx::Display::HasForceDeviceScaleFactor())
    return gfx::Display::GetForcedDeviceScaleFactor();
  // Do not permanently cache across EnableHighDPISupport(); GetDPIScale()
  // already caches LOGPIXELS after awareness is enabled.
  return GetDeviceScaleFactorImpl();
}

gfx::Point ScreenToDIPPoint(const gfx::Point& pixel_point) {
  return gfx::ToFlooredPoint(
      gfx::ScalePoint(pixel_point, 1.0f / GetDeviceScaleFactor()));
}

gfx::Rect ScreenToDIPRect(const gfx::Rect& pixel_bounds) {
  // TODO(kevers): Switch to non-deprecated method for float to int conversions.
  return gfx::ToFlooredRectDeprecated(
      gfx::ScaleRect(pixel_bounds, 1.0f / GetDeviceScaleFactor()));
}

gfx::Rect DIPToScreenRect(const gfx::Rect& dip_bounds) {
  // TODO(kevers): Switch to non-deprecated method for float to int conversions.
  return gfx::ToFlooredRectDeprecated(
      gfx::ScaleRect(dip_bounds, GetDeviceScaleFactor()));
}

gfx::Size ScreenToDIPSize(const gfx::Size& size_in_pixels) {
  return gfx::ToFlooredSize(
      gfx::ScaleSize(size_in_pixels, 1.0f / GetDeviceScaleFactor()));
}

gfx::Size DIPToScreenSize(const gfx::Size& dip_size) {
  return gfx::ToFlooredSize(gfx::ScaleSize(dip_size, GetDeviceScaleFactor()));
}

int GetSystemMetricsInDIP(int metric) {
  return static_cast<int>(GetSystemMetrics(metric) /
      GetDeviceScaleFactor() + 0.5);
}

double GetUndocumentedDPIScale() {
  // TODO(girard): Remove this code when chrome is DPIAware.
  static double scale = -1.0;
  if (scale == -1.0) {
    scale = 1.0;
    if (!IsProcessDPIAwareWrapper()) {
      base::win::RegKey key(HKEY_CURRENT_USER,
                            L"Control Panel\\Desktop\\WindowMetrics",
                            KEY_QUERY_VALUE);
      if (key.Valid()) {
        DWORD value = 0;
        if (key.ReadValueDW(L"AppliedDPI", &value) == ERROR_SUCCESS) {
          scale = static_cast<double>(value) / kDefaultDPIX;
        }
      }
    }
  }
  return scale;
}

}  // namespace win

}  // namespace ui
