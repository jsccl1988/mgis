// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#ifndef CONTENT_BROWSER_GLANG_RUNTIME_EXEC_SUITE_RUNNER_H_
#define CONTENT_BROWSER_GLANG_RUNTIME_EXEC_SUITE_RUNNER_H_

#include <string>

namespace content {

class CapabilityHost;

// Locked design: one in-process CPython executing main().
// This runner is the P0 stand-in until that interpreter is pinned.
// It does not vendor CPython. It reads suite text and applies lines of the
// form `mark <name>`. Any other verb fails the run.
// On failure, no mark from |text| is written to |host|.
bool run_suite_text(CapabilityHost& host,
                    const std::string& text,
                    std::string* error);

}  // namespace content

#endif  // CONTENT_BROWSER_GLANG_RUNTIME_EXEC_SUITE_RUNNER_H_
