// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include <string>
#include <vector>

#include <windows.h>

#include "gtest/gtest.h"

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
