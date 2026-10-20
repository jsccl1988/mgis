<!--
Copyright (c) 2026 The Mogu Authors.
All rights reserved.
-->

# VTK 9.5.2（无 Qt）

清单名 `vtk`。源码在 `third_party/.src/vtk`，不进本仓库 GN。CMake 参数关掉 Qt 组、Rendering 组、Views 组和 Web 组。

```bat
py -3 third_party\tools\fetch.py --package vtk
cmake -S third_party\.src\vtk -B third_party\.build\vtk -DVTK_GROUP_ENABLE_Qt=NO -DVTK_GROUP_ENABLE_Rendering=NO -DVTK_GROUP_ENABLE_Views=NO -DVTK_BUILD_TESTING=OFF
cmake --build third_party\.build\vtk --config Release
cmake --install third_party\.build\vtk --prefix third_party\.install
cmake -S third_party\vtk\example -B third_party\.build\vtk-example -G "Visual Studio 18 2026" -A x64 -DVTK_DIR=third_party\.install\lib\cmake\vtk-9.5
cmake --build third_party\.build\vtk-example --config Release
set PATH=%CD%\third_party\.install\bin;%PATH%
third_party\.build\vtk-example\Release\vtk_image_grid.exe third_party\.build\vtk-example\grid.vti
```

示例只分配 `vtkImageData` 并写出 `.vti`，不创建渲染窗口。
