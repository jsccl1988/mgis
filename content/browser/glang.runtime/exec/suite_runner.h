// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#ifndef CONTENT_BROWSER_GLANG_RUNTIME_EXEC_SUITE_RUNNER_H_
#define CONTENT_BROWSER_GLANG_RUNTIME_EXEC_SUITE_RUNNER_H_

#include <string>

namespace content {

class CapabilityHost;

// Locked design: one in-process CPython executing main().
// This runner is the P0 stand-in until that interpreter is pinned.
// It does not vendor CPython. It reads suite text and applies mark, command,
// activate_tool, pointer, attach, and scene_mode lines. expect_camera runs
// when a slot is installed. expect_layer, expect_entity, and capture fail
// closed.
// On a parse failure, no mark from |text| is written to |host|.
bool run_suite_text(CapabilityHost& host,
                    const std::string& text,
                    std::string* error);

}  // namespace content

#endif  // CONTENT_BROWSER_GLANG_RUNTIME_EXEC_SUITE_RUNNER_H_
