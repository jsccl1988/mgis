// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include <string>
#include <vector>

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

#include "base/at_exit.h"
#include "gtest/gtest.h"

#include "content/browser/devtools/mcp_server.h"
#include "content/browser/glang.runtime/exec/suite_runner.h"
#include "content/public/debug_agent.h"
#include "content/public/gis_contents.h"
#include "content/public/tool_session.h"
#include "content/public/view_host.h"

namespace {

class GestureWatch : public content::EventBus::Observer {
 public:
  int count;
  content::GestureApplied last;

  GestureWatch() : count(0) {}

  void on_gesture_applied(const content::GestureApplied& event) override {
    ++count;
    last = event;
  }
};

class HostWatch : public content::ViewHost::Delegate {
 public:
  explicit HostWatch(content::ToolSession* tools) : tools(tools), count(0) {}

  bool OnChildGesture(const content::InputEvent& event) override {
    ++count;
    return tools->dispatch_input(event);
  }

  content::ToolSession* tools;
  int count;
};

HWND MakeParent() {
  return CreateWindowExW(0, L"STATIC", L"mview-session-wire", WS_OVERLAPPED,
                         0, 0, 200, 120, NULL, NULL, GetModuleHandleW(NULL),
                         NULL);
}

bool PostMcp(int port, const std::string& body, std::string* response) {
  WSADATA wsa;
  if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    return false;
  SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (sock == INVALID_SOCKET)
    return false;
  sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  addr.sin_port = htons(static_cast<u_short>(port));
  if (connect(sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) != 0) {
    closesocket(sock);
    return false;
  }
  std::string bytes = "POST /mcp HTTP/1.1\r\nHost: 127.0.0.1\r\n"
                      "Content-Type: application/json\r\nContent-Length: ";
  bytes += std::to_string(body.size());
  bytes += "\r\nConnection: close\r\n\r\n";
  bytes += body;
  int sent = 0;
  while (sent < static_cast<int>(bytes.size())) {
    const int n = send(sock, bytes.data() + sent,
                       static_cast<int>(bytes.size()) - sent, 0);
    if (n <= 0) {
      closesocket(sock);
      return false;
    }
    sent += n;
  }
  std::string data;
  char buf[1024];
  for (;;) {
    const int n = recv(sock, buf, sizeof(buf), 0);
    if (n <= 0)
      break;
    data.append(buf, buf + n);
  }
  closesocket(sock);
  const size_t split = data.find("\r\n\r\n");
  if (split == std::string::npos)
    return false;
  *response = data.substr(split + 4);
  return true;
}

std::string JsonTextField(const std::string& json) {
  const std::string key = "\"text\":\"";
  size_t pos = json.find(key);
  if (pos == std::string::npos)
    return std::string();
  pos += key.size();
  std::string out;
  while (pos < json.size() && json[pos] != '"') {
    if (json[pos] == '\\' && pos + 1 < json.size()) {
      ++pos;
      if (json[pos] == 'n')
        out.push_back('\n');
      else if (json[pos] == 'r')
        out.push_back('\r');
      else if (json[pos] == 't')
        out.push_back('\t');
      else
        out.push_back(json[pos]);
    } else {
      out.push_back(json[pos]);
    }
    ++pos;
  }
  return out;
}

std::string ReadDiscoveryFile() {
  wchar_t local_app[MAX_PATH];
  const DWORD n =
      GetEnvironmentVariableW(L"LOCALAPPDATA", local_app, MAX_PATH);
  if (n == 0 || n >= MAX_PATH)
    return std::string();
  const std::wstring path =
      std::wstring(local_app) + L"\\MView\\debug-agent.json";
  HANDLE file = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL,
                            OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
  if (file == INVALID_HANDLE_VALUE)
    return std::string();
  char buf[512];
  DWORD read = 0;
  std::string text;
  while (ReadFile(file, buf, sizeof(buf), &read, NULL) && read > 0)
    text.append(buf, buf + read);
  CloseHandle(file);
  return text;
}

void DeleteDiscoveryFile() {
  wchar_t local_app[MAX_PATH];
  const DWORD n =
      GetEnvironmentVariableW(L"LOCALAPPDATA", local_app, MAX_PATH);
  if (n == 0 || n >= MAX_PATH)
    return;
  const std::wstring path =
      std::wstring(local_app) + L"\\MView\\debug-agent.json";
  DeleteFileW(path.c_str());
}

int FailMcpBind(SOCKET, const sockaddr*, int) {
  return SOCKET_ERROR;
}

void EnsureAtExit() {
  // MessageLoop::current() registers a lazy TLS slot. Without an
  // AtExitManager that path logs and deadlocks in this test process.
  static base::AtExitManager* manager = new base::AtExitManager();
  (void)manager;
}

}  // namespace

TEST(SessionWire, WheelIsConsumedAndPublished) {
  content::GisContents contents;
  content::ToolSession tools;
  tools.set_event_bus(contents.event_bus());
  tools.set_trace_log(contents.trace_log());
  GestureWatch watch;
  contents.event_bus()->add_observer(&watch);

  content::InputEvent event;
  event.kind = content::kInputEventWheel;
  event.view_id = content::kViewIdMap;
  event.wheel_delta = 120;
  EXPECT_TRUE(tools.dispatch_input(event));
  EXPECT_EQ(1, watch.count);
  EXPECT_TRUE(watch.last.consumed);

  std::vector<std::string> perf;
  contents.trace_log()->Snapshot(content::kTracePerf, &perf);
  ASSERT_FALSE(perf.empty());
  EXPECT_NE(std::string::npos, perf[0].find("dispatch_input"));
  contents.event_bus()->remove_observer(&watch);
}

TEST(SessionWire, TrackballConsumesView0MouseOnly) {
  content::ToolSession tools;
  content::GisContents contents;
  tools.set_event_bus(contents.event_bus());
  GestureWatch watch;
  contents.event_bus()->add_observer(&watch);

  EXPECT_FALSE(tools.activate("view3d.trackball", content::kViewIdMap));

  content::InputEvent move;
  move.kind = content::kInputEventMouse;
  move.mouse_action = content::kMouseMove;
  move.view_id = content::kViewIdView0;
  move.x = 4;
  move.y = 8;
  EXPECT_FALSE(tools.dispatch_input(move));
  EXPECT_FALSE(watch.last.consumed);

  EXPECT_TRUE(tools.activate("view3d.trackball", content::kViewIdView0));
  EXPECT_TRUE(tools.dispatch_input(move));
  EXPECT_TRUE(watch.last.consumed);
  EXPECT_EQ(content::kViewIdView0, watch.last.view_id);
  contents.event_bus()->remove_observer(&watch);
}

TEST(SessionWire, MapLibreDoesNotAttach) {
  HWND parent = MakeParent();
  ASSERT_TRUE(parent != NULL);
  content::ViewHost host;
  content::GisContents contents;
  host.set_trace(contents.trace_log());
  EXPECT_FALSE(host.Attach(parent, content::kViewIdMap,
                           content::kViewBackendMapLibre, 80, 60));
  EXPECT_TRUE(host.child_hwnd() == NULL);
  std::vector<std::string> lines;
  contents.trace_log()->Snapshot(content::kTraceConsole, &lines);
  ASSERT_FALSE(lines.empty());
  EXPECT_NE(std::string::npos, lines[0].find("backend.unavailable"));
  DestroyWindow(parent);
}

TEST(SessionWire, PlaceholderForwardsGesture) {
  HWND parent = MakeParent();
  ASSERT_TRUE(parent != NULL);
  content::GisContents contents;
  content::ToolSession tools;
  tools.set_event_bus(contents.event_bus());
  tools.set_trace_log(contents.trace_log());
  HostWatch watch(&tools);
  content::ViewHost host;
  host.set_delegate(&watch);
  host.set_trace(contents.trace_log());
  ASSERT_TRUE(host.Attach(parent, content::kViewIdView0,
                          content::kViewBackendFlyCube, 80, 60));
  ASSERT_TRUE(host.child_hwnd() != NULL);
  SendMessageW(reinterpret_cast<HWND>(host.child_hwnd()), WM_LBUTTONDOWN, 0,
               MAKELPARAM(3, 5));
  EXPECT_EQ(1, watch.count);
  EXPECT_TRUE(tools.has_last_input());
  EXPECT_EQ(3, tools.last_input().x);
  EXPECT_EQ(content::kMouseDown, tools.last_input().mouse_action);
  host.Detach();
  DestroyWindow(parent);
}

TEST(SessionWire, UnknownVerbWritesNoMarks) {
  content::CapabilityHost host;
  std::string error;
  EXPECT_FALSE(content::run_suite_text(host, "mark ready\nnope\n", &error));
  EXPECT_TRUE(host.marks().empty());
  EXPECT_NE(std::string::npos, error.find("unknown verb"));
}

TEST(SessionWire, AgentCommandAndClosedVerb) {
  content::GisContents contents;
  content::ToolSession tools;
  tools.set_event_bus(contents.event_bus());
  tools.set_trace_log(contents.trace_log());
  content::DebugAgent agent;
  agent.Bind(&tools, contents.trace_log(), NULL, NULL);

  const std::string help = agent.ExecLine("help");
  EXPECT_NE(std::string::npos, help.find("activate_tool"));

  EXPECT_EQ("ok", agent.ExecLine("command view.pan map"));
  EXPECT_EQ("view.pan", tools.last_command_id());
  EXPECT_EQ(content::kViewIdMap, tools.last_command_view_id());

  const std::string closed = agent.ExecLine("expect_camera view0");
  EXPECT_NE(std::string::npos, closed.find("not available"));

  std::vector<std::string> agent_lines;
  contents.trace_log()->Snapshot(content::kTraceAgent, &agent_lines);
  EXPECT_GE(agent_lines.size(), 3u);
}

TEST(SessionWire, OnlySceneViewAttaches) {
  HWND parent = MakeParent();
  ASSERT_TRUE(parent != NULL);
  content::GisContents contents;
  EXPECT_FALSE(contents.AttachView(parent, content::kViewIdMap, 80, 60));
  EXPECT_FALSE(contents.AttachView(parent, content::kViewIdView0, 80, 60));
  EXPECT_TRUE(contents.AttachView(parent, content::kViewIdScene, 80, 60));
  EXPECT_EQ(content::SceneMode::kScene2D, contents.scene_mode());
  contents.SetSceneMode(content::SceneMode::kScene3D);
  EXPECT_EQ(content::SceneMode::kScene3D, contents.scene_mode());
  std::vector<std::string> lines;
  contents.trace_log()->Snapshot(content::kTraceConsole, &lines);
  EXPECT_NE(std::string::npos, lines[0].find("backend.unavailable map"));
  DestroyWindow(parent);
}

TEST(SessionWire, McpHelpMatchesExecLine) {
  content::GisContents contents;
  content::ToolSession tools;
  content::DebugAgent agent;
  agent.Bind(&tools, contents.trace_log(), NULL, NULL);
  const std::string help = agent.ExecLine("help");
  DeleteDiscoveryFile();
  EnsureAtExit();
  ASSERT_TRUE(agent.Start());
  const int port = agent.port();
  EXPECT_GT(port, 0);
  const std::string discovery = ReadDiscoveryFile();
  EXPECT_NE(std::string::npos,
            discovery.find("\"port\":" + std::to_string(port)));
  EXPECT_EQ(std::string::npos, discovery.find("\"port\":0"));
  EXPECT_NE(std::string::npos,
            discovery.find("http://127.0.0.1:" + std::to_string(port) + "/mcp"));

  const std::string body =
      "{\"jsonrpc\":\"2.0\",\"id\":1,\"method\":\"tools/call\","
      "\"params\":{\"name\":\"help\"}}";
  std::string response;
  ASSERT_TRUE(PostMcp(port, body, &response));
  EXPECT_EQ(help, JsonTextField(response));
  EXPECT_NE(std::string::npos, JsonTextField(response).find("activate_tool"));
  agent.Stop();
  std::string after_stop;
  EXPECT_FALSE(PostMcp(port, body, &after_stop));
}

TEST(SessionWire, McpUnknownToolWritesNoMarks) {
  content::GisContents contents;
  content::ToolSession tools;
  content::DebugAgent agent;
  agent.Bind(&tools, contents.trace_log(), NULL, NULL);
  EnsureAtExit();
  ASSERT_TRUE(agent.Start());
  const std::string body =
      "{\"jsonrpc\":\"2.0\",\"id\":7,\"method\":\"tools/call\","
      "\"params\":{\"name\":\"nope\",\"arguments\":{\"name\":\"leaked\"}}}";
  std::string response;
  ASSERT_TRUE(PostMcp(agent.port(), body, &response));
  EXPECT_NE(std::string::npos, response.find("\"error\""));
  EXPECT_EQ(std::string::npos, response.find("\"result\""));
  EXPECT_FALSE(agent.has_mark("leaked"));
  EXPECT_TRUE(agent.marks().empty());
  agent.Stop();
}

TEST(SessionWire, McpBindFailureKeepsExecLine) {
  content::GisContents contents;
  content::ToolSession tools;
  content::DebugAgent agent;
  agent.Bind(&tools, contents.trace_log(), NULL, NULL);
  const std::string help = agent.ExecLine("help");
  DeleteDiscoveryFile();
  EnsureAtExit();
  content::SetMcpBindHookForTest(&FailMcpBind);
  EXPECT_FALSE(agent.Start());
  content::SetMcpBindHookForTest(NULL);
  EXPECT_EQ(help, agent.ExecLine("help"));
  std::vector<std::string> lines;
  contents.trace_log()->Snapshot(content::kTraceConsole, &lines);
  ASSERT_FALSE(lines.empty());
  EXPECT_NE(std::string::npos, lines.back().find("mcp.bind failed"));
  const std::string discovery = ReadDiscoveryFile();
  EXPECT_EQ(std::string::npos, discovery.find("\"port\":0"));
  EXPECT_TRUE(discovery.empty());
}

TEST(SessionWire, McpExecLineToolMatchesExecLine) {
  content::GisContents contents;
  content::ToolSession tools;
  content::DebugAgent agent;
  agent.Bind(&tools, contents.trace_log(), NULL, NULL);
  const std::string help = agent.ExecLine("help");
  const std::string command = agent.ExecLine("command view.pan map");
  EnsureAtExit();
  ASSERT_TRUE(agent.Start());
  std::string response;
  const std::string help_body =
      "{\"jsonrpc\":\"2.0\",\"id\":2,\"method\":\"tools/call\","
      "\"params\":{\"name\":\"exec_line\",\"arguments\":{\"line\":\"help\"}}}";
  ASSERT_TRUE(PostMcp(agent.port(), help_body, &response));
  EXPECT_EQ(help, JsonTextField(response));
  const std::string command_body =
      "{\"jsonrpc\":\"2.0\",\"id\":3,\"method\":\"tools/call\","
      "\"params\":{\"name\":\"command\",\"arguments\":{\"id\":\"view.pan\","
      "\"view\":\"map\"}}}";
  ASSERT_TRUE(PostMcp(agent.port(), command_body, &response));
  EXPECT_EQ(command, JsonTextField(response));
  EXPECT_EQ("view.pan", tools.last_command_id());
  agent.Stop();
}
