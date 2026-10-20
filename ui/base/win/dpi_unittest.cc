// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "ui/base/win/dpi.h"

#include <windows.h>

#include "testing/gtest/include/gtest/gtest.h"

namespace ui {
namespace {

TEST(WinDpiTest, EnableHighDPISupportMarksProcessAware) {
  EnableHighDPISupport();

  typedef BOOL(WINAPI *IsProcessDPIAwarePtr)(VOID);
  IsProcessDPIAwarePtr is_aware = reinterpret_cast<IsProcessDPIAwarePtr>(
      GetProcAddress(GetModuleHandleA("user32.dll"), "IsProcessDPIAware"));
  ASSERT_TRUE(is_aware);
  EXPECT_TRUE(is_aware());
}

TEST(WinDpiTest, DeviceScaleFactorMatchesGetDPIScaleWhenHidpiEnabled) {
  EnableHighDPISupport();
  // ENABLE_HIDPI is on for Windows product builds; layout scale must track
  // the real DPI ratio instead of staying pinned at 1.0f.
  EXPECT_FLOAT_EQ(GetDPIScale(), win::GetDeviceScaleFactor());
  EXPECT_GE(win::GetDeviceScaleFactor(), 1.0f);
}

}  // namespace
}  // namespace ui
