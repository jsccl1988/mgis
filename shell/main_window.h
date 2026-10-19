// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#ifndef SHELL_MAIN_WINDOW_H_
#define SHELL_MAIN_WINDOW_H_

namespace shell {

// Shows the product main window plus the map, view0, and view1 widgets.
// The caller owns MessageLoopForUI and must run it.
void ShowMainWindow();

// Creates the same four widgets, writes out/harness/browser.chrome/mark.json
// after they exist, then closes them and runs the current message loop until
// quit. Returns 0 when all six browser.chrome marks were recorded.
// Does not start UE. The caller must already have a MessageLoopForUI.
int RunChromeHarness();

}  // namespace shell

#endif  // SHELL_MAIN_WINDOW_H_
