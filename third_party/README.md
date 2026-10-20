# third_party

对齐 smartgis：`manifest.json` 是 pin 的唯一来源。本仓库的 Chromium 快照已经在树里，`tools/fetch.py` 把它们认作 **`third_party/.src/<name>`**（`source_policy: vendored`），不从网络重新克隆。

`third_party/<name>` 与 `testing/gtest`、`testing/gmock` 是指向 `.src` 的 junction，用来保持现有 `#include "third_party/skia/..."` 和 GYP 相对路径。junction 已写入 `.gitignore`，避免 Git 记成第二份源码。

GN 编译这些库，不在 ninja 里跑 CMake。`build.bat t` 只核对清单里的 vendored 目录是否还在，并跳过 `skip_fetch_all` 与 `source_policy: external`。

产品默认图只链 Chromium 快照（见 `//third_party:third_party_all`）。GIS / 工具库用 `//third_party:<name>` **按需**引入；本机可用 junction 复用 smartgis 检出。

## 从 smartgis 同步（方案 B）

一次性同步清单 + 薄 GN + `.src` junction：

```bat
py -3 third_party\tools\sync_from_smartgis.py
```

**硬排除（壳 UI，永不迁入）：** `cef`、`webview2`、`windows_app_sdk`、`bcg`。

**不覆盖：** Chromium 的 `.src/zlib`（以及 skia / gtest 等已有树）。smartgis 给 PDAL 用的 cmake zlib 只进 `.install`，不替换 vendored zlib。

**`.install`：** 勿整树替换 mview 已有前缀。缺某个 CMake 安装产物时，对 `include` / `lib` / `bin` 单项 junction 到 smartgis 的 `.install`，或 `build.bat t <pkg>`。

公开标签示例：`//third_party:rapidjson`、`//third_party:gdal`、`//third_party:asio`。pipe JSON 使用 RapidJSON，不再手写迷你解析器。

MapLibre Native 的 `git_ref` 与 smartgis 相同：`ios-v6.30.0`。本机若已有那份检出，用目录联接，不要再克隆一份：

```bat
mklink /J third_party\.src\maplibre-native C:\Dev\src\gis\smartgis\third_party\.src\maplibre-native
```

没有这份检出时：

```bat
py -3 third_party\tools\fetch.py --package maplibre-native
```

## 引擎类 pin（默认不进 `build.bat t`）

| 包 | pin | 源码位置 | 示例 |
| --- | --- | --- | --- |
| `unreal` | UE 5.6，环境变量 `UE_5_6_ROOT` | 不进 `.src` | `third_party/unreal/example` |
| `cesium-native` | `v0.64.0` | `.src/cesium-native` | `third_party/cesium-native/example` |
| `cesium-unreal` | Cesium for Unreal 2.30，`CesiumForUnreal-56` 源码包 | `.src/cesium-unreal` | `third_party/cesium-unreal/example` |
| `cesium-flycube` | 本仓库实现，对照 Cesium for Unreal 运行时 | `third_party/cesium-flycube` | `dual_view`、`ellipsoid_city` |
| `vtk` | `v9.5.2`，关闭 Qt 与 Rendering | `.src/vtk` | `third_party/vtk/example` |
| `maplibre-native` | `ios-v6.30.0` | `.src/maplibre-native` | `third_party/maplibre/example` |

产品进程只链 Cesium for Unreal 插件里的那一份 cesium-native。`third_party/.src/cesium-native` 只给旁边的示例用。VTK 与 MapLibre 不进本仓库的默认 GN 图。

```bat
py -3 third_party\tools\fetch.py --package vtk
py -3 third_party\tools\fetch.py --package cesium-native
py -3 third_party\tools\fetch.py --package cesium-unreal
```

```bat
build.bat t
build.bat debug
```

生成根目录是 `out/Debug` 与 `out/Release`。
