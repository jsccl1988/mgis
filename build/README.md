<!--
Copyright (c) 2026 The Mogu Authors.
All rights reserved.
-->

# build — GN/Ninja toolchain

对齐 mogu `build/`：全局 args、toolchain、默认 `config`。mgis 以 **Windows MSVC** 为主；Linux/mac toolchain 文件保留但不作为日常入口。

## 职责

任何 compiler/linker 选项、平台切换、第三方 **路径约定** 应在此修改，而不是写进各个模块 `BUILD.gn`。

## 目录

| Path | 说明 |
| --- | --- |
| `BUILDCONFIG.gn` | `default_toolchain`、`is_debug` / `is_build_third_party` / `cc_std`、默认 `configs` |
| `config/BUILD.gn` | `default`、`c_std`、`cc_std`、可选 `warnings` |
| `config/win/` | MSVC 默认 flags、CRT、subsystem |
| `toolchain/` | win/linux/mac toolchains |
| `tools/` | `cmake.gni` / `makefile.gni` 及 Python 驱动 |

## 常用操作

```bat
gn gen out --args="is_debug=true is_build_third_party=false"
gn args out --list
ninja -C out all
ninja -C out test_all
```

第三方按 mogu：`build.bat t` → `third_party/tools/batch.py` → **`third_party/.install`**。GN `//third_party:*` 是门面，不再跑 `cmake()`。

```bat
build.bat t
build.bat t glog
```

入口脚本：仓库根 `build.bat`（可带 `m` / `te` / `a` / `b` / `t`）。

## 与 mogu 的差异（有意保留）

- 使用 `third_party/.install` / `build.bat t`（对齐 mogu `build.sh build t`）；**不**引入 Bazel
- **不**默认打开 `/W4` 或 sanitizers（MSVC 默认 `/W3`；sanitizer args 已声明但未接到 MSVC flags）
- `use_fast_debug` 已声明，MSVC 仍用 `/Zi`

新增编译选项时：在 `config/` 加 `config()`，再 `configs +=`，不要改单个 target 的裸 flags。

---

**最后更新：** 2026-09-13
