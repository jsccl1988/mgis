// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#ifndef APP_MGIS_LAYOUT_CHECK_LAYOUT_H_
#define APP_MGIS_LAYOUT_CHECK_LAYOUT_H_

namespace mgis {

// Builds a temp install tree, checks relative product paths beside mgis.exe,
// and writes out/harness/browser.install/mark.json. Does not start Unreal.
int CheckLayout();

}  // namespace mgis

#endif  // APP_MGIS_LAYOUT_CHECK_LAYOUT_H_
