// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "content/browser/agent/debug/debug_agent.h"

#include <vector>

#include "base/bind.h"
#include "base/message_loop.h"
#include "base/synchronization/waitable_event.h"
#include "content/browser/agent/devtools/mcp_server.h"
#include "content/browser/glang.runtime/exec/suite_runner.h"
#include "content/browser/tooling/tool_session.h"
#include "content/public/browser/view_host.h"
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

struct UiExec {
  DebugAgent* agent;
  const std::string* line;
  std::string* reply;
  base::WaitableEvent* done;
};

void RunExecOnUi(UiExec* req) {
  *req->reply = req->agent->ExecLine(*req->line);
  req->done->Signal();
}

}  // namespace

DebugAgent::DebugAgent()
    : tools_(NULL),
      trace_(NULL),
      view0_(NULL),
      parent_hwnd_(NULL),
      contents_(NULL),
      mcp_(NULL),
      ui_loop_(NULL) {
  host_.set_command_slot(&DebugAgent::OnCommand, this);
  host_.set_activate_slot(&DebugAgent::OnActivate, this);
  host_.set_pointer_slot(&DebugAgent::OnPointer, this);
  mcp_ = new McpServer(&DebugAgent::OnMcpLine, this);
}

DebugAgent::~DebugAgent() {
  Stop();
  delete mcp_;
  mcp_ = NULL;
}

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

CapabilityHost& DebugAgent::ActiveHost() {
  if (contents_)
    return contents_->capability_host();
  return host_;
}

const CapabilityHost& DebugAgent::ActiveHost() const {
  if (contents_)
    return contents_->capability_host();
  return host_;
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
        "pointer <view> <move|down|up|wheel> <x> <y> [delta] | "
        "scene_mode | expect_camera | expect_layer | expect_entity";
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
      if (!run_suite_script(ActiveHost(), rest, &error))
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
    if (!run_suite_text(ActiveHost(), Trim(line), &error))
      reply = error.empty() ? "rejected" : error;
    else
      reply = "ok";
  }
  if (trace_)
    trace_->Append(kTraceAgent, reply);
  return reply;
}

bool DebugAgent::Start() {
  ui_loop_ = base::MessageLoop::current();
  if (!mcp_ || !mcp_->Start()) {
    if (trace_ && mcp_ && mcp_->bind_failed())
      trace_->Append(kTraceConsole, "mcp.bind failed");
    return false;
  }
  return true;
}

void DebugAgent::Stop() {
  if (mcp_)
    mcp_->Stop();
}

int DebugAgent::port() const {
  return mcp_ ? mcp_->port() : 0;
}

bool DebugAgent::has_mark(const std::string& name) const {
  return ActiveHost().has_mark(name);
}

const std::vector<std::string>& DebugAgent::marks() const {
  return ActiveHost().marks();
}

std::string DebugAgent::OnMcpLine(void* context, const std::string& line) {
  DebugAgent* agent = static_cast<DebugAgent*>(context);
  if (!agent)
    return std::string();
  return agent->RunLineOnUi(line);
}

std::string DebugAgent::RunLineOnUi(const std::string& line) {
  // Tests have no message loop, so the accept thread calls ExecLine directly.
  // A live UI loop receives the call via PostTask before ExecLine runs.
  if (!ui_loop_)
    return ExecLine(line);
  if (base::MessageLoop::current() == ui_loop_)
    return ExecLine(line);
  base::WaitableEvent done(false, false);
  std::string reply;
  UiExec req;
  req.agent = this;
  req.line = &line;
  req.reply = &reply;
  req.done = &done;
  ui_loop_->PostTask(FROM_HERE, base::Bind(&RunExecOnUi, &req));
  done.Wait();
  return reply;
}

}  // namespace content
