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
| `cesium_flycube/tiles` | 瓦片 I/O（线程、HTTP、credits）、选块、glTF 网格 |
| `cesium_flycube/gpu` | 把网格打成 FlyCube 的 upload buffer |
| `example` | `dual_view`、`ellipsoid_city` |

两台相机的 ECEF 距离超过 100 km 时返回 `origin_conflict`，不改地理原点。壳进程不链接本目录。产品里的地球仍只来自 Cesium for Unreal 插件自带的 cesium-native；壳上现有三维子窗口仍是 `GlobeView`。

## 示例

依赖已经装好的 `third_party/.install`（cesium-native）和 `third_party/.src/flycube/out/lib/FlyCube.lib`（Release `/MD`）。

```bat
cmake -S third_party\cesium-flycube -B third_party\cesium-flycube\build
cmake --build third_party\cesium-flycube\build --config Release
third_party\cesium-flycube\build\Release\cesium_flycube_dual_view.exe
third_party\cesium-flycube\build\Release\cesium_flycube_ellipsoid_city.exe
```

- `dual_view`：上海原点、两个锚点、两台相机，以及跨洲时的 `origin_conflict`。
- `ellipsoid_city`：椭球瓦片选块、采样高度，并把选中网格上传到 FlyCube DX12。
