// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "content/browser/glang.runtime/script_runtime.h"

#include <string_view>

#include "base/logging.h"
#include "content/browser/glang.runtime/exec/py/cpython.h"
#include "content/browser/glang.runtime/exec/py/eval_verbs.h"
#include "content/browser/glang.runtime/exec/py/script_call.h"
#include "content/browser/glang.runtime/exec/suite_runner.h"
#include "content/browser/glang.runtime/language/suite_loader.h"

namespace content {
namespace {

bool EndsWithPy(std::string_view path) {
  if (path.size() < 3) {
    return false;
  }
  const std::string_view tail = path.substr(path.size() - 3);
  return tail == ".py" || tail == ".PY" || tail == ".Py" || tail == ".pY";
}

bool Fail(std::string text, std::string* error) {
  if (error) {
    *error = text;
  }
  LOG(ERROR) << "script: " << text;
  return false;
}

// Live verbs for one .py suite. Earlier statements keep their effects when a
// later name throws inside main().
class SuiteScriptHandler final : public code::ScriptHandler {
 public:
  explicit SuiteScriptHandler(CapabilityHost& host) : host_(host) {}

  bool call(const code::ScriptCall& call,
            std::string* error,
            std::string* result) override {
    return code::eval_script_call(host_, call, error, result);
  }

 private:
  CapabilityHost& host_;
};

}  // namespace

bool run_suite_script(CapabilityHost& host,
                      const std::string& path_utf8,
                      std::string* error) {
  if (EndsWithPy(path_utf8)) {
    if (!py::interpreter().is_ready() && !py::interpreter().init()) {
      return Fail("error: python not ready", error);
    }
    SuiteScriptHandler handler(host);
    code::set_script_handler(&handler);
    struct ClearHandler {
      ~ClearHandler() { code::clear_script_handler(); }
    } clear_handler;

    std::string py_error;
    if (!py::interpreter().exec_file(path_utf8, &py_error)) {
      return Fail(py_error.empty() ? "error: script failed" : py_error, error);
    }
    return true;
  }

  // Line interpreter retained for DebugAgent ExecLine / transitional suites.
  std::string text;
  if (!load_suite_file(path_utf8, &text, error))
    return false;
  return run_suite_text(host, text, error);
}

}  // namespace content
