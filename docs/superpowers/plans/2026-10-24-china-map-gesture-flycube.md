<!--
Copyright (c) 2026 The Mogu Authors.
All rights reserved.
-->

# 中国底图 · 手势 · FlyCube Google Earth Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 产品 2D 离线可读中国矢量图（可选 demotiles/自托管）；子窗鼠标/触摸经 `InputEvent` 驱动相机；3D FlyCube 对齐 `google_earth` + 中国 DEM/影像。

**Architecture:** 方案 1 — 加深 `MapLibrePresenter` / `FlyCubePresenter` 与 C ABI DLL；壳与 `content/public` 不链引擎头。

**Tech Stack:** MapLibre Native（`mview_map` / `mview_maplibre`）、cesium-flycube、`tool::Workspace`、mgis `ViewportSurface`。

## Global Constraints

- 实现顺序：2D → 手势 → 3D。
- 不扩 Interaction 表；不接 Photorealistic / 天地图 GCJ。
- 不把 MapLibre / FlyCube 链进壳 GN。

---

## Task 1: 2D 离线中国 style + MVIEW_MAP_STYLE

**Files:** `third_party/maplibre/mview_map/maplibre_present.cc`; product ABI now `gis/map/` (`map_surface.cc`, `mview_map.cc`, `mview_map.h`)

- [x] 默认 style：大陆 + 台/琼示意多边形 + 描边；mercator 回退同轮廓
- [x] `MVIEW_MAP_STYLE`：空 | `demotiles` | `http(s)` | 本地路径
- [x] `mln_session_set_style` + `mvm_set_style_url` 进渲染路径
- [x] `build.bat debug` mview_map / product_presenters / mgis / mview_shell 绿

## Task 2: 子窗手势 → GisContents::HandlePointer

**Files:** `content/public/browser/gis_contents.h`, `gis_contents.cc`, `scene.h`, `scene_impl.*`, `maplibre_presenter.*`, `flycube_host.*`, `map_surface.cc`, `app/mgis/workbench/workbench.cc`

- [x] 公开 `HandlePointer`；Attach 后 `SetInputForwarder`
- [x] MapSurface / FlyCubeHost：左键 pan、滚轮 zoom、右键倾斜、捏合→wheel
- [x] 绝对坐标→delta；默认 `view.pan` / 切 3D 时 `view3d.trackball`
- [x] 2D 导航工具激活时滚轮始终 zoom

## Task 3: FlyCube Google Earth + 中国 DEM/影像

**Files:** `render/rhi/flycube/*`, `third_party/cesium-flycube/cesium_flycube/gpu/scene_present.*`, `draped_terrain.*`

- [x] `mview_flycube` 接 `CaptureDrapedEllipsoid` + `ProductMeshCache`（对齐 `google_earth` 飞入/orbit）
- [x] 默认中国原点 `105E/35N`；`MVIEW_FLYCUBE_LON/LAT`；`MVIEW_FLYCUBE_IMAGERY` + `MVIEW_FLYCUBE_IMAGERY_URL`
- [x] 指针回调与 Task 2 同路径（FlyCubeHost 子类化 DX12 HWND）
- [x] 失败时回退手搓球；非目标：ion Photorealistic、真 GPU 纹理绑定
