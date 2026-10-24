<!--
Copyright (c) 2026 The Mogu Authors.
All rights reserved.
-->

# MapLibre Native

清单名 `maplibre-native`，标签 `ios-v6.30.0`，从 smartgis 的 pin 原样抄来。源码在 `third_party/.src/maplibre-native`（可联接到 smartgis 已有目录）。本仓库 GN 不编译它。Windows 示例走 OpenGL，产出上游 `mbgl-render` / `mbgl-glfw`。

产品二维地图在 `vista/pass/map`（`vista::MapPass` / `vista_map.dll`）。可选 `mview_maplibre.dll` 的 CMake 嵌入源仍在 `third_party/maplibre/mview_map/`。
