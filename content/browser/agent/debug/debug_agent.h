// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#ifndef CONTENT_BROWSER_AGENT_DEBUG_DEBUG_AGENT_H_
#define CONTENT_BROWSER_AGENT_DEBUG_DEBUG_AGENT_H_

#include <string>
#include <vector>

#include "content/browser/glang.runtime/script_runtime.h"

namespace base {
class MessageLoop;
}

namespace content {

class GisContents;
class McpServer;
class ToolSession;
class TraceLog;
class ViewHost;

// In-process glang console. ExecLine runs a builtin or one suite line.
// Start serves that console as MCP on 127.0.0.1 and writes the real port
// into the local discovery file. When set_contents is bound, suite lines
// use GisContents::capability_host() so scene / layer / entity slots apply.
class DebugAgent {
 public:
  DebugAgent();
  ~DebugAgent();

  // Unowned. Any pointer may be NULL; commands that need it then fail.
  void Bind(ToolSession* tools,
            TraceLog* trace,
            ViewHost* view0,
            void* parent_hwnd);

  // Unowned. attach scene uses this contents and the parent from Bind.
  // Suite / glang verbs then prefer contents->capability_host().
  void set_contents(GisContents* contents);

  // Returns a short UTF-8 reply. Also appends that reply to the agent channel
  // when a trace log is bound.
  std::string ExecLine(const std::string& line);

  // Binds 127.0.0.1:0. On failure ExecLine still works.
  bool Start();
  void Stop();
  int port() const;

  bool has_mark(const std::string& name) const;
  const std::vector<std::string>& marks() const;

 private:
  DebugAgent(const DebugAgent&) = delete;
  DebugAgent& operator=(const DebugAgent&) = delete;

  CapabilityHost& ActiveHost();
  const CapabilityHost& ActiveHost() const;

  static bool OnCommand(void* context,
                        const std::string& command_id,
                        const std::string& view_id);
  static bool OnActivate(void* context,
                         const std::string& command_id,
                         const std::string& view_id);
  static bool OnPointer(void* context, const InputEvent& event);
  static std::string OnMcpLine(void* context, const std::string& line);

  std::string RunLineOnUi(const std::string& line);

  ToolSession* tools_;
  TraceLog* trace_;
  ViewHost* view0_;
  void* parent_hwnd_;
  GisContents* contents_;
  // Fallback host when contents_ is NULL (command / activate / pointer only).
  CapabilityHost host_;
  McpServer* mcp_;
  base::MessageLoop* ui_loop_;
};

}  // namespace content

#endif  // CONTENT_BROWSER_AGENT_DEBUG_DEBUG_AGENT_H_
