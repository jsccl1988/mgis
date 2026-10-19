<!--
Copyright (c) 2026 The Mogu Authors.
All rights reserved.
-->

# Unreal Engine 5.6

清单名 `unreal`，`source_policy` 为 `external`。目标位置是本机的 UE 5.6 安装根（`Engine\Binaries\Win64`），用 `UE_5_6_ROOT` 指向它。`https://github.com/EpicGames/UnrealEngine.git` 需要已关联 Epic 账号的 GitHub 权限，当前环境返回 repository not found，本机也没有现成的引擎目录，所以还不能放进 `third_party/.src`。`build.bat t` 不会克隆引擎。

示例工程：`third_party/unreal/example/Blank56.uproject`，`EngineAssociation` 为 `5.6`。用已安装的 5.6 编辑器打开。产品窗口框仍由 aura/views 创建，这个工程不带标题栏产品界面。
