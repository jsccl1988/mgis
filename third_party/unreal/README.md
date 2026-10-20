<!--
Copyright (c) 2026 The Mogu Authors.
All rights reserved.
-->

# Unreal Engine 5.6

清单名 `unreal`，`source_policy` 为 `external`。目标位置是本机的 UE 5.6 安装根（含 `Engine\Binaries\Win64`），用用户环境变量 `UE_5_6_ROOT` 指向它。官方源码仓需要已关联 Epic 的 GitHub；本仓库不把引擎放进 `third_party/.src`。`build.bat t` 不会克隆引擎。

## 本机安装（Launcher）

1. 已可用 winget / MSI 安装 **Epic Games Launcher**（常见路径 `C:\Program Files\Epic Games\Launcher\...`）。
2. 登录 Epic 账号 → 左侧 **Unreal Engine** → **Library** → **Engine Versions** → 安装 **5.6**（默认约在 `C:\Program Files\Epic Games\UE_5.6`，体积数十 GB）。
3. 安装完成后执行：

```bat
powershell -ExecutionPolicy Bypass -File third_party\unreal\set_ue_5_6_root.ps1
```

该脚本会探测 `UnrealEditor.exe` 并把用户级 `UE_5_6_ROOT` 写成引擎根目录。新开终端后生效。

示例工程：`third_party/unreal/example/Blank56.uproject`，`EngineAssociation` 为 `5.6`。产品窗口框仍由 aura/views 创建，这个工程不带标题栏产品界面。
