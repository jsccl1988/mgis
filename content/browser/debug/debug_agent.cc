// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "content/browser/debug/debug_agent.h"

#include <vector>

#include "content/browser/glang/exec/suite_runner.h"
#include "content/browser/tool_session.h"
#include "content/browser/view_host.h"
#include "content/public/browser/gis_contents.h"
#include "content/public/browser/trace_log.h"

namespace content {
namespace {

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

}  // namespace

DebugAgent::DebugAgent()
    : tools_(NULL),
      trace_(NULL),
      view0_(NULL),
      parent_hwnd_(NULL),
      contents_(NULL) {
  host_.set_command_slot(&DebugAgent::OnCommand, this);
  host_.set_activate_slot(&DebugAgent::OnActivate, this);
  host_.set_pointer_slot(&DebugAgent::OnPointer, this);
}

DebugAgent::~DebugAgent() {}

void DebugAgent::Bind(ToolSession* tools,
                      TraceLog* trace,
                      ViewHost* view0,
                      void* parent_hwnd) {
  tools_ = tools;
  trace_ = trace;
  view0_ = view0;
  parent_hwnd_ = parent_hwnd;
}

void DebugAgent::set_contents(GisContents* contents) {
  contents_ = contents;
}

bool DebugAgent::OnCommand(void* context,
                           const std::string& command_id,
                           const std::string& view_id) {
  DebugAgent* agent = static_cast<DebugAgent*>(context);
  if (!agent || !agent->tools_)
    return false;
  return agent->tools_->execute(command_id, view_id);
}

bool DebugAgent::OnActivate(void* context,
                            const std::string& command_id,
                            const std::string& view_id) {
  DebugAgent* agent = static_cast<DebugAgent*>(context);
  if (!agent || !agent->tools_)
    return false;
  return agent->tools_->activate(command_id, view_id);
}

bool DebugAgent::OnPointer(void* context, const InputEvent& event) {
  DebugAgent* agent = static_cast<DebugAgent*>(context);
  if (!agent || !agent->tools_)
    return false;
  agent->tools_->dispatch_input(event);
  return true;
}

std::string DebugAgent::ExecLine(const std::string& line) {
  TraceScope scope(trace_, "debug.exec_line");
  std::string verb;
  std::string rest;
  std::string reply;
  if (!SplitVerb(line, &verb, &rest)) {
    reply = "empty";
  } else if (verb == "help") {
    reply =
        "help | trace | suite <path> | attach scene | "
        "mark | command <id> <view> | activate_tool <id> <view> | "
        "pointer <view> <move|down|up|wheel> <x> <y> [delta]";
  } else if (verb == "trace") {
    reply.clear();
    if (trace_) {
      std::vector<std::string> lines;
      trace_->Snapshot(kTraceConsole, &lines);
      for (size_t i = 0; i < lines.size(); ++i) {
        if (!reply.empty())
          reply += "\n";
        reply += lines[i];
      }
    }
    if (reply.empty())
      reply = "(no console lines)";
  } else if (verb == "suite") {
    if (rest.empty()) {
      reply = "suite needs a path";
    } else {
      std::string error;
      if (!run_suite_script(host_, rest, &error))
        reply = error.empty() ? "suite failed" : error;
      else
        reply = "suite ok";
    }
  } else if (verb == "attach") {
    std::vector<std::string> tokens;
    SplitTokens(rest, &tokens);
    if (tokens.size() != 1 || tokens[0] != kViewIdScene) {
      reply = "attach accepts only scene";
    } else if (!contents_) {
      reply = "scene host not bound";
    } else if (!parent_hwnd_) {
      reply = "parent hwnd missing";
    } else {
      const bool ok =
          contents_->AttachView(parent_hwnd_, kViewIdScene, 640, 480);
      reply = ok ? "attached" : "attach failed";
    }
  } else {
    std::string error;
    if (!run_suite_text(host_, Trim(line), &error))
      reply = error.empty() ? "rejected" : error;
    else
      reply = "ok";
  }
  if (trace_)
    trace_->Append(kTraceAgent, reply);
  return reply;
}

}  // namespace content
