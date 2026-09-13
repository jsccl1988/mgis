# mgis

Windows 桌面 GIS 壳（WTL）。工程管理对齐 mogu：GN/Ninja、根目标组、测试注册、`out/` 产物。

系统设计与产品边界见工作区其它文档；本文只覆盖**怎么编、编什么**。

## 快速开始

工具：`build\bin\gn.exe` + ninja（或 `D:\Dev\buildtools` / `D:\Dev\depot_tools`）。缺省时 `build.bat` 会跑 `python build\fetch_binaries.py`。

```bat
build.bat
build.bat te
```

等价手搓：

```bat
gn gen out --root=./ --ide=vs2019 --args="is_debug=true is_build_third_party=false"
ninja -C out all
ninja -C out test_all
```

产物只在仓库根 **`out/`**。第三方按 mogu：先 `build.bat t` 装到 **`third_party/.install`**，GN 只消费该 prefix（不再 `cmake()` 竞态编译）。

## 根目标（对齐 mogu）

| 目标 | 别名 | 含义 |
| --- | --- | --- |
| `//:all` | `m` | 产品库 + `smartgis`（`is_build_third_party=false`） |
| `//:test_all` | `te` | 已注册单测 |
| `//:all_with_tests` | `a` | `all` + `test_all` |
| `//:benchmark_all` | `b` | 预留；尚无 benchmark 目标 |
| （tools） | `t` | `batch.py` → `third_party/.install`（可跟包名） |

`build.bat` 可选参数：`m` / `te` / `a` / `b` / `t` / 任意 ninja 目标名。

## 模块

| 树 | 产品组 | 测试 |
| --- | --- | --- |
| `base/` | `//base:base_all` | `//base:base_test` |
| `core/` | `//core:core_all` | `//core:core_test` |
| `content/` | `//content:content_all` | `//content:content_test` |
| `gui/` | `//gui:gui_all`（`smartgis`） | — |
| `gfx/2d/` | `//gfx/2d:gfx_2d_all` | `//gfx/2d:renderer2d_test` |
| `gfx/3d/` | `//gfx/3d:gfx_3d_all` | `//gfx/3d:renderer3d_test` |
| `third_party/` | `//third_party:third_party_all` | 门面 group；先 `build.bat t` |

测试/benchmark 模板：`testing/README.md`。GN 布局：`build/README.md`。

## 约定

- C++：Win `c++20`（MSVC `/std:c++20`；C++23 特性在本仓库 MSVC 能编过时可用），非 Win `c++23`（`build/BUILDCONFIG.gn`）
- 格式：Google + 80 列（`.clang-format` / `.editorconfig`）
- 警告：默认 `/W3`；可选 `//build/config:warnings`（`/W4`，不自动打开）

---

**最后更新：** 2026-09-13
