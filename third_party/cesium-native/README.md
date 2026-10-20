<!--
Copyright (c) 2026 The Mogu Authors.
All rights reserved.
-->

# cesium-native v0.64.0

清单名 `cesium-native`。这是 Cesium for Unreal 2.30 文档所写的 native 版本，只给本目录的示例。产品地理底座用插件包里的那一份，不要把这里再链进 `mview_shell` 或 UE 模块。

源码在 `third_party/.src/cesium-native`，CMake 安装前缀是 `third_party/.install`。

```bat
py -3 third_party\tools\fetch.py --package cesium-native
set EZVCPKG_BASEDIR=C:\cn-vcpkg
py -3 third_party\tools\install.py --manifest third_party\manifest.json --package cesium-native --src-root third_party\.src --build-root third_party\.build\cesium-native --install-prefix third_party\.install --stamp third_party\.build\cesium-native\.gn_publish_stamp --unified-prefix --build-type Release --no-deps --no-cmake-calibrate
cmake -S third_party\cesium-native\example -B third_party\.build\cesium-native-example -G "Visual Studio 18 2026" -A x64
cmake --build third_party\.build\cesium-native-example --config Release
third_party\.build\cesium-native-example\Release\cesium_ellipsoid.exe
```

示例 CMake 会把 `third_party/.install` 和 `EZVCPKG_BASEDIR`（默认 `C:\cn-vcpkg`）下的 `x64-windows-static-md` 工具链拼进查找路径；不链进产品进程。
