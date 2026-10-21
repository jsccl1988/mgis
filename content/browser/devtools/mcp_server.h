// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#ifndef CONTENT_BROWSER_DEVTOOLS_MCP_SERVER_H_
#define CONTENT_BROWSER_DEVTOOLS_MCP_SERVER_H_

#include <winsock2.h>

#include <atomic>
#include <string>
#include <thread>

namespace content {

// Replaces ::bind inside Start. Tests use this to force a bind failure.
typedef int (*McpBindFn)(SOCKET, const struct sockaddr*, int);
void SetMcpBindHookForTest(McpBindFn hook);

// Loopback MCP Streamable HTTP endpoint. Listens on 127.0.0.1 with port 0
// and dispatches tools/call lines through |line_fn| on the caller-supplied
// thread policy (the callback itself decides the UI thread).
class McpServer {
 public:
  typedef std::string (*LineFn)(void* context, const std::string& line);

  McpServer(LineFn line_fn, void* context);
  ~McpServer();

  // Binds 127.0.0.1:0, writes the discovery file, and accepts connections
  // until Stop. Returns false when the socket cannot bind. Does not call
  // |line_fn| for an unknown tool.
  bool Start();
  void Stop();

  int port() const;
  bool bind_failed() const;

 private:
  McpServer(const McpServer&) = delete;
  McpServer& operator=(const McpServer&) = delete;

  void Run();
  void Serve(SOCKET client);
  std::string HandleJson(const std::string& body) const;

  LineFn line_fn_;
  void* context_;
  SOCKET listen_;
  int port_;
  bool bind_failed_;
  std::atomic<bool> stopping_;
  std::thread thread_;
};

}  // namespace content

#endif  // CONTENT_BROWSER_DEVTOOLS_MCP_SERVER_H_
