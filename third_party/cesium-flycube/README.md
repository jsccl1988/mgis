<!--
Copyright (c) 2026 The Mogu Authors.
All rights reserved.
-->

# Cesium for FlyCube

对照 `third_party/.src/cesium-unreal` 的运行时。FlyCube 没有 Pawn / CameraComponent，相机只走 `AdditionalCameras`（不提供已废弃的 `AddCamera`）。

| 目录 | 职责 |
| --- | --- |
| `cesium_flycube/geo` | 地理参考与锚点。WGS84，右手系，X 东 / Y 北 / Z 上，米 |
| `cesium_flycube/view` | 相机集合、原点分离策略、ViewState |
| `cesium_flycube/tiles` | 瓦片 I/O、选块、glTF 网格、`RasterDrape` |
| `cesium_flycube/gpu` | `FrameToPresent` / `CaptureDrapedEllipsoid`、FlyCube upload |
| `example` | 全部走椭球 + overlay 闭环；领域示意叠在 draped 地形上 |

两台相机的 ECEF 距离超过 100 km 时返回 `origin_conflict`，不改地理原点。壳进程不链接本目录。

## 示例

依赖 `third_party/.install`（cesium-native）和 `third_party/.src/flycube/out/lib/FlyCube.lib`（Release `/MD`）。

```bat
cmake -S third_party\cesium-flycube -B third_party\cesium-flycube\build
cmake --build third_party\cesium-flycube\build --config Release
third_party\cesium-flycube\build\Release\cesium_flycube_align_native.exe
third_party\cesium-flycube\build\Release\cesium_flycube_ellipsoid_city.exe
third_party\cesium-flycube\build\Release\cesium_flycube_browse.exe --check
third_party\cesium-flycube\build\Release\cesium_flycube_google_earth.exe --check
```

| 目标 | 说明 |
| --- | --- |
| `align_native` | 原点 / settings / 要素 / SampleRaster / tileset+overlay |
| `dual_view` | draped 地形 + 双相机 + `origin_conflict` |
| `ellipsoid_city` / `globe_city` | 上海椭球 + debug colorize + present |
| `browse` / `google_earth` | `CaptureDrapedEllipsoid` + 飞行；`--imagery` 试 Esri URL |
| `vista_world` | 产品 `vista::WorldPass`：draped 椭球进子 HWND |
| `geology` / `groundwater` / `river_flood` / `wind_field` | 领域网格叠在 draped 地形上 |

默认 overlay 是 `MakeDebugColorizeTilesOverlay`（离线可用）。`--imagery` 使用 Esri World Imagery URL template（需网络，走 cesium-native Curl）。
