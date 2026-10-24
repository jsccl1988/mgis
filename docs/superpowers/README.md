<!--
Copyright (c) 2026 The Mogu Authors.
All rights reserved.
-->

# Superpowers

方案放在 `specs/`。本仓库还没有 living 伞；桌面 GIS 的第一份规格就是下面这一行。实现计划开在 `plans/`，图放在 `diagrams/`。这两项目前为空。

| 文档 | 内容 |
| --- | --- |
| [`specs/2026-10-19-ue-mview-maplibre-design.md`](specs/2026-10-19-ue-mview-maplibre-design.md) | aura/views 界面；content / glang / 插件；UE 与 MapLibre 共用 tool::Workspace；P0–P7 门控；第三方 pin；§ Mgis；§ 产品入口合并（`mgis.exe`） |
| [`specs/2026-10-21-debug-component-build-design.md`](specs/2026-10-21-debug-component-build-design.md) | Debug `is_component_build`：产品库 + Skia DLL；Release 静态 |
| [`diagrams/debug-component-build.html`](diagrams/debug-component-build.html) | component DLL 分层示意 |
| [`specs/2026-10-20-views-declarative-markup-design.md`](specs/2026-10-20-views-declarative-markup-design.md) | views markup（mview 方言）+ Yoga + ControlFactory；独立 UiDesigner |
| [`plans/2026-10-20-mgis-workbench-chrome.md`](plans/2026-10-20-mgis-workbench-chrome.md) | `app/mgis` 四区壳第一刀（面板已自 `ui/gis` 并入） |
| [`plans/2026-10-23-mgis-smartgis-chrome-align.md`](plans/2026-10-23-mgis-smartgis-chrome-align.md) | 壳对齐 SmartGIS：工具条 / Layers·Sources·Maps / GroupBar / 底五页 + 状态栏 |
| [`plans/2026-10-24-mgis-theme-chrome.md`](plans/2026-10-24-mgis-theme-chrome.md) | 主题基座 + chrome 视觉（light 默认 / dark；双轨 token） |
| [`diagrams/mgis-theme.html`](diagrams/mgis-theme.html) | ThemeService → Widget → horizon/markup 双轨 |
| [`plans/2026-10-20-views-declarative-markup.md`](plans/2026-10-20-views-declarative-markup.md) | markup 运行时 + UiDesigner 落地任务 |
| [`diagrams/third-party-pins.html`](diagrams/third-party-pins.html) | UE 5.6、Cesium、VTK、MapLibre 的清单落点 |
| [`diagrams/mgis-workbench-chrome.html`](diagrams/mgis-workbench-chrome.html) | 工作台分层与 Tab 联动 |
| [`diagrams/content-wiring.html`](diagrams/content-wiring.html) | Content 接线：手势事实、FlyCube 子窗口、底栏 trace、glang |
| [`diagrams/plugin-wiring.html`](diagrams/plugin-wiring.html) | 插件：Registry/Loader → PluginHost → Workspace；右栏可跑 |
| [`diagrams/scene-presenter-registry.html`](diagrams/scene-presenter-registry.html) | Scene 呈现器注册表：双 ViewportSurface、MapLibre / UE→FlyCube |
| [`diagrams/views-declarative-markup.html`](diagrams/views-declarative-markup.html) | markup 资源 → 管线 → UiDesigner / 产品宿主 |
| [`plans/2026-10-24-china-map-gesture-flycube.md`](plans/2026-10-24-china-map-gesture-flycube.md) | 2D 中国离线矢量 → 手势接线 → FlyCube GE |
| [`plans/2026-10-25-real-map-dem-sources.md`](plans/2026-10-25-real-map-dem-sources.md) | 默认真源：OpenFreeMap Bright + DEM 色/影像 |
| [`diagrams/china-map-gesture-flycube.html`](diagrams/china-map-gesture-flycube.html) | 真数据源分层与混合缓存 |
| [`diagrams/gis-render-product-abi.html`](diagrams/gis-render-product-abi.html) | （历史）gis/map + render C ABI；已由 vista 取代 |
| [`specs/2026-10-24-vista-render-engine-design.md`](specs/2026-10-24-vista-render-engine-design.md) | 顶层 `vista/`：MapPass / WorldPass C++；MapLibre + cesium_flycube；删 C ABI |
| [`diagrams/vista-render-engine.html`](diagrams/vista-render-engine.html) | vista 分层与壳接线 |
| [`plans/2026-10-24-vista-render-engine.md`](plans/2026-10-24-vista-render-engine.md) | vista 落地任务 |
