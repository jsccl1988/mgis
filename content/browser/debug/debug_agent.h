// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#ifndef CONTENT_BROWSER_DEBUG_DEBUG_AGENT_H_
#define CONTENT_BROWSER_DEBUG_DEBUG_AGENT_H_

#include <string>

#include "content/browser/script_runtime.h"

namespace content {

class GisContents;
class ToolSession;
class TraceLog;
class ViewHost;

// In-process glang console. One session, main thread, no socket and no
// Python worker. ExecLine either runs a builtin or one suite line.
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
  void set_contents(GisContents* contents);

  // Returns a short UTF-8 reply. Also appends that reply to the agent channel
  // when a trace log is bound.
  std::string ExecLine(const std::string& line);

 private:
  DebugAgent(const DebugAgent&) = delete;
  DebugAgent& operator=(const DebugAgent&) = delete;

  static bool OnCommand(void* context,
                        const std::string& command_id,
                        const std::string& view_id);
  static bool OnActivate(void* context,
                         const std::string& command_id,
                         const std::string& view_id);
  static bool OnPointer(void* context, const InputEvent& event);

  ToolSession* tools_;
  TraceLog* trace_;
  ViewHost* view0_;
  void* parent_hwnd_;
  GisContents* contents_;
  CapabilityHost host_;
};

}  // namespace content

#endif  // CONTENT_BROWSER_DEBUG_DEBUG_AGENT_H_
