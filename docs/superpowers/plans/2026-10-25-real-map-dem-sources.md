<!--
Copyright (c) 2026 The Mogu Authors.
All rights reserved.
-->

# 真数据源 2D/3D Implementation Plan

> **For agentic workers:** 与 living §「中国底图」同步；方案 1 混合缓存。

**Goal:** 默认交付可读 OSM Bright 中国区 2D + 椭球 DEM 色/影像 3D；示意 GeoJSON / debug colorize 仅作离线回退。

**Architecture:** 仍走 `vista::MapPass` / `WorldPass`；不链壳 GN。

**Tech Stack:** OpenFreeMap Bright、Esri imagery TMS、AWS Terrarium DEM 色、MapLibre disk cache。

**Spec:** `docs/superpowers/specs/2026-10-19-ue-mview-maplibre-design.md` § 中国底图（真源）。

---

### Task 1: living § + 图

- [x] 更新验收与 env；修订 `china-map-gesture-flycube.html`

### Task 2: 2D 默认 OpenFreeMap Bright

- [x] `maplibre_present.cc`：空 `MVIEW_MAP_STYLE` → OpenFreeMap；`offline` → 示意 GeoJSON；失败回退 offline

### Task 3: 3D 默认影像 + DEM 色

- [x] `DrapedTerrainOptions` + `CaptureDrapedEllipsoid` 叠 DEM/影像 overlay
- [x] `world_pass` 默认开启（`MVIEW_FLYCUBE_OFFLINE=1` 关网）

### Task 4: 构建

- [x] `build.bat debug mgis` 绿（含 mview_maplibre / vista_world）
