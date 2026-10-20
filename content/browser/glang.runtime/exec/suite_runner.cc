// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "content/browser/glang.runtime/exec/suite_runner.h"

#include <stdlib.h>

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

bool SplitVerb(const std::string& line, std::string* verb, std::string* rest) {
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

void SplitTokens(const std::string& rest, std::vector<std::string>* out) {
  size_t index = 0;
  while (index < rest.size()) {
    while (index < rest.size() && (rest[index] == ' ' || rest[index] == '\t'))
      ++index;
    if (index >= rest.size())
      break;
    size_t end = index;
    while (end < rest.size() && rest[end] != ' ' && rest[end] != '\t')
      ++end;
    out->push_back(rest.substr(index, end - index));
    index = end;
  }
}

bool ParseInt(const std::string& text, int* out) {
  if (text.empty() || !out)
    return false;
  char* end = NULL;
  const long value = strtol(text.c_str(), &end, 10);
  if (!end || end == text.c_str() || *end != '\0')
    return false;
  *out = static_cast<int>(value);
  return true;
}

std::string LineError(const std::string& message, int line_no) {
  return message + " at line " +
         std::to_string(static_cast<long long>(line_no));
}

enum OpKind {
  kOpMark,
  kOpCommand,
  kOpActivate,
  kOpPointer,
};

struct Op {
  OpKind kind;
  std::string a;
  std::string b;
  InputEvent event;
};

bool ParsePointer(const std::string& rest, InputEvent* event, std::string* error,
                  int line_no) {
  std::vector<std::string> tokens;
  SplitTokens(rest, &tokens);
  if (tokens.size() < 4 || tokens.size() > 5) {
    SetError(error, LineError("pointer needs view phase x y", line_no));
    return false;
  }
  if (!is_known_view_id(tokens[0])) {
    SetError(error, LineError("pointer view id", line_no));
    return false;
  }
  int x = 0;
  int y = 0;
  if (!ParseInt(tokens[2], &x) || !ParseInt(tokens[3], &y)) {
    SetError(error, LineError("pointer coordinates", line_no));
    return false;
  }
  event->view_id = tokens[0];
  event->x = x;
  event->y = y;
  const std::string& phase = tokens[1];
  if (phase == "move") {
    event->kind = kInputEventMouse;
    event->mouse_action = kMouseMove;
  } else if (phase == "down") {
    event->kind = kInputEventMouse;
    event->mouse_action = kMouseDown;
  } else if (phase == "up") {
    event->kind = kInputEventMouse;
    event->mouse_action = kMouseUp;
  } else if (phase == "wheel") {
    event->kind = kInputEventWheel;
    int delta = 120;
    if (tokens.size() == 5 && !ParseInt(tokens[4], &delta)) {
      SetError(error, LineError("pointer wheel delta", line_no));
      return false;
    }
    event->wheel_delta = delta;
  } else {
    SetError(error, LineError("pointer phase", line_no));
    return false;
  }
  return true;
}

bool ParseCommand(const std::string& rest,
                  std::string* id,
                  std::string* view_id,
                  std::string* error,
                  int line_no,
                  const char* verb) {
  std::vector<std::string> tokens;
  SplitTokens(rest, &tokens);
  if (tokens.size() != 2 || tokens[0].empty() || !is_known_view_id(tokens[1])) {
    SetError(error, LineError(std::string(verb) + " needs id and view", line_no));
    return false;
  }
  *id = tokens[0];
  *view_id = tokens[1];
  return true;
}

}  // namespace

bool run_suite_text(CapabilityHost& host,
                    const std::string& text,
                    std::string* error) {
  std::vector<Op> ops;
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
      Op op;
      if (verb == "mark") {
        if (rest.empty()) {
          SetError(error, LineError("mark requires a name", line_no));
          return false;
        }
        op.kind = kOpMark;
        op.a = rest;
      } else if (verb == "command" || verb == "activate_tool") {
        if (verb == "command" && !host.has_command_slot()) {
          SetError(error, LineError("command slot not installed", line_no));
          return false;
        }
        if (verb == "activate_tool" && !host.has_activate_slot()) {
          SetError(error, LineError("activate slot not installed", line_no));
          return false;
        }
        if (!ParseCommand(rest, &op.a, &op.b, error, line_no, verb.c_str()))
          return false;
        op.kind = verb == "command" ? kOpCommand : kOpActivate;
      } else if (verb == "pointer") {
        if (!host.has_pointer_slot()) {
          SetError(error, LineError("pointer slot not installed", line_no));
          return false;
        }
        if (!ParsePointer(rest, &op.event, error, line_no))
          return false;
        op.kind = kOpPointer;
      } else if (verb == "expect_camera" || verb == "expect_layer" ||
                 verb == "expect_entity" || verb == "capture") {
        SetError(error, LineError(verb + " is not available in this slice", line_no));
        return false;
      } else {
        SetError(error, LineError("unknown verb: " + verb, line_no));
        return false;
      }
      ops.push_back(op);
    }

    if (!has_newline)
      break;
    index = end + 1;
    ++line_no;
  }

  for (size_t i = 0; i < ops.size(); ++i) {
    const Op& op = ops[i];
    if (op.kind == kOpMark) {
      host.mark(op.a);
    } else if (op.kind == kOpCommand) {
      if (!host.RunCommand(op.a, op.b)) {
        SetError(error, "command failed: " + op.a);
        return false;
      }
    } else if (op.kind == kOpActivate) {
      if (!host.RunActivate(op.a, op.b)) {
        SetError(error, "activate_tool failed: " + op.a);
        return false;
      }
    } else if (!host.RunPointer(op.event)) {
      SetError(error, "pointer failed");
      return false;
    }
  }
  ClearError(error);
  return true;
}

}  // namespace content
