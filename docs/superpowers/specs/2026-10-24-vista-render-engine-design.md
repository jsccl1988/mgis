<!--
Copyright (c) 2026 The Mogu Authors.
All rights reserved.
-->

# Vista 渲染引擎层（MapLibre + cesium_flycube 通路）

**Status:** landed (P0 pathway)  
**Date:** 2026-10-24  
**Diagram:** [`../diagrams/vista-render-engine.html`](../diagrams/vista-render-engine.html)  
**Plan:** [`../plans/2026-10-24-vista-render-engine.md`](../plans/2026-10-24-vista-render-engine.md)  
**Living:** 修订 [`2026-10-19-ue-mview-maplibre-design.md`](2026-10-19-ue-mview-maplibre-design.md) 中「禁 vista / C ABI 产品门面 / UE 三维 SoT」相关锁。

## Goal

在本仓增加 **vista**（smartgis scenic 的下一代定位）：把 `gis` 数据面与 `render` RHI/引擎面收成一条 **C++** 呈现通路，P0 打通：

1. 二维：`vista::MapPass` → MapLibre Native 真帧（失败可 mercator 回退）。
2. 三维：`vista::WorldPass` → cesium_flycube / FlyCube DX12 真帧（壳默认；UE 管道后置）。
3. 删除对外 **`mvm_*` / `mvf_*` C ABI**；壳与测试只 include `vista/` 头、链 vista 产物。

## Non-goals（P0）

- 不搬 smartgis 全量 MapIR / 大气 / 地形 Pass 实现，只保留 **目录与依赖方向** 通路。
- 不把 MapLibre / FlyCube 引擎源编进壳；引擎仍在 `third_party`。
- 不在本刀恢复 UE 为三维 SoT。
- 不引入 Qt / CEF / WebView2。

## Locked decisions

| # | 决定 |
| --- | --- |
| 1 | 顶层目录 **`vista/`**；公开命名空间 **`vista`**；方法 **PascalCase**（对齐本仓 content/views，不用 C ABI，也不强制 smartgis `snake_case`）。 |
| 2 | 依赖方向：`gis`（数据，P0 可极薄）↛ Pass GPU；`vista` Pass → `render` / third_party 引擎；`content` → **仅** `vista` 公开头。 |
| 3 | 产品包：GN group `//vista:vista`。P0 允许两个 DLL（链接边界）：`vista_map.dll`（MapLibre 通路）、`vista_world.dll`（FlyCube 通路）；**无** `extern "C"` 导出。 |
| 4 | 三维 SoT = FlyCube / cesium_flycube；UE 命名管道为后续可选后端。 |
| 5 | 删除 `gis/map/mview_map.h` 与 `render/rhi/flycube/mview_flycube.h` 的 C API；实现迁入 `vista/pass/map` 与 `vista/pass/world`。 |
| 6 | `MapLibrePresenter` / `FlyCubeHost` 改为直接使用 `vista::MapPass` / `vista::WorldPass`（导入库链接，不再 `GetProcAddress(mvf_*)`）。 |

## 分层

```
app/mgis · content presenters
        │  C++ only
        ▼
vista/pass/map  (MapPass)     vista/pass/world (WorldPass)
        │                            │
        ▼                            ▼
third_party/maplibre          render glue → cesium-flycube + FlyCube
gis/  (future data/style)     render/rhi/  (future shared RHI; P0 world 内聚)
```

## 公开 API（P0 最小）

`vista::MapPass`：Create / Destroy 生命周期由类表示；`Present`、`Resize`、`SetCamera` / `GetCamera`、`SetPointerCallback`、`hwnd`、`LastPresentKind`。

`vista::WorldPass`：`Create(parent,w,h)`、`Present`、`Resize`、`Orbit`、`SetCamera` / `GetCamera`、`hwnd`。

## 验收

- mgis Map 页：MapLibre（或可识别 mercator）子 HWND 有非纯色帧；相机手势可用。
- mgis 3D 页：FlyCube DX12 真帧（`flycube-dx12`）；相机 orbit 可用。
- 树内无产品路径 `#include` 旧 C 头；无 `mvm_` / `mvf_` 符号被壳引用。
- `build.bat debug` 相关目标与既有 map/flycube 测试（改挂 vista）通过。

## 与活规格关系

本文件是 **新顶层子系统** `vista/` 的规格。活规格中「不把 vista 搬进本仓」「产品呈现 C ABI」等锁以本文件为准修订。
