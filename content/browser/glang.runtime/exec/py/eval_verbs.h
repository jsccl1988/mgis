// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

// Thin CapabilityHost dispatch for glang.plugin calls. No opcode table.

#ifndef CONTENT_BROWSER_GLANG_RUNTIME_EXEC_PY_EVAL_VERBS_H_
#define CONTENT_BROWSER_GLANG_RUNTIME_EXEC_PY_EVAL_VERBS_H_

#include <string>

#include "content/browser/glang.runtime/exec/py/script_call.h"

namespace content {

class CapabilityHost;

namespace code {

// Maps |call.name| onto CapabilityHost slots. Supports mark, command,
// activate_tool, pointer, scene_mode, expect_camera, and attach.
// expect_layer, expect_entity, and capture fail closed with a clear error.
bool eval_script_call(CapabilityHost& host,
                      const ScriptCall& call,
                      std::string* error,
                      std::string* result = nullptr);

}  // namespace code
}  // namespace content

#endif  // CONTENT_BROWSER_GLANG_RUNTIME_EXEC_PY_EVAL_VERBS_H_
