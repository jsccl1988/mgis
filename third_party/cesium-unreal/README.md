<!--
Copyright (c) 2026 The Mogu Authors.
All rights reserved.
-->

# Cesium for Unreal 2.30（UE 5.6）

清单名 `cesium-unreal`。`fetch.py --package cesium-unreal` 下载 `CesiumForUnreal-56-SourceOnly-v2.30.0.zip` 到 `third_party/.src/cesium-unreal`。发行用的二进制包名是 `CesiumForUnreal-56-v2.30.0.zip`。ninja 不编译这个插件。

示例工程 `third_party/cesium-unreal/example/MViewGlobe.uproject`：

1. 把解压后的插件目录放到该工程的 `Plugins/CesiumForUnreal`（或装进引擎的 `Engine/Plugins`）。
2. 用 UE 5.6 打开工程，生成 `MViewGlobe` 的工程文件后再编译。
3. `MViewCameraPawn` 把相机写入 `ACesiumCameraManager::AdditionalCameras`，不调用已废弃的 `AddCamera`。

产品里不要再另链一份独立的 cesium-native。
