<!--
Copyright (c) 2026 The Mogu Authors.
All rights reserved.
-->

# Cesium for Unreal 2.30（UE 5.6）

清单名 `cesium-unreal`。`fetch.py --package cesium-unreal` 下载 `CesiumForUnreal-56-SourceOnly-v2.30.0.zip` 到 `third_party/.src/cesium-unreal`。发行用的二进制包名是 `CesiumForUnreal-56-v2.30.0.zip`。ninja 不编译这个插件。

示例工程 `third_party/cesium-unreal/example/MViewGlobe.uproject`：

1. `fetch.py --package cesium-unreal` 解压到 `third_party/.src/cesium-unreal` 后，工程侧已用 junction：`example/Plugins/CesiumForUnreal` → 该目录（也可装进引擎的 `Engine/Plugins`）。
2. 设置 `UE_5_6_ROOT` 指向本机 UE 5.6 安装根。用 `example/launch-editor.ps1` 打开工程：它把 `Engine\Binaries\ThirdParty\FBX\2020.2\Win64` 放进 `PATH`，否则加载器在 `LdrpInitializeProcess` 里找不到 `libfbxsdk.dll`（`c0000135`），主线程停在 `LdrpDrainWorkQueue`。生成 `MViewGlobe` 工程文件后再编译。
3. `MViewCameraPawn` 把相机写入 `ACesiumCameraManager::AdditionalCameras`，不调用已废弃的 `AddCamera`。

本机没有 Epic 关联的 GitHub 凭证、也没有 UE 5.6 安装时，无法在此树内编译/运行该示例。

产品里不要再另链一份独立的 cesium-native。
