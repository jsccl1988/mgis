// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "content/browser/script_runtime.h"

#include "content/browser/glang/exec/suite_runner.h"
#include "content/browser/glang/language/suite_loader.h"

namespace content {

bool run_suite_script(CapabilityHost& host,
                      const std::string& path_utf8,
                      std::string* error) {
  std::string text;
  if (!load_suite_file(path_utf8, &text, error))
    return false;
  return run_suite_text(host, text, error);
}

}  // namespace content
