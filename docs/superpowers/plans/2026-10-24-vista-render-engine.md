<!--
Copyright (c) 2026 The Mogu Authors.
All rights reserved.
-->

# Vista 渲染引擎层 Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 落地顶层 `vista/`，C++ `MapPass`/`WorldPass` 打通 MapLibre 与 cesium_flycube，删除对外 C ABI，壳改接线。

**Architecture:** `content` → `vista::MapPass` / `vista::WorldPass`；实现从 `gis/map` 与 `render/rhi/flycube` 迁入 `vista/pass/{map,world}`；DLL `vista_map` / `vista_world`（无 `extern "C"`）。

**Tech Stack:** GN/Ninja、MSVC C++23、MapLibre Native DLL、cesium-flycube + FlyCube（CMake 编 world DLL）。

## Global Constraints

- 命名空间 `vista`；方法 PascalCase；注释英文。
- 不引入 Qt/CEF/WebView2；不搬 smartgis 全量 vista 实现。
- 三维 SoT = FlyCube；UE 后置。
- 工作在 `master`；用户未要求则不另开分支。

---

### Task 1: vista 树与 MapPass C++

- [x] 新增 `vista/vista_export.h`、`vista/BUILD.gn`、`vista/pass/map/*`（自 `gis/map` 迁入并改类 API）。
- [x] 删除 `mvm_*`；`output_name = "vista_map"`。
- [x] 测试改挂 `vista::MapPass`；`build.bat debug vista_map` / map 测试。

### Task 2: WorldPass C++ + CMake

- [x] 新增 `vista/pass/world/world_pass.h|.cc`（自 `mview_flycube` 迁入）。
- [x] CMake 目标改为 `vista_world`；删 `mvf_*`。
- [x] GN action 输出 `vista_world.dll`。

### Task 3: 壳接线

- [x] `MapLibrePresenter` → `vista::MapPass`。
- [x] `FlyCubeHost` 链 `WorldPass`，去掉 `GetProcAddress(mvf_*)`。
- [x] `content/BUILD.gn` deps 指向 `//vista/pass/...`。

### Task 4: 清理与规格

- [x] 旧路径变为转发 group；更新活规格锁与 `docs/superpowers/README.md`。
- [x] `vista_map` / `vista_world` / `map_pass_test` / `product_presenters` Debug 绿。
