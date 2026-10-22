// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include <string>
#include <vector>

#include <windows.h>

#include "gtest/gtest.h"

#include "content/browser/debug/debug_agent.h"
#include "content/browser/glang/exec/suite_runner.h"
#include "content/browser/script_runtime.h"
#include "content/browser/tool_session.h"
#include "content/browser/view_host.h"
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

TEST(SessionWire, ExpectLayerAndEntityStayExplicit) {
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
