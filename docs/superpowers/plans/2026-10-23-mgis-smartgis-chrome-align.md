<!--
Copyright (c) 2026 The Mogu Authors.
All rights reserved.
-->

# Mgis 壳对齐 SmartGIS chrome Implementation Plan

> **For agentic workers:** 本计划与 living §「Mgis 工作台壳」同步；用户授权一次落地 A′+C′。

**Goal:** `app/mgis` 主窗分区/Tab/工具条/状态栏对齐 SmartGIS Views 截图；工具条与 Plugins Run 接到 `ToolSession` / `PluginHost`；状态栏接 `EventBus`。

**Architecture:** 本仓重做 `horizon/*` 面板，不移植 smartgis 源码。`GisContents::ActivateSceneTool` 为壳侧工具激活入口（避免壳 `#include content/browser`）。

**Tech Stack:** Chromium `ui/views`、`build.bat debug mview_shell`。

**Spec:** `docs/superpowers/specs/2026-10-19-ue-mview-maplibre-design.md` § Mgis 工作台壳 · SmartGIS chrome 对齐。

---

### Task 1: 规格与图

- [x] 更新 living § 与 `mgis-workbench-chrome.html`

### Task 2: GisContents 工具入口

- [x] `ActivateSceneTool(interaction_id)` → `tool_session()->activate(..., kViewIdScene)`

### Task 3: horizon 面板

- [x] `map_toolbar` / `status_bar`
- [x] `catalog_tabs` → Layers/Sources/Maps
- [x] `tool_plugin_panel` → GroupBar + Plugins
- [x] `bottom_panel` → Output|Console|Trace|Memory|Agent

### Task 4: Workbench 组合与接线

- [x] Layout：toolbar + 四区 + status
- [x] 中心 Tab 标签 Map|3D；catalog 不再强制切视口
- [x] EventBus → status；TraceLog → Output；toolbar → ActivateSceneTool；plugins → run_processing

### Task 5: 编译

- [x] `build.bat debug mview_shell`
