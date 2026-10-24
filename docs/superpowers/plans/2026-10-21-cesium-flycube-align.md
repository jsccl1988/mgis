<!--
Copyright (c) 2026 The Mogu Authors.
All rights reserved.
-->

# Cesium for FlyCube 对齐 cesium-native

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 让 `third_party/cesium-flycube` 覆盖 Cesium for Unreal 里落在 cesium-native 上的视图、瓦片集、影像和要素。

**Architecture:** 每个虚幻组件对应一个 FlyCube 类型，调用同一份 cesium-native。不引入 Pawn、材质或编辑器。默认原点漂移关闭，保留 100 km `origin_conflict`。

**Tech Stack:** C++20、cesium-native、CMake。不进壳的 GN 图。

## Global Constraints

- 世界坐标：右手系、米、X 东 / Y 北 / Z 上。
- 不调用已废弃的 `AddCamera`。
- 壳不包含 `cesium_flycube` 头。
- 平替路径不要求 Cesium ion。
- 新增注释用英文。

---

### Task 1: 视图

- [x] `Georeference::SetOriginEcef`、`GlobeAnchor::SetEcef`
- [x] `OriginShift` 三种模式
- [x] `Session::ShiftOrigin`
- [x] `align_native` 的原点检查

### Task 2: 瓦片集

- [x] `TilesetSettings` 写入 `TilesetOptions`
- [x] `last_load_error`、`LoadProgress`
- [x] `align_native` 的选项检查

### Task 3: 影像

- [x] 九种 overlay 工厂
- [x] `PrepareRendererResources` 拷贝并挂上 `RasterDrape`
- [x] `SampleRaster`
- [x] `ellipsoid_city` 挂调试分色

### Task 4: 要素

- [x] `_FEATURE_ID_0` 与字符串属性列
- [x] `FeatureIdAt` / `PropertyAt`
- [x] `align_native` 的三角形检查
