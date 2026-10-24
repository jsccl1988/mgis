// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include <string>
#include <vector>

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

#include "base/at_exit.h"
#include "gtest/gtest.h"

#include "content/browser/agent/debug/debug_agent.h"
#include "content/browser/agent/devtools/mcp_server.h"
#include "content/browser/glang.runtime/exec/suite_runner.h"
#include "content/browser/glang.runtime/script_runtime.h"
#include "content/browser/tooling/tool_session.h"
#include "content/public/browser/view_host.h"
#include "content/public/browser/gis_contents.h"

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

TEST(SessionWire, PySuiteMarksViaPlugin) {
  content::CapabilityHost host;
  std::string error;
#ifndef GLANG_PY_MARK_SUITE
#error GLANG_PY_MARK_SUITE must be defined by //content:session_wire
#endif
  const std::string path = GLANG_PY_MARK_SUITE;
#if defined(HAS_PYTHON)
  EXPECT_TRUE(content::run_suite_script(host, path, &error)) << error;
  EXPECT_TRUE(host.has_mark("py.ready"));
#else
  EXPECT_FALSE(content::run_suite_script(host, path, &error));
  EXPECT_TRUE(host.marks().empty());
  EXPECT_NE(std::string::npos, error.find("python"));
#endif
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
  EXPECT_EQ(std::string::npos, closed.find("not available"));
  EXPECT_NE(std::string::npos, closed.find("scene"));

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

TEST(SessionWire, ModeSelectsControllerAndPrimitive) {
  HWND parent = MakeParent();
  ASSERT_TRUE(parent != NULL);
  content::GisContents contents;
  ASSERT_TRUE(contents.AttachView(parent, "scene", 80, 60));
  content::ToolSession* tools = contents.tool_session();
  ASSERT_TRUE(tools != NULL);
  EXPECT_FALSE(content::is_known_view_id("scene"));
  EXPECT_FALSE(content::is_known_view_id(content::kViewIdScene));

  EXPECT_TRUE(tools->activate("view.pan", "scene"));
  EXPECT_TRUE(tools->activate("view.zoom_in", "scene"));
  EXPECT_TRUE(tools->activate("view.zoom_out", "scene"));
  EXPECT_FALSE(tools->activate("view3d.trackball", "scene"));
  EXPECT_EQ("primitive:basemap:m0", contents.Pick(1, 1));

  contents.SetSceneMode(content::SceneMode::kScene3D);
  EXPECT_FALSE(tools->activate("view.pan", "scene"));
  EXPECT_FALSE(tools->activate("view.zoom_in", "scene"));
  EXPECT_FALSE(tools->activate("view.zoom_out", "scene"));
  EXPECT_TRUE(tools->activate("view3d.trackball", "scene"));
  EXPECT_EQ("primitive:groundwater:g0", contents.Pick(1, 1));

  contents.SetSceneMode(content::SceneMode::kColumbusView);
  EXPECT_TRUE(tools->activate("view.pan", "scene"));
  EXPECT_TRUE(tools->activate("view.zoom_in", "scene"));
  EXPECT_TRUE(tools->activate("view.zoom_out", "scene"));
  EXPECT_FALSE(tools->activate("view3d.trackball", "scene"));
  EXPECT_EQ("", contents.Pick(1, 1));

  contents.SetSceneMode(content::SceneMode::kScene2D);
  EXPECT_EQ("primitive:basemap:m0", contents.Pick(9, 9));
  DestroyWindow(parent);
}

TEST(SessionWire, SelectionEmitsPickMarks) {
  HWND parent = MakeParent();
  ASSERT_TRUE(parent != NULL);
  content::GisContents contents;
  ASSERT_TRUE(contents.AttachView(parent, "scene", 80, 60));
  std::string error;
  EXPECT_TRUE(content::run_suite_text(
      contents.capability_host(),
      "activate_tool selection.point scene\n"
      "pointer scene down 4 5\n",
      &error))
      << error;
  EXPECT_TRUE(contents.capability_host().has_mark("pick.2d"));
  EXPECT_EQ("primitive:basemap:m0", contents.Pick(4, 5));

  EXPECT_TRUE(content::run_suite_text(
      contents.capability_host(),
      "scene_mode scene3d\n"
      "activate_tool selection.point scene\n"
      "pointer scene down 6 7\n",
      &error))
      << error;
  EXPECT_TRUE(contents.capability_host().has_mark("pick.3d"));
  EXPECT_EQ("primitive:groundwater:g0", contents.Pick(6, 7));
  DestroyWindow(parent);
}

TEST(SessionWire, SuiteSceneModeAndUnknownVerb) {
  content::GisContents contents;
  HWND parent = MakeParent();
  ASSERT_TRUE(contents.AttachView(parent, "scene", 64, 64));
  std::string error;
  EXPECT_TRUE(content::run_suite_text(
      contents.capability_host(),
      "scene_mode scene3d\nmark mode.ready\n", &error));
  EXPECT_EQ(content::SceneMode::kScene3D, contents.scene_mode());
  EXPECT_TRUE(contents.capability_host().has_mark("mode.ready"));

  EXPECT_FALSE(content::run_suite_text(
      contents.capability_host(), "mark gone\nnope\n", &error));
  EXPECT_FALSE(contents.capability_host().has_mark("gone"));
  EXPECT_TRUE(contents.capability_host().has_mark("mode.ready"));
  DestroyWindow(parent);
}

TEST(SessionWire, ExpectCameraSceneTolerances) {
  HWND parent = MakeParent();
  ASSERT_TRUE(parent != NULL);
  content::GisContents contents;
  ASSERT_TRUE(contents.AttachView(parent, "scene", 64, 64));
  std::string error;
  EXPECT_TRUE(content::run_suite_text(
      contents.capability_host(),
      "expect_camera scene 0 0 0\n"
      "expect_camera scene 0.000001 0 1\n",
      &error))
      << error;
  EXPECT_FALSE(content::run_suite_text(
      contents.capability_host(),
      "expect_camera scene 0.000002 0 0\n", &error));
  EXPECT_NE(std::string::npos, error.find("expect_camera"));
  EXPECT_FALSE(content::run_suite_text(
      contents.capability_host(), "expect_camera scene 0 0 1.1\n", &error));
  EXPECT_NE(std::string::npos, error.find("expect_camera"));

  EXPECT_FALSE(content::run_suite_text(
      contents.capability_host(),
      "scene_mode scene3d\nexpect_camera scene\n", &error));
  EXPECT_TRUE(content::run_suite_text(
      contents.capability_host(),
      "scene_mode scene3d\n"
      "activate_tool view3d.trackball scene\n"
      "pointer scene move 3 4\n"
      "expect_camera scene\n",
      &error))
      << error;
  DestroyWindow(parent);
}

TEST(SessionWire, AttachAcceptsOnlyScene) {
  HWND parent = MakeParent();
  ASSERT_TRUE(parent != NULL);
  content::GisContents contents;
  content::ToolSession tools;
  content::DebugAgent agent;
  agent.Bind(&tools, contents.trace_log(), NULL, parent);
  agent.set_contents(&contents);
  const std::string backends = agent.ExecLine("attach view0 flycube");
  EXPECT_NE(std::string::npos, backends.find("only scene"));
  EXPECT_NE(std::string::npos, agent.ExecLine("attach map").find("only scene"));
  EXPECT_EQ("attached", agent.ExecLine("attach scene"));
  EXPECT_EQ(content::SceneMode::kScene2D, contents.scene_mode());
  DestroyWindow(parent);
}

TEST(SessionWire, ExpectLayerAndEntityNeedSlotsWithoutContents) {
  content::DebugAgent agent;
  const std::string layer = agent.ExecLine("expect_layer basemap");
  const std::string entity = agent.ExecLine("expect_entity g0");
  EXPECT_FALSE(layer.empty());
  EXPECT_FALSE(entity.empty());
  EXPECT_NE(std::string::npos, layer.find("expect_layer"));
  EXPECT_NE(std::string::npos, entity.find("expect_entity"));
  EXPECT_EQ(std::string::npos, layer.find("ok"));
  EXPECT_EQ(std::string::npos, entity.find("ok"));
}

TEST(SessionWire, AgentExpectLayerAndEntityViaContentsHost) {
  content::GisContents contents;
  content::DebugAgent agent;
  agent.Bind(contents.tool_session(), contents.trace_log(), NULL, NULL);
  agent.set_contents(&contents);

  EXPECT_EQ("ok", agent.ExecLine("expect_layer basemap"));
  EXPECT_EQ("ok", agent.ExecLine("expect_layer groundwater"));
  EXPECT_NE(std::string::npos,
            agent.ExecLine("expect_layer nope").find("missing"));

  EXPECT_EQ("ok", agent.ExecLine("scene_mode scene2d"));
  EXPECT_EQ("ok", agent.ExecLine("expect_entity m0"));
  EXPECT_NE(std::string::npos,
            agent.ExecLine("expect_entity g0").find("missing"));

  EXPECT_EQ("ok", agent.ExecLine("scene_mode scene3d"));
  EXPECT_EQ("ok", agent.ExecLine("expect_entity g0"));
  EXPECT_NE(std::string::npos,
            agent.ExecLine("expect_entity m0").find("missing"));

  const std::string capture = agent.ExecLine("capture scene out.bmp");
  EXPECT_NE(std::string::npos, capture.find("not available"));
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
