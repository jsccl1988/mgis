# mview

Views 示例。编译入口是 `build.bat`（GN + ninja，`out/Debug` 与 `out/Release`）。`m` 编产品目标，`sln` 会被拒绝。三方源码在 `third_party/.src`，说明见 `third_party/README.md`。工程描述在各目录的 `BUILD.gn`。

```bat
build.bat debug
build.bat release
build.bat t
```

Debug 默认 **component build**（`is_component_build=true`）：`base` / `ui` / `views` / `skia` 等为 DLL，加快增量链接。Release 保持静态库。产物在 `out/Debug/`（exe + 对应 dll）。
