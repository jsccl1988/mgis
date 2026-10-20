<!--
Copyright (c) 2026 The Mogu Authors.
All rights reserved.
-->

# Mgis 工作台壳骨架 Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 落地 `app/mgis` 单主窗四区壳：左栏图层|实体 Tab、中心 2D|3D Tab、右工具/插件壳、底 console|trace|agent；双向联动；占位视口。

**Architecture:** chrome 面板与 `WorkbenchContents` / `SelectViewTab` 均在 `app/mgis`（`mgis` 命名空间；原 `ui/gis` 已并入）；`app/shell` 产品入口调 `mgis::ShowWorkbench`，harness/render-check 仍可挂卫星 ViewportWindow。

**Tech Stack:** Chromium `ui/views`（`TabbedPane`、`TreeView`、`Slider`）、GN、`build.bat debug mview_shell`。

**Spec:** living `docs/superpowers/specs/2026-10-19-ue-mview-maplibre-design.md` § Mgis 工作台壳；图 `docs/superpowers/diagrams/mgis-workbench-chrome.html`。

---

### Task 1: app/mgis + shell 接线

- [x] 创建 `app/mgis` 面板与 Workbench（曾短暂放在 `ui/gis`，已并入）
- [x] `shell::ShowMainWindow` → `mgis::ShowWorkbench`
- [x] harness 主窗改用 WorkbenchContents，保留卫星窗 mark
- [x] `build.bat debug mview_shell`
