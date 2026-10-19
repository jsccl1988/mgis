// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "content/browser/glang.runtime/exec/suite_runner.h"

#include <vector>

#include "content/public/script_runtime.h"

namespace content {
namespace {

void SetError(std::string* error, const std::string& message) {
  if (error)
    *error = message;
}

void ClearError(std::string* error) {
  if (error)
    error->clear();
}

std::string Trim(const std::string& line) {
  size_t begin = 0;
  while (begin < line.size() && (line[begin] == ' ' || line[begin] == '\t'))
    ++begin;
  size_t end = line.size();
  while (end > begin && (line[end - 1] == ' ' || line[end - 1] == '\t' ||
                         line[end - 1] == '\r'))
    --end;
  return line.substr(begin, end - begin);
}

bool SplitVerb(const std::string& line,
               std::string* verb,
               std::string* rest) {
  const std::string trimmed = Trim(line);
  if (trimmed.empty()) {
    verb->clear();
    rest->clear();
    return false;
  }
  size_t split = 0;
  while (split < trimmed.size() && trimmed[split] != ' ' &&
         trimmed[split] != '\t')
    ++split;
  *verb = trimmed.substr(0, split);
  size_t rest_begin = split;
  while (rest_begin < trimmed.size() &&
         (trimmed[rest_begin] == ' ' || trimmed[rest_begin] == '\t'))
    ++rest_begin;
  *rest = trimmed.substr(rest_begin);
  return true;
}

}  // namespace

// Locked design: one in-process CPython executing main().
// This runner is the P0 stand-in until that interpreter is pinned.
bool run_suite_text(CapabilityHost& host,
                    const std::string& text,
                    std::string* error) {
  std::vector<std::string> pending;
  size_t index = 0;
  int line_no = 1;
  while (index < text.size()) {
    const size_t newline = text.find('\n', index);
    const bool has_newline = newline != std::string::npos;
    const size_t end = has_newline ? newline : text.size();
    const std::string raw = text.substr(index, end - index);

    std::string verb;
    std::string rest;
    if (SplitVerb(raw, &verb, &rest)) {
      if (verb == "mark") {
        if (rest.empty()) {
          SetError(error, "mark requires a name at line " +
                              std::to_string(static_cast<long long>(line_no)));
          return false;
        }
        pending.push_back(rest);
      } else {
        SetError(error, "unknown verb: " + verb + " at line " +
                            std::to_string(static_cast<long long>(line_no)));
        return false;
      }
    }

    if (!has_newline)
      break;
    index = end + 1;
    ++line_no;
  }

  for (size_t i = 0; i < pending.size(); ++i)
    host.mark(pending[i]);
  ClearError(error);
  return true;
}

}  // namespace content
