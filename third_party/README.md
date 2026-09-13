# third_party — mogu-aligned fetch / install

## 概述

对齐 mogu：`manifest.json` 是 pin 唯一真相；`tools/fetch.py` / `install.py` /
`batch.py` 拉源码并装到 **`third_party/.install/{bin,lib,include}`**。GN 只消费
该 prefix（`gn/tp.gni` + `gn/BUILD.gn` 门面），**不再**用 `cmake()` 在 ninja
里竞态编译三方。

Gitea 基址与 mogu 相同：`http://localhost:3000/ccl`（`manifest.json` →
`gitea_mirror`）。已在 Gitea 上的仓直接复用 manifest，不重新上传。

## 入口

```bat
REM 按拓扑安装全部 install_skip=false 包（Debug + MSVC，匹配 build.bat is_debug）
build.bat t

REM 单包
build.bat t glog
py -3 third_party\tools\fetch.py --package glog
py -3 third_party\tools\install.py --manifest third_party\manifest.json --package glog --src-root third_party\.src --build-root third_party\.build\glog --install-prefix third_party\.install --stamp third_party\.build\glog\.gn_publish_stamp --unified-prefix --build-type Debug
```

本地已有 `third_party/<name>/CMakeLists.txt` 时，fetch 跳过克隆（vendored）。
强制从 Gitea 拉：`set MGIS_TP_FORCE_FETCH=1`。

成功后若 `out\third_party` 不存在，`build.bat t` 会 junction 到 `.install`，
以便桌面 `DirectoryAsDLLSearchPath(...\third_party\bin)` 仍能找到 DLL。

## 结构

| Path | 说明 |
| --- | --- |
| `manifest.json` | `incubator_third_party_manifest_v1` |
| `.src/` | fetch 克隆（gitignore）；有 vendored 树时可空 |
| `.build/<pkg>/` | 单包 CMake 树（gitignore） |
| `.install/` | 合并安装前缀（gitignore）；GN `third_party_install_prefix` |
| `gn/` | `tp.gni` + 门面 `config`/`group` |
| `BUILD.gn` | `//third_party:<name>` → `//third_party/gn:<name>` |
| `CxImage/` | 仍由 GN 编（`install_skip`） |
| `wtl8.0/` | header-only（`install_skip`） |
| `GITEA_MIRROR.md` | 各仓 Gitea 状态 |

## 与 mogu 的差异

- Windows / MSVC only：`install.py` 用 VS generator、`CMAKE_MSVC_RUNTIME_LIBRARY`、`--config Debug`
- 不引入 Bazel
- GIS 包（GDAL/GEOS/PROJ/…）是 mgis 清单，mogu 没有
- glog / googletest / cpp-httplib 的 **git_ref 与 mogu 相同**，URL 指向已有 Gitea tag
- sanitizer / GCC 12 未接线（MSVC）
