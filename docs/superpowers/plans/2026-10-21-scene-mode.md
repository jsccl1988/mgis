<!--
Copyright (c) 2026 The Mogu Authors.
All rights reserved.
-->

# Scene 模式 Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 一个 `Scene` 用 `SceneMode` 切换投影，图元命中、行套件、harness 和本机 MCP 都走这条会话；测试只用 `FakeScene`。

**Architecture:** 壳只包含 `content/public/browser`。`GisContents` 挂接 `view_id == "scene"` 的一个子窗口。`SceneImpl` 保存相机、模式和图元，不包含 UE、MapLibre、FlyCube 头。`session_wire` 链接 `FakeScene`。glang 行解释器把 `scene_mode` / `pointer` / `command` 送进同一个 `ToolSession`。harness 按套件顺序写 `out/harness/<suite>/mark.json`。`DebugAgent` 在 `127.0.0.1` 上提供 MCP Streamable HTTP，工具在界面线程调用 `ExecLine`。

**Tech Stack:** C++（`cc_std`，MSVC `/std:c++23preview`），GN + ninja，`build.bat debug <target>`，gtest，Win32 HWND。

**Spec:** `docs/superpowers/specs/2026-10-19-ue-mview-maplibre-design.md` 的「Content 核心与领域组件」「场景模式」「呈现器分级」。

## Global Constraints

- 壳只 `#include "content/public/browser/…"`。不得包含 `content/browser`、UE 头、MapLibre 头、FlyCube 头。
- `content` 不得包含 `ui/views` 或 UE 头。`tool/` 不得包含 HWND、UE、MapLibre、FlyCube。
- 不建 `content/renderer`。不留 `content/public/*.h` 转发头。`glang.runtime` 迁到 `content/browser/glang/`。
- 这一刀不链 Cesium for Unreal、MapLibre Native、`cesium-flycube`。三者是后续呈现器计划，不在本文。
- `AttachView` 只接受 `"scene"`。`map` / `view0` / `view1` 失败且不留下子 HWND。
- `SetSceneMode` 立即切换，不播放形变动画。图元列表保持。
- 命中 mark：`primitive:basemap:m0`、`primitive:groundwater:g0`。
- 未知套件动词丢掉该文件全部 mark。
- MCP 只绑定 `127.0.0.1`。发现文件 `%LOCALAPPDATA%\MView\debug-agent.json` 里的 `port` 是绑定后的真实端口。
- 测试入口：`build.bat debug session_wire` 与 `build.bat debug harness_campaign`。
- 注释用英文。不要提交 `Co-authored-by: Cursor`。

## 本计划不实现

`content/browser/map/present/` 下的三个呈现器：UE 管道里的 Cesium for Unreal（`SCENE3D` 标配）、MapLibre（`SCENE2D` 标配）、`cesium-flycube`（`SCENE3D` 平替，`presenter.fallback flycube`）。`COLUMBUS_VIEW` 的呈现器可拒绝。对象模型和 `SetSceneMode` 在本计划落地，绘制仍是 `FakeScene`。

## 文件

| 路径 | 职责 |
| --- | --- |
| `content/public/browser/scene_mode.h` | `content::SceneMode` |
| `content/public/browser/gis_contents.h` | `AttachView` / `SetSceneMode` |
| `content/browser/gis_contents_impl.cc` | 会话持有 `FakeScene` |
| `content/browser/map/scene.h` | `Scene` 接口（相机、模式、拾取、草稿） |
| `content/browser/map/fake_scene.h` | 测试用场景。`SCENE2D` 命中 `m0`，`SCENE3D` 命中 `g0` |
| `content/browser/map/primitive.h` | 图元 id |
| `content/browser/glang/` | 自 `glang.runtime` 迁入 |
| `content/browser/devtools/mcp_server.h` | 本机 MCP |
| `testing/tools/harness/browser.map/` 等 | 套件正文与 `required_marks.txt` |
| `content/browser/harness_campaign_test.cc` | gtest 目标 `harness_campaign` |

---

### Task 1: `SceneMode` 与只接受 `scene` 的挂接

**Files:**
- Create: `content/public/browser/scene_mode.h`
- Modify: `content/public/gis_contents.h`（本任务仍用现路径；Task 2 再迁目录）
- Modify: `content/browser/gis_contents.cc`
- Create: `content/browser/map/fake_scene.h`
- Create: `content/browser/map/fake_scene.cc`
- Modify: `content/browser/session_wire_test.cc`
- Modify: `content/BUILD.gn`（把 `fake_scene.cc` 放进 `:content`）

**Interfaces:**
- Produces: `enum class content::SceneMode { kScene2D, kScene3D, kColumbusView }`
- Produces: `bool GisContents::AttachView(void* parent_hwnd, const std::string& view_id, int width, int height)`
- Produces: `void GisContents::SetSceneMode(SceneMode mode)`
- Produces: `SceneMode GisContents::scene_mode() const`
- Produces: `const char content::kViewIdScene[] = "scene"`

- [ ] **Step 1: 写失败测试**

在 `session_wire_test.cc` 增加：

```cpp
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
```

- [ ] **Step 2: 跑测试，确认失败**

Run: `build.bat debug session_wire`

Expected: 编译失败，`AttachView` 不存在。

- [ ] **Step 3: 最小实现**

`scene_mode.h`：

```cpp
#ifndef CONTENT_PUBLIC_BROWSER_SCENE_MODE_H_
#define CONTENT_PUBLIC_BROWSER_SCENE_MODE_H_

namespace content {

enum class SceneMode { kScene2D, kScene3D, kColumbusView };

}  // namespace content

#endif
```

`types.cc` 增加 `const char kViewIdScene[] = "scene";`。`is_known_view_id` 仍只认 `map` / `view0` / `view1`（工具 id）。`scene` 只用于挂接，不要塞进 `is_known_view_id`，否则旧的三视图命令表会被放宽。

`FakeScene::Attach` 在 `parent_hwnd == NULL` 或 `view_id != "scene"` 时返回 false，并 `trace->Append(kTraceConsole, "backend.unavailable " + view_id)`。成功时用 `CreateWindowExW` 建子 `STATIC`，默认模式 `kScene2D`。`SetSceneMode` 只改枚举并追加 `scene.mode scene2d|scene3d|columbus`。失败路径不得留下子 HWND。

`GisContents` 持有 `FakeScene`，三个方法转发。

- [ ] **Step 4: 测试通过**

Run: `build.bat debug session_wire`

Expected: `OnlySceneViewAttaches` 通过，原有 `SessionWire.*` 仍通过。

- [ ] **Step 5: Commit**

```bat
git add content/public/browser/scene_mode.h content/public/gis_contents.h content/public/types.h content/browser/gis_contents.cc content/browser/map/fake_scene.h content/browser/map/fake_scene.cc content/browser/session_wire_test.cc content/BUILD.gn
git commit -m "feat: attach a single scene view and switch SceneMode"
```

### Task 2: 公开头迁到 `content/public/browser`

**Files:**
- Move into `content/public/browser/`: `gis_contents.h`、`gis_contents_client.h`、`gis_document.h`、`event_bus.h`、`trace_log.h`、`plugin_host.h`、`types.h`、`scene_mode.h`
- Move beside `.cc` under `content/browser/`: `view_host.h`、`tool_session.h`、`script_runtime.h`、`debug_agent.h`（自 `content/public/`）
- Modify includes in `content/browser/**`、`app/shell/main_window.cc`、`app/mgis/workbench/workbench.h`、`app/mgis/workbench/workbench.cc`、`app/mgis/viewport/backend_hooks.cc`
- Delete: 旧的 `content/public/*.h`。不要留转发头。

**Interfaces:**
- Consumes: Task 1 的 `GisContents::AttachView`
- Produces: 壳的包含路径 `"content/public/browser/gis_contents.h"`

- [ ] **Step 1: 移动头并改 include guard**

公开头 guard 改为 `CONTENT_PUBLIC_BROWSER_GIS_CONTENTS_H_` 这一形。头内部互相包含改为 `content/public/browser/...`。

实现头 guard 改为 `CONTENT_BROWSER_VIEW_HOST_H_` 这一形。`app/mgis` 与 `app/shell` 不得再包含 `view_host.h`、`tool_session.h`、`script_runtime.h`、`debug_agent.h`。工作台若仍要子 HWND，只调用 `GisContents::AttachView`。`backend_hooks.cc` 里对 `ViewHost` 的引用改到 `content/browser` 侧能看见的调用，或在本任务删掉产品路径上的 `SetBackendHooks`（`GlobeView` 渲染检查可保留，但不得再被 `StartSession` 登记为 `view0` 后端）。

- [ ] **Step 2: 编译**

Run: `build.bat debug session_wire`

Expected: 通过。若 `//app/mgis:mgis` 已在默认图里，一并 `build.bat debug mgis`，壳编译不得包含 `content/browser`。

- [ ] **Step 3: Commit**

```bat
git add content app
git commit -m "refactor: move embedder headers under content/public/browser"
```

### Task 3: 模式决定导航，拾取写出图元 mark

**Files:**
- Create: `content/browser/map/scene.h`
- Modify: `content/browser/map/fake_scene.cc`
- Modify: `content/browser/tool_session.cc`
- Modify: `content/browser/session_wire_test.cc`

**Interfaces:**
- Consumes: `SceneMode`，`kViewIdScene`
- Produces: `FakeScene::Pick()` 在 `kScene2D` 返回 `primitive:basemap:m0`，在 `kScene3D` 返回 `primitive:groundwater:g0`，未命中返回空字符串
- Produces: `ToolSession::activate("view3d.trackball", "scene")` 仅在 `kScene3D` 为 true；`view.pan` 仅在 `kScene2D` 或 `kColumbusView` 为 true

- [ ] **Step 1: 写失败测试**

```cpp
TEST(SessionWire, ModeSelectsControllerAndPrimitive) {
  HWND parent = MakeParent();
  content::GisContents contents;
  ASSERT_TRUE(contents.AttachView(parent, "scene", 80, 60));
  content::ToolSession* tools = contents.tool_session();
  EXPECT_TRUE(tools->activate("view.pan", "scene"));
  EXPECT_FALSE(tools->activate("view3d.trackball", "scene"));
  EXPECT_EQ("primitive:basemap:m0", contents.Pick(1, 1));

  contents.SetSceneMode(content::SceneMode::kScene3D);
  EXPECT_FALSE(tools->activate("view.pan", "scene"));
  EXPECT_TRUE(tools->activate("view3d.trackball", "scene"));
  EXPECT_EQ("primitive:groundwater:g0", contents.Pick(1, 1));
  DestroyWindow(parent);
}
```

`GisContents::tool_session()` 返回会话内部的 `ToolSession*`，壳只在测试里用。不要把 `tool_session.h` 放回 `content/public`。测试是 `content/browser` 的翻译单元，可以包含 `content/browser/tool_session.h`。产品壳不要调用 `tool_session()`。若不想把该方法放进公开头，把测试友元留在 `gis_contents.h` 的 `private` 并用 `FRIEND_TEST`，公开头只保留 `Pick`。

- [ ] **Step 2: 跑测试，确认失败**

Run: `build.bat debug session_wire`

Expected: `Pick` 或 `tool_session` 未定义。

- [ ] **Step 3: 实现**

`FakeScene` 登记两个图元 id：`basemap`、`groundwater`。`Pick` 不看像素坐标，按模式返回上表（假场景的约定）。`SetSceneMode` 不清空图元。`view.zoom_in` / `view.zoom_out` 与 `view.pan` 同一模式条件。滚轮仍由现有 `wheel.zoom` 截断，不要在 `FakeScene` 里再消费滚轮。

- [ ] **Step 4: 测试通过**

Run: `build.bat debug session_wire`

Expected: 新测试与 `WheelIsConsumedAndPublished` 通过。

- [ ] **Step 5: Commit**

```bat
git add content/browser/map content/browser/tool_session.cc content/browser/session_wire_test.cc content/public/browser/gis_contents.h
git commit -m "feat: pick primitives and gate navigation on SceneMode"
```

### Task 4: 行套件动词进入 `ToolSession`

**Files:**
- Move: `content/browser/glang.runtime/` → `content/browser/glang/`（`language/`、`exec/`、`host/`）
- Modify: `content/browser/glang/exec/suite_runner.cc`
- Modify: `content/browser/debug/debug_agent.cc`
- Modify: `content/BUILD.gn` 源路径
- Modify: `content/browser/session_wire_test.cc`

**Interfaces:**
- Consumes: `GisContents::SetSceneMode`、`ToolSession::activate`、`FakeScene::Pick`
- Produces: 行 `scene_mode scene2d|scene3d|columbus`
- Produces: `expect_camera scene`：`kScene2D` 比较经纬度；`kScene3D` 要求轨道位移大于 0
- Produces: `attach` 只接受 `scene`

- [ ] **Step 1: 写失败测试**

```cpp
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
  DestroyWindow(parent);
}
```

`capability_host()` 与 `tool_session()` 同样只给 browser 测试用，不要放进壳能依赖的行为说明以外。测试文件在 `content/browser`，可以直接拿内部访问器。

- [ ] **Step 2: 跑测试，确认失败**

Run: `build.bat debug session_wire`

Expected: `scene_mode` 为未知动词，或 `run_suite_text` 因目录未迁而仍返回 unknown verb。

- [ ] **Step 3: 实现**

把 `glang.runtime` 目录改名为 `glang`，修正 `BUILD.gn` 与 include。`suite_runner.cc` 识别 `scene_mode`，调用绑定到 `GisContents` 的槽。`DebugAgent::ExecLine` 的 `attach` 只接受一个 token `scene`。`expect_camera` 在未挂接时返回错误字符串，不再用 `"not available"` 假装所有相机都未实现。`expect_layer` 与 `expect_entity` 仍返回明确错误。

`SCENE2D` 的相机比较：套件行 `expect_camera scene <lon> <lat> <height>`，绝对误差经纬度 `1e-6`、高度 `1`。`SCENE3D` 的行 `expect_camera scene` 无期望值，内部位移大于 0 即成功。假场景在 `view3d.trackball` 激活后的一次 `pointer scene move` 把位移设为 1。

- [ ] **Step 4: 测试通过**

Run: `build.bat debug session_wire`

Expected: 通过。全库不再出现路径 `glang.runtime`。

- [ ] **Step 5: Commit**

```bat
git add content
git commit -m "feat: drive SceneMode from glang suite lines"
```

### Task 5: `harness_campaign`

**Files:**
- Create: `testing/tools/harness/browser.map/suite.txt`
- Create: `testing/tools/harness/browser.map/required_marks.txt`
- Create: `testing/tools/harness/browser.pick/suite.txt`
- Create: `testing/tools/harness/browser.pick/required_marks.txt`
- Create: `testing/tools/harness/browser.draw/suite.txt`
- Create: `testing/tools/harness/browser.draw/required_marks.txt`
- Create: `content/browser/harness_campaign_test.cc`
- Modify: `content/BUILD.gn` 增加 `executable("harness_campaign")`

**Interfaces:**
- Consumes: `run_suite_script`、`FakeScene`、mark 名
- Produces: 顺序 `browser.map` → `browser.pick` → `browser.draw`
- Produces: 失败套件不写 `out/harness/<suite>/mark.json`，且不跑下一套

`browser.map/suite.txt`：

```text
attach scene
scene_mode scene2d
activate_tool view.pan scene
pointer scene move 1 1
expect_camera scene 0 0 0
mark pixel_ratio
```

`required_marks.txt` 一行：`pixel_ratio`。假场景的 `SCENE2D` 初始相机就是 `0,0,0`，`expect_camera` 因此通过。`browser.pick` 先 `scene2d` 再 `selection.point`，mark `primitive:basemap:m0`；再 `scene3d`，mark `primitive:groundwater:g0`。`browser.draw` 在两种模式下 `edit.append.point` 后提交，mark `draft.basemap` 与 `draft.groundwater`。

- [ ] **Step 1: 写失败测试**

```cpp
TEST(HarnessCampaign, StopsWhenMarkMissing) {
  // 用临时目录里一份缺少 pixel_ratio 的 browser.map 跑 campaign。
  // 断言 out/harness/browser.pick/mark.json 不存在。
}
```

测试把 `out` 指到测试临时目录，避免污染真实 `out/harness`。`HarnessCampaign::Run(const base::FilePath& suite_root, const base::FilePath& out_root)` 是新入口，放在 `content/browser/harness_campaign.cc`。

- [ ] **Step 2: 跑测试，确认失败**

Run: `build.bat debug harness_campaign`

Expected: 目标不存在，ninja 报错。

- [ ] **Step 3: 实现**

`Run` 按固定数组顺序读三套。每套调用 `run_suite_script`。缺任一 required mark 则返回 false，不写该套 `mark.json`，不打开下一套。通过则把 mark 名写成 JSON 数组。

- [ ] **Step 4: 测试通过**

Run: `build.bat debug harness_campaign`

Expected: `StopsWhenMarkMissing` 通过。再跑一次完整三套（测试 `HarnessCampaign.WritesMarksInOrder`），三个 `mark.json` 都存在。

- [ ] **Step 5: Commit**

```bat
git add content testing/tools/harness
git commit -m "test: run scene suites in harness_campaign order"
```

### Task 6: 本机 MCP

**Files:**
- Create: `content/browser/devtools/mcp_server.h`
- Create: `content/browser/devtools/mcp_server.cc`
- Modify: `content/browser/debug/debug_agent.cc`
- Modify: `content/browser/session_wire_test.cc`
- Modify: `content/BUILD.gn`

**Interfaces:**
- Consumes: `DebugAgent::ExecLine`
- Produces: `DebugAgent::Start()` 绑定 `127.0.0.1:0`，把真实端口写入 `%LOCALAPPDATA%\MView\debug-agent.json`
- Produces: `POST /mcp` 的 JSON-RPC `tools/call`，工具名 `help` 的文本与 `ExecLine("help")` 相同
- 工具在调用 `ExecLine` 之前投递到界面线程；测试里界面线程就是测试线程，用直接调用即可，但生产路径用 `MessageLoop` 的 `PostTask`

- [ ] **Step 1: 写失败测试**

```cpp
TEST(SessionWire, McpHelpMatchesExecLine) {
  content::DebugAgent agent;
  content::GisContents contents;
  content::ToolSession tools;
  agent.Bind(&tools, contents.trace_log(), NULL, NULL);
  ASSERT_TRUE(agent.Start());
  const int port = agent.port();
  EXPECT_GT(port, 0);
  const std::string body =
      "{\"jsonrpc\":\"2.0\",\"id\":1,\"method\":\"tools/call\","
      "\"params\":{\"name\":\"help\"}}";
  // Winsock 连接 127.0.0.1:port，发送 POST /mcp，body 为上一行。
  // 响应体里的 result.content[0].text 含有 "activate_tool"。
  agent.Stop();
}
```

请求体用上面的一行 JSON。响应 `result.content[0].text` 等于 `agent.ExecLine("help")` 在 Start 之前就能返回的那段帮助（帮助不依赖监听）。比较时先把期望值存下来再 `Start`，避免 `ExecLine` 被监听副作用改变。

- [ ] **Step 2: 跑测试，确认失败**

Run: `build.bat debug session_wire`

Expected: `Start` 不存在。

- [ ] **Step 3: 实现**

`socket` / `bind` / `listen`，地址 `htonl(INADDR_LOOPBACK)`，端口 0。`getsockname` 取出端口。发现文件 JSON：`{"port":<n>,"url":"http://127.0.0.1:<n>/mcp"}`。接受一个连接，读完 HTTP 头和 body，只处理 `tools/call`。`name` 为 `exec_line` 时 `arguments.line` 整行交给 `ExecLine`。`help`、`scene_mode`、`attach`、`pointer`、`command`、`activate_tool`、`expect_camera`、`capture`、`suite`、`mark`、`trace` 把 JSON 字段拼成与行语法相同的一行再调用 `ExecLine`。未知工具返回 JSON-RPC error，不写 mark。`bind` 失败时 `Start` 返回 false，`ExecLine` 仍可用，trace 写 `mcp.bind failed`。

- [ ] **Step 4: 测试通过**

Run: `build.bat debug session_wire`

Expected: `McpHelpMatchesExecLine` 通过。进程退出前 `Stop` 关掉监听套接字。

- [ ] **Step 5: Commit**

```bat
git add content/browser/devtools content/browser/debug content/browser/session_wire_test.cc content/BUILD.gn
git commit -m "feat: expose ExecLine as a loopback MCP endpoint"
```

### Task 7: 工作台用一个场景子窗口

**Files:**
- Modify: `app/mgis/workbench/workbench.cc`
- Modify: `app/mgis/workbench/workbench.h`

**Interfaces:**
- Consumes: `GisContents::AttachView`、`SetSceneMode`
- 壳不包含 `content/browser`

- [ ] **Step 1: 写失败测试**

在 `session_wire` 不覆盖壳。为 `mgis` 增加一个不链 GL 的 gtest 不现实时，用编译即约束：`workbench.cc` 不得出现 `SetBackendHooks`、`kViewBackendFlyCube`、`kViewIdView0`。

在 `content/browser/session_wire_test.cc` 已有挂接测试覆盖行为。本任务的检查是：

```bat
build.bat debug mgis
```

然后在 `app/mgis` 上搜索，`SetBackendHooks` 结果为空。

- [ ] **Step 2: 改 StartSession**

`StartSession` 对中心区父 HWND 调用一次 `AttachView(..., "scene", width, height)`。2D 页按钮或已有 `SelectViewTab` 在选中 2D 时 `SetSceneMode(kScene2D)`，选中 3D 时 `SetSceneMode(kScene3D)`。不要创建第二个子 HWND。`GlobeView` 文件可留在 `viewport/`，`StartSession` 不调用它。

- [ ] **Step 3: 编译**

Run: `build.bat debug mgis` 与 `build.bat debug session_wire`

Expected: 两者通过。

- [ ] **Step 4: Commit**

```bat
git add app/mgis
git commit -m "feat: drive the workbench center from one scene mode"
```

## 规格覆盖

| 规格要求 | 任务 |
| --- | --- |
| 一个 `scene` 子窗口，`map`/`view0`/`view1` 挂接失败 | Task 1 |
| `SCENE2D` / `SCENE3D` / `COLUMBUS_VIEW` 立即切换且图元还在 | Task 1、Task 3 |
| 公开头在 `content/public/browser`，壳不包含 `content/browser` | Task 2、Task 7 |
| `glang/` 行动词、未知动词丢 mark | Task 4 |
| `primitive:basemap:m0` 与 `primitive:groundwater:g0` | Task 3、Task 5 |
| harness 缺 mark 即停 | Task 5 |
| 本机 MCP，`help` 与 `ExecLine` 一致 | Task 6 |
| 工作台 2D/3D 页只改模式 | Task 7 |
| 三套呈现器、体素光线步进、形变动画、CPython | 不在本计划 |

## 自检

- 计划里的 `Pick` 返回值与规格 mark 一致。
- `is_known_view_id` 不把 `scene` 并进旧的三视图，避免 `view3d.trackball` 在 `map` 上变成功。
- 呈现器没有被写进 `session_wire` 的 deps。
