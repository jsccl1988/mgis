// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#ifndef SHELL_MAIN_WINDOW_H_
#define SHELL_MAIN_WINDOW_H_

namespace shell {

// Shows the product mgis workbench (single main window). The caller owns
// MessageLoopForUI and must run it.
void ShowMainWindow();

// Creates the same four widgets, then closes them and runs the current
// message loop until quit. Does not write browser.chrome/mark.json: the
// suite runner is not on content/public/browser. Returns 0 when the four
// widgets and both trees exist. Does not start UE. The caller must already
// have a MessageLoopForUI.
int RunChromeHarness();

// Creates the map widget and the view0/view1 GlobeView satellites. Presents
// a browsed frame on view0. The map widget is not a MapLibre surface.
// Writes out/harness/render-check/mark.json and returns 0 only when the
// view0 frame was actually drawn.
int RunRenderCheck();

}  // namespace shell

#endif  // SHELL_MAIN_WINDOW_H_
