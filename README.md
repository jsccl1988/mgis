# mview

Views 示例。编译入口是 `build.bat`（GN + ninja，`out/Debug` 与 `out/Release`）。`m` 编产品目标，`sln` 会被拒绝。三方源码在 `third_party/.src`，说明见 `third_party/README.md`。工程描述在各目录的 `BUILD.gn`。上游 Skia 自带的 `gyp/` 不参与本仓库编译。

```bat
build.bat debug
build.bat release
build.bat t
```

Debug 产物：`out/Debug/base_widget.exe`、`out/Debug/activex_widget.exe`。`extra_views` 是静态库，随默认组一起编。
