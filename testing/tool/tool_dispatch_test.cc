// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "tool/workspace.h"

#include "gtest/gtest.h"

namespace {

class CountingHandler : public tool::CommandHandler {
 public:
  CountingHandler() : calls(0) {}
  bool execute() override {
    ++calls;
    return true;
  }
  int calls;
};

class Probe : public tool::Interaction {
 public:
  Probe() : wheels(0), moves(0), in_input_(false) {}

  const char* id() const override { return "probe"; }

  bool on_input(const tool::InputEvent& event) override {
    in_input_ = true;
    if (event.kind == tool::kWheel)
      ++wheels;
    if (event.kind == tool::kMouse && event.action == tool::kMouseMove)
      ++moves;
    in_input_ = false;
    return false;
  }

  bool in_input() const override { return in_input_; }

  int wheels;
  int moves;

 private:
  bool in_input_;
};

class ProbeFactory : public tool::InteractionFactory {
 public:
  explicit ProbeFactory(Probe** slot) : slot_(slot) {}

  std::unique_ptr<tool::Interaction> Create() const override {
    Probe* probe = new Probe();
    if (slot_)
      *slot_ = probe;
    return std::unique_ptr<tool::Interaction>(probe);
  }

 private:
  Probe** slot_;
};

class RecordingSink : public tool::EditSink {
 public:
  RecordingSink() : commits(0), clears(0), called_during_input(false), watch(nullptr) {}

  void commit_selection() override {
    ++commits;
    if (watch && watch->in_input())
      called_during_input = true;
  }

  void clear_selection() override { ++clears; }

  int commits;
  int clears;
  bool called_during_input;
  tool::Interaction* watch;
};

tool::InputEvent MakeMouse(tool::MouseAction action, double x, double y) {
  tool::InputEvent event;
  event.kind = tool::kMouse;
  event.action = action;
  event.x_px = x;
  event.y_px = y;
  event.view_id = "map";
  return event;
}

}  // namespace

TEST(ToolDispatch, UnknownCommandDoesNotCallHandler) {
  tool::CommandCatalog catalog;
  CountingHandler* handler = new CountingHandler();
  ASSERT_TRUE(catalog.add(
      "view.refresh", std::unique_ptr<tool::CommandHandler>(handler)));
  EXPECT_TRUE(catalog.contains("view.refresh"));
  EXPECT_EQ(handler, catalog.find("view.refresh"));

  CountingHandler* duplicate = new CountingHandler();
  EXPECT_FALSE(catalog.add(
      "view.refresh", std::unique_ptr<tool::CommandHandler>(duplicate)));
  EXPECT_FALSE(catalog.add(
      "", std::unique_ptr<tool::CommandHandler>(new CountingHandler())));
  EXPECT_TRUE(catalog.find("missing.command") == nullptr);

  tool::CommandDispatcher dispatcher(&catalog);
  EXPECT_FALSE(dispatcher.execute("missing.command"));
  EXPECT_EQ(0, handler->calls);
  EXPECT_TRUE(dispatcher.execute("view.refresh"));
  EXPECT_EQ(1, handler->calls);

  tool::Workspace workspace("map");
  EXPECT_FALSE(workspace.execute("missing.command"));
  EXPECT_TRUE(workspace.stack().current() == nullptr);
  EXPECT_FALSE(workspace.selection_requested());
  EXPECT_FALSE(workspace.catalog().contains("wheel.zoom"));
  EXPECT_FALSE(workspace.catalog().contains("hover.cursor"));
}

TEST(ToolDispatch, WheelZoomConsumesWheelBeforeExclusiveTool) {
  tool::Workspace workspace("map");
  Probe* probe = nullptr;
  ASSERT_TRUE(workspace.registry().add(
      "probe", std::unique_ptr<tool::InteractionFactory>(new ProbeFactory(&probe))));
  ASSERT_TRUE(workspace.stack().activate("probe"));
  ASSERT_TRUE(probe != nullptr);

  tool::InputEvent wheel;
  wheel.kind = tool::kWheel;
  wheel.wheel_dy = -1.0;
  wheel.view_id = "map";

  EXPECT_TRUE(workspace.always_on("wheel.zoom")->on_input(wheel));
  EXPECT_TRUE(workspace.dispatch(wheel));
  EXPECT_EQ(0, probe->wheels);
}

TEST(ToolDispatch, HoverCursorDoesNotConsumeMove) {
  tool::Workspace workspace("map");
  Probe* probe = nullptr;
  ASSERT_TRUE(workspace.registry().add(
      "probe", std::unique_ptr<tool::InteractionFactory>(new ProbeFactory(&probe))));
  ASSERT_TRUE(workspace.stack().activate("probe"));
  ASSERT_TRUE(probe != nullptr);

  tool::InputEvent move = MakeMouse(tool::kMouseMove, 8.0, 9.0);
  EXPECT_FALSE(workspace.always_on("hover.cursor")->on_input(move));
  EXPECT_FALSE(workspace.dispatch(move));
  EXPECT_EQ(1, probe->moves);
}

TEST(ToolDispatch, SelectionPointSinkRunsAfterGesture) {
  RecordingSink sink;
  tool::Workspace workspace("map");
  workspace.set_edit_sink(&sink);

  ASSERT_TRUE(workspace.execute("selection.point"));
  EXPECT_TRUE(workspace.selection_requested());
  tool::Interaction* selection = workspace.stack().current();
  ASSERT_TRUE(selection != nullptr);
  EXPECT_STREQ("selection.point", selection->id());
  sink.watch = selection;

  EXPECT_TRUE(workspace.dispatch(MakeMouse(tool::kMousePress, 3.0, 4.0)));
  EXPECT_TRUE(selection->has_draft());
  EXPECT_FALSE(selection->has_result());
  EXPECT_EQ(0, sink.commits);
  EXPECT_FALSE(sink.called_during_input);

  EXPECT_TRUE(workspace.dispatch(MakeMouse(tool::kMouseRelease, 5.0, 6.0)));
  EXPECT_TRUE(selection->has_result());
  EXPECT_FALSE(selection->in_input());
  EXPECT_EQ(1, sink.commits);
  EXPECT_FALSE(sink.called_during_input);

  tool::Workspace edit_workspace("view0");
  edit_workspace.set_edit_sink(&sink);
  const int commits_before_edit = sink.commits;
  ASSERT_TRUE(edit_workspace.execute("edit.append.polygon"));
  tool::Interaction* edit = edit_workspace.stack().current();
  ASSERT_TRUE(edit != nullptr);
  EXPECT_TRUE(edit_workspace.dispatch(MakeMouse(tool::kMousePress, 1.0, 1.0)));
  EXPECT_TRUE(edit->has_draft());
  EXPECT_EQ(commits_before_edit, sink.commits);
  EXPECT_TRUE(edit_workspace.dispatch(MakeMouse(tool::kMouseRelease, 2.0, 2.0)));
  EXPECT_TRUE(edit->has_result());
  EXPECT_EQ(commits_before_edit, sink.commits);

  EXPECT_TRUE(workspace.execute("selection.clear"));
  EXPECT_EQ(1, sink.clears);
  EXPECT_STREQ("selection.point", workspace.stack().current()->id());
}

TEST(ToolDispatch, TrackballActivatesOnlyOnVolumeViews) {
  tool::Workspace map_workspace("map");
  EXPECT_FALSE(map_workspace.execute("view3d.trackball"));
  EXPECT_TRUE(map_workspace.stack().current() == nullptr);
  EXPECT_FALSE(map_workspace.stack().activate("view3d.trackball"));
  EXPECT_FALSE(map_workspace.execute("view3d.fps"));
  EXPECT_FALSE(map_workspace.stack().activate("view3d.fps"));

  tool::Workspace view0("view0");
  EXPECT_TRUE(view0.execute("view3d.trackball"));
  ASSERT_TRUE(view0.stack().current() != nullptr);
  EXPECT_STREQ("view3d.trackball", view0.stack().current()->id());

  tool::Workspace view1("view1");
  EXPECT_TRUE(view1.stack().activate("view3d.fps"));
  ASSERT_TRUE(view1.stack().current() != nullptr);
  EXPECT_STREQ("view3d.fps", view1.stack().current()->id());
}
