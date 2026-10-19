// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#ifndef CONTENT_PUBLIC_TOOL_SESSION_H_
#define CONTENT_PUBLIC_TOOL_SESSION_H_

#include <string>

#include "content/public/types.h"

namespace content {

// Per-view command and input session. P0 stores the last call so the shell
// can link against a real object. When tool::Workspace exists, these methods
// forward there. This header does not include tool/ private headers.
class ToolSession {
 public:
  ToolSession();
  ~ToolSession();

  // Records |command_id| for |view_id|. Returns false, and leaves the last
  // command unchanged, when either string is empty or |view_id| is unknown.
  bool execute(const std::string& command_id, const std::string& view_id);

  // Records |event| and returns true. Input is not published on EventBus.
  bool dispatch_input(const InputEvent& event);

  // Records |interaction_id| for |view_id|. Returns false, and leaves the last
  // activation unchanged, when either string is empty or |view_id| is unknown.
  bool activate(const std::string& interaction_id, const std::string& view_id);

  const std::string& last_command_id() const;
  const std::string& last_command_view_id() const;
  bool has_last_input() const;
  const InputEvent& last_input() const;
  const std::string& last_interaction_id() const;
  const std::string& last_interaction_view_id() const;

 private:
  ToolSession(const ToolSession&) = delete;
  ToolSession& operator=(const ToolSession&) = delete;

  std::string last_command_id_;
  std::string last_command_view_id_;
  bool has_last_input_;
  InputEvent last_input_;
  std::string last_interaction_id_;
  std::string last_interaction_view_id_;
};

}  // namespace content

#endif  // CONTENT_PUBLIC_TOOL_SESSION_H_
