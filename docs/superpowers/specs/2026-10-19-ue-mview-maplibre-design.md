<!--
Copyright (c) 2026 The Mogu Authors.
All rights reserved.
-->

# 桌面 GIS：UE 双视口 + mview 界面 + MapLibre Native

**Status:** draft
**Date:** 2026-10-19
**Updated:** 2026-10-24 — § 产品入口合并（`launcher` + `app/shell` → `app/mgis`，单 PE `mgis.exe`）；删除顶层占位 `gis/` / `render/` / `ipc/`；§ Vista 渲染引擎层；§ 中国底图/手势/FlyCube GE；§Content DebugAgent/MCP。2026-10-23 — § Mgis chrome；§ Scene FlyCube。
**Diagram:** [`../diagrams/third-party-pins.html`](../diagrams/third-party-pins.html) · [`../diagrams/mgis-workbench-chrome.html`](../diagrams/mgis-workbench-chrome.html) · [`../diagrams/content-wiring.html`](../diagrams/content-wiring.html) · [`../diagrams/scene-presenter-registry.html`](../diagrams/scene-presenter-registry.html) · [`../diagrams/plugin-wiring.html`](../diagrams/plugin-wiring.html) · [`../diagrams/cesium-flycube-align.html`](../diagrams/cesium-flycube-align.html) · [`../diagrams/china-map-gesture-flycube.html`](../diagrams/china-map-gesture-flycube.html) · [`../diagrams/vista-render-engine.html`](../diagrams/vista-render-engine.html)
**Plan:** [`../plans/2026-10-24-china-map-gesture-flycube.md`](../plans/2026-10-24-china-map-gesture-flycube.md)
**Scope:** 独立桌面 GIS 的进程、窗口、地理底座、体渲染与界面边界。本文是方案，不包含实现。

## Goal

做出一套可安装的 Windows 桌面 GIS：

1. 用户能看到的界面全部由本仓库的 aura/views 创建：一个二维窗口、两个三维窗口、图层树、实体树，以及时间轴和菜单。
2. 两个三维窗口显示 DEM、3D Tiles、风场，以及与场景深度正确遮挡的标量体。二维窗口用 MapLibre Native 画底图和业务矢量。
3. 地理底座使用 Cesium Native，通过 Cesium for Unreal 进入工程，不从零写 Cesium Native 胶水。
4. 窗口框、停靠、树和菜单都在 `mgis.exe` 里。UE 进程不出现自己的标题栏。
5. 界面进程崩溃或二维地图崩溃时，三维渲染进程继续出帧（Job Object / 壳重启为后续；本刀为单 PE）。
6. 进程之间只传递轻量 JSON。体纹理、风场粒子和大块网格不进管道。
7. 壳只包含 `content/public`。文档、目录、管道、glang 和插件宿主在 `content/browser`。插件只走 `content::PluginHost`。
8. 场景脚本、回归和代理迭代都走 glang 套件。具名 `eval_*` 是唯一的能力入口。

## Non-goals

- 不使用 Qt（Widgets、Quick、QML 以及 VTK 的 Qt 模块都不进构建）。
- 不使用 CEF、WebView2、HTML 或 MapLibre GL JS 作为界面或二维地图。
- 不把本方案做进 `ui/views` 的控件实现里，也不把 UE 工程放进本仓库的 GN 图。
- 不在进程内再链一份独立的 Cesium Native。地理库只有 Cesium for Unreal 插件自带的那一份。
- 不把 MapLibre 像素合成进 aura 的 `ui::Layer`。
- 不在两个三维窗口里同时以满精度观看两个相距很远的地区。
- 不做交互式流体求解（Navier–Stokes）。风场是时序矢量场的粒子和平流。
- 第一版不把 MapLibre 再拆成第三个进程。
- 第一版不使用 UE 的 Slate 窗口、UMG 或编辑器面板作为产品界面。
- 不把 smartgis vista 的全量 MapIR/大气/地形实现搬进本仓库；本仓自有瘦 `vista/`（见 [`2026-10-24-vista-render-engine-design.md`](2026-10-24-vista-render-engine-design.md)）。三维产品 SoT 为进程内 FlyCube；UE 管道后置。
- 不让 `app/shell/` 或 `plugin/` 包含管道帧、UE 头文件或 MapLibre 头文件。
- 不为套件再做一套 opcode 中间语言。glang 只有一个进程内 CPython，套件执行 `main()`。
- 不把代理的迭代写成壳里的私有测试钩子。缺能力时只增加一个具名 `eval_*`。
- 不把 MapLibre 的手势回调或 UE 的 Enhanced Input / Pawn 操控当成产品工具。它们只提供命中和相机执行。

## Locked decisions

| # | 决定 |
| --- | --- |
| 1 | 产品进程：UE 打包进程 + `mgis.exe`（单 PE 界面；原独立启动器已并入）。Job/壳重启为后续。 |
| 2 | 产品窗口全部是 shell 进程里的 `views::Widget`（`aura::RootWindow` + `RootWindowHostWin`）：主窗口、二维窗口、`view0`、`view1`。UE 不创建任务栏上的顶层窗口。 |
| 3 | 引擎钉扎 Unreal Engine 5.6，插件钉扎 Cesium for Unreal 2.30 的 `CesiumForUnreal-56` 包。 |
| 4 | 一个 `UWorld`、一个 `ACesiumGeoreference`。两台相机每帧写入 `ACesiumCameraManager::AdditionalCameras`。不用已废弃的 `AddCamera`。 |
| 5 | VTK 9.4（或 9.5）编成 `mview_volume.dll`，关闭 Qt、GUI 和 VTK Rendering。只输出规则网格。画面由 UE 后处理光线步进和 Niagara 绘制。 |
| 6 | 产品呈现走顶层 **`vista/`** C++：`vista::MapPass`（`vista_map.dll`）与 `vista::WorldPass`（`vista_world.dll`）。无对外 `mvm_*`/`mvf_*`。MapLibre / cesium-flycube 仍在 `third_party`，不把引擎源链进壳。 |
| 7 | 二维窗口和两个三维窗口的内容区都是 `views::NativeViewHost`。二维挂 MapLibre 子 HWND。三维挂 UE 在该父 HWND 下创建的子 HWND。图层树和实体树是主窗口里的 `views::TreeView`，不压在这些子 HWND 上。 |
| 8 | mview UI 线程跑 `MessageLoopForUI`。MapLibre 的 `mbgl::util::RunLoop` 在 DLL 内部线程。 |
| 9 | 命名管道 `\\.\pipe\mview`，服务端是 UE。帧为小端 `uint32` 长度 + UTF-8 JSON，单帧不超过 65536 字节。 |
| 10 | 时间轴的权威在 mview。地理坐标与三维渲染的权威在 UE。出进程的坐标只用 WGS84 经纬高（EPSG:4979）。 |
| 11 | 目录数据库只由 UE 写入。界面「打开文件」只提交路径。 |
| 12 | `content/public` 是壳的唯一嵌入面。`content` 不包含 `ui/views`，也不包含 UE 头。壳实现 `GisContentsClient`，只在浏览器进程钩子里建窗口。 |
| 13 | glang 放在 `content/browser/glang.runtime/`（`language/`、`exec/`（含 `exec/py/`）、`host/` + `script_runtime`）。公开入口只有 `content::run_suite_script`。`.py` 走进程内 CPython `main()` + `glang.plugin`；非 `.py` 保留行解释器。 |
| 14 | 插件进程内加载，清单是 `plugin.json`。插件不实现 `GisContentsClient`，不写管道。处理算法走 `PluginHost::run_processing`。 |
| 15 | 每个阶段一个套件。`required_marks` 全部落地才算通过。前一阶段未通过时，campaign 不写后续 mark，也不进入下一阶段。 |
| 16 | 每个视图一个 `tool::Workspace`（`map`、`view0`、`view1`）。命令和交互分开。工具不写文档，只交草稿；`content::EditSession` 提交；`content::EventBus` 公布已经发生的事。指针先变成 `content::InputEvent`，再进 `InputRouter`。 |

## 版本

| 组件 | 钉扎 |
| --- | --- |
| 三维引擎 | Unreal Engine 5.6 |
| 地理底座 | Cesium for Unreal 2.30（内含 cesium-native）。2.29 起要求 UE 5.6+；相机数组从 2.24 起替换 `AddCamera` |
| 体数据 | VTK 9.4 或 9.5，`/MD`，与 UE 5.6 相同的 VS 2022 工具集 |
| 二维 | MapLibre Native，按上游 `platform/glfw` 的桌面集成方式编译，窗口换成 HWND |
| 界面 | 本仓库 `ui/views`、`ui/aura`。编译入口 `build.bat`。Windows `cc_std` 见 `build/BUILDCONFIG.gn`（当前 `c++14`） |
| 投影 | PROJ 只出现在 `mview_volume.dll` 内，用于把投影网格的八个角点转到经纬高 |

发版前单独核对 Epic 对非游戏桌面应用的许可。Cesium ion 的地形和影像是另一份订阅，默认 ion token 不进安装包。三维瓦片可以自托管。MapLibre、VTK、Cesium for Unreal / cesium-native 的许可证随二进制分发声明。

## 进程与窗口

```
mgis.exe                      唯一的产品界面进程（含 layout 门禁开关）
│    主 Widget                菜单、图层树、实体树、时间轴
│    二维 Widget              NativeViewHost → MapLibre 子 HWND
│    view0 / view1 Widget     NativeViewHost → UE 子 HWND
└─ UE 打包进程                管道服务端，无任务栏窗口
     FSceneViewport × 2
     mview_volume.dll
```

启动器：

- 命名互斥量 `Local\MView.Launcher`，单实例。
- UE 与 `mview_shell.exe` 放进同一个 Job Object。启动器退出时子进程退出。
- 每个进程的清单打开 Per-Monitor DPI V2。窗口位置由 shell 按主显示器工作区摆放自己的 Widget。
- `WaitForMultipleObjects` 监视界面进程。界面退出后只重启 `mview_shell.exe`。
- UE 退出后，三维窗口的内容区留在 aura 里并显示「三维引擎已停止」，由用户确认后再拉起 UE。

二维子 HWND 的父窗口是二维 Widget。三维子 HWND 的父窗口是对应的三维 Widget。UE 与 shell 之间跨进程 `SetParent` 只发生在这块内容区上。aura 的标题栏不交给 UE。

### 崩溃域

| 故障 | 结果 |
| --- | --- |
| `mview_shell.exe` 或 MapLibre 访问违例 | 启动器重启界面进程。父 HWND 随之销毁。UE 进程和已加载的瓦片、体纹理保留，收到新的 `viewport.attach` 后重建子 HWND |
| VTK 在 UE 进程内访问违例 | 三维进程退出。DLL 把 C++ 异常转成错误码；可读路径必须先在目录中登记；VTK 不创建 OpenGL 上下文 |
| 管道断开 | UE 进入 `ui-offline`。三维场景留在内存里，直到新的父 HWND 到来 |

## UE 双视口

一个 `UWorld`。`view0` 和 `view1` 各有一个 `FSceneViewport`、一个 `ULocalPlayer` 和一个 `AMViewCameraPawn`（`UCameraComponent` + `UCesiumGlobeAnchorComponent`）。用户看见的窗口框是 aura Widget。UE 收到 `viewport.attach` 后，在消息里的父 HWND 下创建 `WS_CHILD` 子窗口，并把对应 `FSceneViewport` 画进这个子窗口。子窗口不进任务栏，不带 UE 标题。

shell 在三维 Widget 显示之后发送父 HWND 和客户区像素大小。`NativeViewHost` 在布局时移动这个子 HWND。一次缩放结束后子窗口收到真实 `WM_SIZE`，UE 用新的像素尺寸更新该相机的 `ViewportSize`。Widget 激活时，shell 把焦点设到该子 HWND，三维矩形内的鼠标由 UE 处理。矩形外的树、菜单和时间轴仍由 aura 命中测试。

`ACesiumCameraManager` 关闭自动收集：

- `UsePlayerCameras = false`
- `UseEditorCameras = false`
- `UseSceneCapturesInLevel = false`

每帧把两台相机写入 `AdditionalCameras`，`ECameraParameterSource::CameraComponent`，并带上该子 HWND 的像素尺寸。`view1` 未登记时，Cesium Native 不会为它选块。用户关闭 `view1` 这个 aura Widget 后，shell 发送 `viewport.close`，数组只留 `view0` 一项。

两台相机共享一次原点平移，因此必须看同一地区（总览 + 局部，或透视 + 剖面）。

主视口默认内容：quantized-mesh 地形、影像 overlay、`ACesium3DTileset`、标量体、风场。副视口默认内容：同一地形、影像和瓦片集，加上剖面或第二套传输函数。层开关带 `view0`、`view1` 或 `both`。

## 体渲染与风场

### 标量体

1. 界面发送 `volume.load`（数据集 id、变量名、时次）。消息里没有体素。
2. UE 工作线程调用 `mview_volume.dll`。
3. DLL 用 `vtkXMLImageDataReader` 或 NetCDF 读入 `vtkImageData`，重采样到预算内的规则网格，输出 `float32` 标量和地理范围。
4. 游戏线程上传为 `R16F` 的 `UVolumeTexture`。
5. 主相机的后处理材质做光线步进，用 256×1 传输函数纹理累积颜色，碰到 `SceneDepth` 停止。建筑因此能挡住体。

默认交互预算 **256³ R16F（约 32 MB）**。工作站档 **512³ R16F（256 MB）**。超出预算时 DLL 先降采样再返回。传输函数最多 32 个色标停靠点。

剖面挂在副相机的后处理上，只对 `view1` 生效。

DLL 头文件是纯 C。UE 模块不包含 VTK 头。投影坐标在 DLL 内用 PROJ 把八个角点转到经纬高；Actor 用 `ACesiumGeoreference::TransformLongitudeLatitudeHeightToUnreal` 摆盒子。着色器在单位立方体里步进。

### 风场

U/V/W 由同一 DLL 抽成规则网格。时间维只保留当前时次和下一时次两张纹理。Niagara 在体块局部 ENU 里平流，粒子留在 GPU 上。时间轴换时次时 UE 换采样纹理。

### 体积 DLL 的最小 C 面

```c
MvvGrid* mvv_open_scalar(const wchar_t* path, const char* variable,
                         int time_index, char* err, int err_cap);
MvvGrid* mvv_open_wind(const wchar_t* path, int time_index,
                       char* err, int err_cap);
void     mvv_resample(MvvGrid*, int nx, int ny, int nz);
void     mvv_copy_f32(MvvGrid*, float* dst, size_t count);
void     mvv_corners_llh(MvvGrid*, double llh_out[8 * 3]);
void     mvv_close(MvvGrid*);
```

`llh_out` 是网格八个角的经度、纬度、高度。失败时返回 `NULL` 并写入 `err`。

## mview 界面与 MapLibre Native

本仓库界面用 `build.bat`（GN + ninja，`out/Debug` 与 `out/Release`）。Windows 的 `cc_std` 在 `build/BUILDCONFIG.gn`（当前 `c++14`）。MapLibre Native 使用 C++17/20 和自己的 CMake，因此单独产出 `mview_map.dll`。集成方式对齐上游 `platform/glfw`：`mbgl::RendererFrontend`、`mbgl::Map`、`mbgl::util::RunLoop`，GLFW 窗口替换为 Win32 HWND。Windows 渲染后端为 OpenGL ES 3。

### 线程

| 线程 | 循环 | 职责 |
| --- | --- | --- |
| mview UI | `base::MessageLoopForUI` | Widget、控件、消化管道命令、处理地图回传 |
| DLL 内部 | `mbgl::util::RunLoop` | GL 上下文、瓦片、样式、`queryRenderedFeatures` |
| 管道读 | 阻塞 `ReadFile` | 把完整 JSON 帧 `PostTask` 到 UI 线程 |

UI 线程不调用 MapLibre 的 C++ 对象。DLL 用 `PostMessage` 把结果送到 Widget 的 HWND，由现有 UI 消息循环处理。

### 产品窗口

壳的入口与 `ui/views/examples/base_widget` 相同：创建 `MessageLoopForUI`。第一版用户能看到的窗口都是这个进程里的 `views::Widget`。

| 窗口 | 内容 |
| --- | --- |
| 主窗口 | `views::Menu`、图层树、实体树、`views::Slider` 时间轴、`views::TableView` 属性 |
| 二维窗口 | 一个 `views::NativeViewHost`，挂 MapLibre 子 HWND |
| `view0` | 一个 `views::NativeViewHost`，挂 UE 主视口子 HWND |
| `view1` | 一个 `views::NativeViewHost`，挂 UE 副视口子 HWND |

主窗口用嵌套的 `views::SingleSplitView` 把图层树和实体树分在视口区两侧。二维窗口和两个三维窗口是独立的 aura 顶层窗口，由主窗口的「窗口」菜单打开和关闭。关闭 Widget 只销毁 aura 窗口和它托管的子 HWND，不退出 UE 进程。`view1` 关闭时发送 `viewport.close`。

树和菜单不压在二维或三维的子 HWND 上。必须盖住地图或三维画面的菜单使用单独的顶层 `views::Widget`。

二维 `NativeViewHost` 在布局时同步 MapLibre 子窗口；一次缩放结束后发送真实 `WM_SIZE`。`pixelRatio` 来自 `GetDpiForWindow`。

### 图层树与实体树

两棵树都是 `views::TreeView`，模型在 shell 进程里。它们不是同一份数据。

图层树的一行是一份数据，还没有放进窗口：

| 字段 | 含义 |
| --- | --- |
| `layerId` | 稳定 id |
| `kind` | `tileset`、`terrain`、`imagery`、`volume`、`wind`、`vector` |
| `name` | 显示名 |
| `visible` | 勾选。矢量走 `mvm_set_layer_visible`；其余走 `layer.set` |
| `viewport` | `map`、`view0`、`view1`、`both`。矢量层固定为 `map` |

图层树的二维行来自 `mvm_list_layers`。三维行来自 `catalog.list`。勾选不传输几何。

实体树的一行是场景里的一个实例。UE 是三维实体的权威，shell 只保存显示用的副本：

| 字段 | 含义 |
| --- | --- |
| `entityId` | 稳定 id |
| `parentId` | 空表示根 |
| `layerId` | 来源图层，可空（例如相机） |
| `kind` | `camera`、`tileset`、`volume`、`wind`、`feature` |
| `name` | 显示名 |
| `viewport` | 这个实例出现在哪个三维窗口 |
| `llh` | 代表点的经纬高，用于飞行 |

内建实体只有两台相机，id 固定为 `camera.view0` 和 `camera.view1`。加载 3D Tiles、体或风场时，UE 发送 `entity.upsert`。点选命中的要素在用户执行「加入实体树」后才 upsert 为 `feature`，避免一次拾取就把瓦片里的全部要素灌进树。删除发送 `entity.remove`。欢迎快照带当前实体列表；单条消息仍受 64 KB 限制，超过则用 `entity.list` 分页。

在实体树上选中一行发送 `entity.select`。UE 在对应窗口里高亮该实例，shell 在属性表里显示已有的小属性。选中相机不新开窗口，只把该 aura 三维窗口提到前台。

### 地图 C 接口

```c
MvmMap* mvm_create(HWND parent, int width, int height, double pixel_ratio);
void    mvm_resize(MvmMap*, int width, int height, double pixel_ratio);
void    mvm_destroy(MvmMap*);
void    mvm_set_notify(MvmMap*, HWND target, UINT message);

void    mvm_set_style_url(MvmMap*, const char* url);
void    mvm_set_camera(MvmMap*, double lon, double lat,
                       double zoom, double bearing, double pitch);
void    mvm_get_camera(MvmMap*, double* lon, double* lat,
                       double* zoom, double* bearing, double* pitch);
void    mvm_set_layer_visible(MvmMap*, const char* layer_id, int visible);
int     mvm_list_layers(MvmMap*, char* json, int cap);
void    mvm_add_geojson(MvmMap*, const char* source_id, const wchar_t* path);
void    mvm_query_point(MvmMap*, double x, double y, uint64_t request_id);
```

`mvm_list_layers` 与点选结果均为 JSON，上限 64 KB。业务矢量把 GeoJSON 或 MVT 的路径交给 DLL，由 MapLibre 自己读盘。样式是一份 MapLibre `style.json`，启动时 `mvm_set_style_url`。图层树用 `mvm_list_layers` 填充，勾选只调用 `mvm_set_layer_visible`。

ion token 和带密钥的服务地址放在用户目录配置中，不进前端资源。二维密钥由 mview 进程内的本地反代添加，不出现在样式文件的分发副本里。

## 命名管道

- 名称：`\\.\pipe\mview`
- 服务端：UE，单实例
- 模式：`PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE`
- ACL：仅当前用户 SID
- 游戏线程不调用 `ReadFile`

信封：

```json
{ "v": 1, "id": "uuid", "op": "camera.set", "body": {} }
```

`v` 不是 1 时回复 `schema`。`id` 配对应答，UE 保留最近 64 个 id 去重。失败体为 `{ "ok": false, "error": { "code": "...", "message": "..." } }`。`code` 只使用 `schema`、`not_found`、`busy`、`too_large`、`io`。

| op | 方向 | body |
| --- | --- | --- |
| `session.hello` / `session.welcome` | 界面 → UE → 界面 | 协议版本与构建号；欢迎消息带会话快照、图层和实体页 |
| `viewport.attach` | 界面 → UE | `view0` 或 `view1`、父 HWND 数值、客户区宽高、DPI |
| `viewport.resize` | 界面 → UE | 同一视口的新像素尺寸 |
| `viewport.close` | 界面 → UE | 关闭该视口的子 HWND，并从 `AdditionalCameras` 移除 |
| `catalog.register` / `catalog.list` | 双向 | 路径或筛选；返回 id、kind、bbox、时间范围、CRS |
| `layer.set` | 界面 → UE | `layerId`、`viewport`、`visible`、`opacity` |
| `entity.upsert` / `entity.remove` | UE → 界面 | 实体树增量。`feature` 仅在用户加入后出现 |
| `entity.list` | 界面 → UE | 分页游标；返回一页实体 |
| `entity.select` | 界面 → UE | `entityId`。UE 高亮并把相机飞到 `llh`（相机实体只激活窗口） |
| `camera.set` | 双向 | `origin`（`ui` / `view0` / `view1`）、`seq`、经度、纬度、高度、航向、俯仰 |
| `camera.mode` | 界面 → UE | `free`、`follow-2d`、`follow-3d`、`lock-center` |
| `time.set` | 界面 → UE | `t`、`playing`、`speed`。播放时由界面按 10 Hz 发送 |
| `volume.load` / `volume.tf` | 界面 → UE | 数据集 id、变量、时次；最多 32 个色标停靠点 |
| `wind.load` / `wind.params` | 界面 → UE | 数据集 id、时次；粒子数量、速度系数、拖尾 |
| `pick.result` | 双向 | 经纬高、要素 id；属性子集截断到 8 KB |
| `health.ping` | 双向 | 1 秒一次 |

接收方应用 `camera.set` 后，不把同一 `origin + seq` 发回来。界面重连时先发 `session.hello`，再对仍然打开的三维 Widget 发送 `viewport.attach`。UE 回复内存中的快照：相机、图层开关、当前时次、已加载数据集 id、实体第一页。然后按新的父 HWND 重建子窗口。

目录是 UE 独占写入的 SQLite `catalog.db`。登记路径必须落在允许的工作区根内。UE 用 VTK 或 Cesium 探元数据后写入 bbox 与时间范围。几何和体素不进库。

## 二维与三维同步

对照在 mview 进程内完成：

| 方向 | 规则 |
| --- | --- |
| 地图 → `view0` | `center` 的经纬度作为相机经纬度；`zoom` 按固定表换成高度 |
| `view0` → 地图 | 经纬度写入 `center`；高度按同一张表换成 `zoom` |
| 姿态 | `bearing` 对航向，`pitch` 对俯仰 |

`follow-2d` 启用地图驱动 `view0`。`follow-3d` 启用 `view0` 驱动地图。`lock-center` 只锁中心。`free` 停止连续同步，点选仍可飞行。

不发送的阈值：经纬度 1e-7 度，高度 0.1 米，角度 0.05 度。连续拖动最快 20 Hz。往返验收：经纬度误差 ≤ 1e-6 度，高度误差 ≤ 1 米（对照表分辨率内）。

点选：地图按下时 UI 调用 `mvm_query_point`；DLL 回传要素 id 与经纬度；`TableView` 显示属性，并发送 `pick.result` 让 `view0` 飞行。三维射线命中的 `featureId` 反向高亮 MapLibre 图层。超过 8 KB 的属性留在 mview 侧按 id 另取，不进管道。

二维底图与三维影像是两套请求，共用逻辑图层 id。矢量业务层只存在于 MapLibre。需要三维贴地时另登记一个 3D Tiles 数据集。

## Content 分层

壳、语言、插件和三维渲染分开。做法与 smartgis 的公开嵌入面相同：产品只包含 `content/public`，`content` 不包含壳。

| 层 | 留下什么 |
| --- | --- |
| `app/shell/` | 薄入口（`mview_shell.exe`）。产品 chrome 在 `app/mgis`；实现 `GisContentsClient`、把指针译成 `InputEvent`。 |
| `content/public` | 嵌入合同。见下表。 |
| `content/browser` | 目录会话、管道客户端、glang、`ToolSession` 的组装。不进 public。 |
| `tool/` | 命令、交互、`Workspace`。没有 HWND、没有 UE 类型、没有 MapLibre 类型。 |
| `plugin/` | `plugin.json` 与产品包。只经 `PluginHost`。 |
| `ue/` | 地形、瓦片、体、风、射线命中、相机执行。 |
| `gis/map` | 二维产品 ABI（`mvm_*`）与 `queryRenderedFeatures`。 |

公开头：`gis_contents.h`、`gis_document.h`、`plugin_host.h`、`tool_session.h`、`event_bus.h`、`types.h`、`gis_contents_client.h`、`script_runtime.h`，以及接线用的 `trace_log.h`、`view_host.h`、`debug_agent.h`。

两条轴不合并：

```
mview_shell.exe
  → GisContentsClient + content/public
  → 建 aura 窗口，交出父 HWND

插件
  → 只经 content::PluginHost
  → 不实现 GisContentsClient，不写管道，不包含 MapLibre / UE 头
```

壳在 `browser_main` 里创建窗口。`viewport.attach` 由 `content/browser` 发送。壳通过 `ShellBridge` 提供父 HWND 和「把 `AuxOverlay` 画到某个视图」。content 不认识 `TreeView` 或 `Widget`。

图层树和实体树显示 content 给出的字符串快照。勾选和选中调用 public 方法，不自己组装管道帧。

## Content 接线

对照 smartgis：`content/public` 是唯一嵌入面；`EventBus` 只公布已经发生的事，主线程、非 RPC、无进程单例；指针不在总线上路由。本树路由仍是 `tool::Workspace`（`wheel.zoom` 截断滚轮，`hover.cursor` 不截断移动）。`content::ToolSession` 按 `view_id` 持有 `map` / `view0` / `view1` 三个 `Workspace`，把 `content::InputEvent` 译成 `tool::InputEvent` 再 `dispatch`。路由返回之后才 `publish(GestureApplied)`，带 `consumed`。订阅者不能把总线当成 `InputRouter`。

```
子 HWND 指针 / 滚轮
  → content::InputEvent
  → tool::Workspace::dispatch
  → GestureApplied + TraceLog
  → 底栏 console / trace
```

视口合同是 `content::ViewHost`：父 HWND 进，子 HWND 出。后端三种：`flycube`、`maplibre`、`ue`。MapLibre 与 UE 这一刀明确返回失败并写 trace `backend.unavailable`，不创建子窗口。`flycube` 是 Cesium / UE 到位之前的三维子窗口，由 `app/mgis` 在 `StartSession` 里用 `SetBackendHooks` 装上自己的 OpenGL 地球（`GlobeView`）。`app/shell` 只是孵化器，可以调用 `mgis::ShowWorkbench`，mgis 不包含 shell，也不等 shell 先登记钩子。未装钩子时 content 自己建一个标注为 Cesium / FlyCube 的子 HWND，测试不链 GL。地球的拖动和滚轮由 mgis 的桥执行相机，content 只记录手势事实。`view3d.trackball` 在 `view0` 上激活后，鼠标才被路由消费。

`GisContents` 拥有本会话的 `TraceLog`（环形，最多 256 行）。通道 `console`、`perf`、`agent`。关键路径用 `TraceScope` 记耗时（attach、按下、滚轮、`ExecLine`），移动只进 console。`app/mgis` 订阅后刷新底栏三个 Tab。content 不包含 `ui/views`。

`DebugAgent::ExecLine` 是进程内核心，不做 smartgis 那套回环套接字、Python 工人或界面点击。内置行：`help`、`trace`、`suite <path>`、`attach scene`。`attach` 只接受一个 token `scene`。其余一行交给 glang 替身。`set_contents` 后 suite / 替身动词走 `GisContents::capability_host()`（场景、图层、实体槽齐全）；未绑 contents 时仅有 agent 自带的 command / activate / pointer 槽。替身动词：`mark`、`command`、`activate_tool`、`pointer`、`scene_mode`、`expect_camera`、`expect_layer`、`expect_entity`。`scene_mode` 取 `scene2d`、`scene3d` 或 `columbus`。`expect_camera scene <lon> <lat> <height>` 在 `SCENE2D` 下比较经纬度（绝对误差 `1e-6` 度）和高度（1 米）；`SCENE3D` 的 `expect_camera scene` 在轨道位移大于 0 时通过。未挂接时返回明确错误，不再用 “not available” 表示相机未实现。`expect_layer <id>` 查 `Scene::HasLayer`（本刀：`basemap` / `groundwater`）；`expect_entity <id>` 查当前 mode 下 Pick 会认的 feature（`m0` / `g0`）。`capture` 仍返回明确错误。`command` / `activate_tool` / `pointer` 的槽接到同一个 `ToolSession`。未知动词失败时，该文件的 mark 一个都不写。

产品窗由 `mgis::ShowWorkbench` → `StartSession` 自己登记 FlyCube 钩子，再把 `view0` 子 HWND 交给 `NativeViewHost`；同一次 `StartSession` 里 `DebugAgent::Bind` + `set_contents` + `Start()`（MCP `127.0.0.1`，发现文件 `%LOCALAPPDATA%\MView\debug-agent.json`）。二维页保持占位。`app/shell` 的卫星视口仍可直接使用 `GlobeView` 做渲染检查，那条路径不负责产品接线。

验收：`build.bat debug session_wire`。

## glang

glang 在 `content/browser/glang.runtime/`，顶层三块：

| 目录 | 职责 |
| --- | --- |
| `language/` | 套件加载与诊断（含非 `.py` 文本） |
| `exec/` | 行解释器（`suite_runner`）+ 可选进程内 CPython（`exec/py/`） |
| `host/` | 把文档、视图、工具、插件填进 `CapabilityHost` |

公开入口只有：

```cpp
namespace content {
bool run_suite_script(CapabilityHost& host,
                      const std::string& path_utf8,
                      std::string* error);
}
```

`run_suite_script` 按扩展名分流：

| 路径 | 行为 |
| --- | --- |
| `*.py` | `py::Interpreter::exec_file` → 加载模块 → 调 `main()`；能力经内建模块 `glang.plugin` → `eval_script_call` → `CapabilityHost` |
| 其它 | 读 UTF-8 文本，走行解释器（`run_suite_text`）；DebugAgent `ExecLine` 与过渡套件仍用这条 |

### CPython 嵌入（GN）

- `build/python.gni`：`has_python`（默认 true）、`python_root`（空则 `%LOCALAPPDATA%/Programs/Python/Python312`）。
- **Debug** 必须匹配 debug CRT：`python312_d.lib` + `python312_d.dll` + `include/Python.h`。缺则 **软降级**（`python_enabled=false`，编桩，不链官方 `/MD` 的 `python312.dll`，避免混 CRT）。
- **Release** 用官方 `python312.lib` / `python312.dll`。缺则同样软降级。
- 强制关嵌入：`has_python=false`。完整前缀：设 `python_root`。
- `//content` 在 `python_enabled` 时定义 `HAS_PYTHON`、`PYTHON_HOME`，并链对应 import lib。

壳填 `CapabilityHost` 槽后调用 `run_suite_script`。content 不包含 `ui/views`。

具名动词经 `eval_script_call` 进能力表，不按场景增加 opcode。第一版登记：`mark`、`expect_camera`、`expect_layer`、`expect_entity`、`activate_tool`、`pointer`、`command`、`attach`、`scene_mode`、`capture`。地图和三维的差异在 `view_id`（`map` / `view0` / `view1` / `scene`），不在第二套脚本语言。

管道 JSON 是 `host/` 下面的传输。代理和人写的是套件，不是管道帧。

验收：`build.bat debug session_wire`（无 debug CPython 时桩路径绿）；有 Release embed 时再跑 `build.bat session_wire` 覆盖 `.py` 套件（`testdata/mark_ready.py`）。

## 插件

**方案锁：** C ABI v1（`plugin/api/mview_plugin_api.h`）；壳持 `plugin::Registry` + `plugin::Loader`；`PluginHost` 挂在 `GisContents` 并 `set_tool_session`；贡献面含 command / Interaction factory / menu / dock；Interaction 经桥接进入各视图 `tool::Workspace`；右栏 `ToolPluginPanel` 列命令并可点跑。

| 角色 | 落点 | 职责 |
| --- | --- | --- |
| C ABI | `plugin/api/mview_plugin_api.h` | DLL 只见此头；导出 `mview_plugin_contribute`（必选）与可选 `mview_plugin_execute` |
| Registry | `plugin/registry.*` | 扫 `plugin.json`（`id` / `name` / `commands` / 可选 `dll`） |
| Loader | `plugin/loader.*` | `LoadLibrary` → `GetProcAddress` → 填 `MviewPluginApi` 回调进 `PluginHost`；version 不符跳过并 trace |
| PluginHost | `content/public/browser/plugin_host.h` + `content/browser/plugin/` | **不** `LoadLibrary`；桥 Interaction；`run_processing` / `ActivateInteraction` |
| 壳 | `app/mgis/session` + `ToolPluginPanel` | 只碰 `content/public`；扫 `UI/plugins` 回落 `plugin/sample`，DLL 旁路 `out/Debug` |
| 示例 | `plugin/sample` → `plugin_sample.dll` | 贡献 `plugin.sample.run` 等；执行写 `out/harness/plugin.sample/mark.json` |

`run_processing` 是算法入口。第一版在壳进程内执行。插件不拿到管道，也不拿到 `mbgl::Map`。面板控件由壳创建，content 不包含 Views。

**Diagram:** [`../diagrams/plugin-wiring.html`](../diagrams/plugin-wiring.html)

## 工具交互

参照 smartgis 的会话分发：命令与地图工具分开，输入不走 `EventBus`，文档修改不发生在 `Interaction` 里。

每个视图一个 `tool::Workspace`：`map`、`view0`、`view1`。焦点所在的 aura 窗口决定当前 `view_id`。命令带 `view_id`。

```
子 HWND 的指针 / 滚轮 / 键
        │  壳译成 content::InputEvent（该视图像素坐标）
        ▼
tool::InputRouter
  常驻：wheel.zoom、hover.cursor
  然后：当前独占 Interaction
        │
        ├─ 导航类  → ViewSink
        │              map  → mvm_set_camera
        │              view0/view1 → camera.set
        ├─ 选择类  → 命中后端
        │              map  → mvm_query_point
        │              三维 → pick.request
        └─ 绘制类  → AuxOverlay 草稿
                       提交时 content::EditSession::commit
                       成功后 EventBus：SelectionChanged / ExtentChanged
```

`InputRouter` 的顺序与 smartgis 相同：常驻处理器按登记顺序，然后是栈顶工具。返回 true 的处理器截断后续。`hover.cursor` 对移动返回 false，当前工具仍能看见移动。`wheel.zoom` 只在滚轮上返回 true。常驻 id 不在命令表里，不能 `execute`。

独占工具用 `activate` 换掉当前工具。`push` / `pop` 保留上一件工具。空栈表示没有独占工具。

第一版命令：

| id | 种类 | 落到哪里 |
| --- | --- | --- |
| `view.pan` / `view.zoom_in` / `view.zoom_out` | 激活交互 | 当前视图的 ViewSink。二维改 zoom/center，三维改高度或视距 |
| `view.full` / `view.refresh` | 命令 | 不占鼠标 |
| `view3d.trackball` / `view3d.fps` | 激活交互 | 仅 `view0` / `view1`。在二维视图上 `execute` 返回 false |
| `selection.point` / `selection.rect` | 激活交互 | 二维 `queryRenderedFeatures`；三维射线。结果进 `EditSession` 选择，再发 `SelectionChanged` |
| `selection.clear` | 命令 | 清选择，不激活工具 |
| `edit.append.point` / `linestring` / `polygon` | 激活交互 | 只产草稿。提交后二维写成临时 GeoJSON，三维发 `entity.upsert` |

橡皮筋不能画在 aura 里盖住子 HWND。`live_preview()` 仍是像素折线或矩形。content 的视图适配器把它交给后端：二维一条临时 GeoJSON，三维一条 `draft.set`（点在 content 里换成经纬高）。工具代码不调用这两条后端。

MapLibre 不登记自己的相机手势。UE Pawn 不接收产品指针。导航是 `view.pan` 和 `view3d.*` 这两件 Interaction。命中只在选择工具的 `on_input` 里向后端要一次结果。

插件新增工具时，向 `PluginHost` 贡献命令 id 和 `Interaction` 工厂。工厂里同样只有 `on_input` 和草稿，没有引擎类型。

glang 的 `activate_tool`、`pointer`、`command` 往指定 `view_id` 注入同一条 `InputEvent` / `execute` 路径。套件不向子 HWND 投鼠标，除非该套件明确在测壳的命中测试。

## 代理迭代

套件是唯一的场景正文。代理、控制台和回归共用 `run_suite_script`。

一轮迭代固定六步：

| 步 | 产物 |
| --- | --- |
| 契约 | 套件列出 `required_marks` |
| 驱动 | campaign 按阶段顺序调用 `run_suite_script` |
| 交互 | `activate_tool` / `pointer` / `command` |
| 记录 | `out/harness/<suite>/mark.json`、各 aura 窗口的 BMP、`pipe.log` |
| 评分 | 缺任何一个 `required_marks` 即失败并停写后续 mark。BMP 旁的 `score.json` 只作记录，除非该套件写明 `score_id` 参与失败 |
| 修订 | 改套件或插件脚本。缺的能力补一个具名 `eval_*`，并在同一套件里调用它 |

`out/harness/<suite>/` 是代理能读的全部结果。不另开壳内调试通道。

## 第三方 pin

`third_party/manifest.json` 仍是唯一 pin。Chromium 快照保持 `source_policy: vendored`。

### smartgis 工具 / GIS 库（按需）

- 清单已并入 smartgis 允许的包（gdal、geos、PROJ、pdal、rapidjson、asio、CLI11 等）。
- **排除：** `cef`、`webview2`、`windows_app_sdk`、`bcg`（及同类壳 UI）。
- **zlib：** 保留 Chromium vendored `.src/zlib`；不覆盖。
- **gtest：** 产品测试继续用 `//testing:gtest`（Chromium 快照）；清单里的 `googletest` 仅供按需 CMake 安装。
- 本机：`py -3 third_party/tools/sync_from_smartgis.py` 建 `.src/<pkg>` junction；`.install` 不整树替换。
- 默认产品 GN **不**链 GIS 大库；需要时 `//third_party:<name>` + `build.bat t <pkg>`。
- `content/browser/ipc` 直接用 RapidJSON（`Document` / `Value`），助手 API 对齐 smartgis `sdbd_json` / `args_json`（`pipe_json_*`），无自制值模型。

### 引擎类 pin（不进 `ui/` 默认图）

| 清单名 | 钉扎 | 落点 | 示例 |
| --- | --- | --- | --- |
| `maplibre-native` | `ios-v6.30.0`（与 smartgis 相同） | `.src/maplibre-native`，可联接已有检出 | `third_party/maplibre/example` |
| `unreal` | UE 5.6，`UE_5_6_ROOT` | 不进仓库 | `third_party/unreal/example` |
| `cesium-unreal` | Cesium for Unreal 2.30，`CesiumForUnreal-56` 源码包 | `.src/cesium-unreal` | `third_party/cesium-unreal/example` |
| `cesium-flycube` | 本仓库编写，不进清单 fetch | `third_party/cesium-flycube` | `example/dual_view`、`example/ellipsoid_city`、`example/google_earth`、`example/browse_city` |
| `cesium-native` | `v0.64.0` | `.src/cesium-native`，仅示例 | `third_party/cesium-native/example` |
| `vtk` | `v9.5.2`，Qt / Rendering / Views 关闭 | `.src/vtk` | `third_party/vtk/example` |

产品进程里的 cesium-native 只来自 Cesium for Unreal 插件。清单里的 `cesium-native` 不链进壳，也不链进 UE 模块。VTK 示例只写规则网格，不创建 OpenGL 窗口。MapLibre 示例跑上游 `mbgl-render`，像素不进 `ui::Layer`。

### Cesium for FlyCube

**Status:** landed (库层闭环刀 1) · **Updated:** 2026-10-23  
**Diagram:** [`../diagrams/cesium-flycube-align.html`](../diagrams/cesium-flycube-align.html)

`third_party/cesium-flycube` 是对照 Cesium for Unreal 运行时写的 FlyCube 绑定，不进 `manifest.json`，也不进壳的 GN 图。

它保留 Unreal 插件里产品实际用到的几件事：一个 WGS84 地理参考、`AdditionalCameras`（不调用已废弃的 `AddCamera`）、椭球 / URL / ion 三种瓦片源，以及 `IPrepareRendererResources`。FlyCube 没有 Actor，相机参数直接写在 `Camera` 上。世界坐标是右手系、米、X 东 / Y 北 / Z 上。两台相机 ECEF 相距超过 100 km 时返回 `origin_conflict`，原点不动。

#### 库层闭环（方案 B / 路径 2 · 刀 1）

| 项 | 约定 |
| --- | --- |
| `TileMesh` | ECEF 位置 + `_CESIUMOVERLAY_0`/`TEXCOORD_0` + 可选法线 + `RasterDrape[]` + 要素 |
| `PrepareRendererResources` | 拷贝 `ImageAsset` → `RasterImage`；`attachRaster` 挂到对应 `TileMesh` |
| `FrameToPresent` / `TileMeshToPresent` | 库内把 drape 采样成顶点色；CPU PNG 与 DX12 共用 |
| 验收 | 全部 `example/*.cc`；默认 debug colorize；`--imagery` 可选 Esri URL |
| Helper | `CaptureDrapedEllipsoid` / `FrameToPresent` |
| 非目标（本刀） | 真 GPU 纹理绑定、把完整 tileset 链进壳 GN；产品 DX12 子窗旁路 `mview_flycube.dll`（§ Scene 呈现器） |

网格先落在 CPU，再上传到 FlyCube DX12。全部 example 经椭球 + overlay 闭环；领域示意（地质 / 地下水 / 洪水 / 风场）叠在 draped 地形上。壳不链接本目录。

### 示例：Google Earth 类浏览（`cesium_flycube_google_earth`）

**Status:** landed · **Updated:** 2026-10-23

| 项 | 约定 |
| --- | --- |
| 底座 | `CaptureDrapedEllipsoid`（上海椭球 + debug colorize；`--imagery` → Esri URL） |
| 手感 | 开场从约 280 km 飞到约 28 km（本地椭球瓦片，非整球）；轨道 / 平移 / 缩放 |
| 产物 | `google_earth.png` / `google_earth_near.png`；`--check`；`--present` |
| 非目标 | 手搓球面 / 本地 SRTM 贴片、ion 密钥、进壳 GN |

`browse_city` 同一底座，城市尺度飞行。

### 示例：UE Google Earth 类（`MViewGlobe` / `cesium-unreal/example`）

**Status:** draft · **Updated:** 2026-10-22

| 项 | 约定 |
| --- | --- |
| 大气与飞入（A） | `ACesiumSunSky`；`AMViewCameraPawn` 从约 20 000 km 经 `UCesiumFlyToComponent` 飞入上海 Bund |
| 倾斜摄影（B） | `CESIUM_ION_TOKEN` 时默认 ion **2275207**（Google Photorealistic，`ShowCreditsOnScreen`） |
| 无 token / 国内 | 椭球 + **MapLibre demotiles** MVT（`demotiles.maplibre.org` → `CesiumVectorTilesRasterOverlay`）；不默认 Esri |
| 入口 | `.\run-demo.ps1 earth`；覆盖：`CESIUM_ION_ASSET`、`MVIEW_TILESET_URL`、`MVIEW_VECTOR_TILES_URL`、`MVIEW_IMAGERY_URL` |
| 非目标 | token 入库；与 World Terrain 叠层混用；进壳 GN |

## Mgis 工作台壳（`app/mgis`）

**Status:** landed (SmartGIS chrome + markup Yoga zones) · **Updated:** 2026-10-24  
**Diagram:** [`../diagrams/mgis-workbench-chrome.html`](../diagrams/mgis-workbench-chrome.html) · [`../diagrams/mgis-theme.html`](../diagrams/mgis-theme.html) · [`../diagrams/views-declarative-markup.html`](../diagrams/views-declarative-markup.html) · [`../diagrams/scene-presenter-registry.html`](../diagrams/scene-presenter-registry.html)  
**Plan:** [`../plans/2026-10-23-mgis-smartgis-chrome-align.md`](../plans/2026-10-23-mgis-smartgis-chrome-align.md) · theme: [`../plans/2026-10-24-mgis-theme-chrome.md`](../plans/2026-10-24-mgis-theme-chrome.md) · markup: [`../plans/2026-10-20-views-declarative-markup.md`](../plans/2026-10-20-views-declarative-markup.md)

对照 smartgis（`src/ui/gis` + `src/app/views`）**布局与字段**，本仓在 `app/mgis` 重做面板，**不**移植 smartgis View 实现。产品主路径是**单主窗 IDE 布局**。

### 主题与 chrome 视觉（1+2）

**Status:** landed · **Updated:** 2026-10-24

全量对标 SmartGIS 的长期路线拆为：①主题基座 ②chrome 视觉 ③Catalog ④Ambox 工具 ⑤右栏 ⑥底栏。本刀落地 **①+②**。

| 决策 | 选择 |
| --- | --- |
| 默认包 | **light**；可切 dark；持久化 `%LocalAppData%\MView\ui_theme_id.txt` |
| 内核位置 | `ui/views/theme/`（`Theme` / `ThemeService` / `ThemeObserver`），API 形对齐 SmartGIS，命名跟 Chromium（`PascalCase`） |
| 双轨 | C++ `Theme::Current()` 自绘 + markup `theme(token)`（`CssParser`）；切换时 `ThemeService` → Widget bridge → `ThemeChanged` / `OnThemeChanged`，`WorkbenchContents` 重刷 slot 背景 |
| 菜单 | View → Theme → Light / Dark（RADIO） |
| 非目标（本刀） | Catalog/Ambox/Inspect 业务实质；链 smartgis DLL；改 Chromium `ThemeProvider` 语义 |

**Chrome 分区**由 `workbench.ui.xml` + Yoga 声明式布局驱动（DIP 长度经 `StyleScale` 乘设备缩放）；`WorkbenchContents` 不再对手写四区 `SetBounds` 像素算术负责。真实控件仍挂到 markup **slot**（`FillLayout`）。

### 布局（对齐 SmartGIS Views）

| 区 | 内容 |
| --- | --- |
| 顶 | 菜单占位 + `MapToolbar`（选择 / 绘制 / Undo…）；时间轴 `Slider` 移出主带（可后置） |
| 左 | `CatalogTabs`：**Layers** \| **Sources** \| **Maps**（各一棵 `TreeView`；Sources/Maps 可占位） |
| 中 | `TabbedPane`：**Map** \| **3D**；两页均为 `ViewportSurface`；仅当前模式子 HWND `Attach`，另一页 `Detach` |
| 右 | `ToolPluginPanel`：GroupBar **Inspect** \| **Map** \| **Process** \| **Extend**；Extend 下 Atmosphere / Plugins；Plugins 可筛选、Enable/Disable/Run |
| 底 | **Output** \| **Console** \| **Trace** \| **Memory** \| **Agent**（Output/Console/Trace/Agent 订 `TraceLog`；Memory 占位） |
| 底缘 | `StatusBar`：message \| scale \| CRS \| XY |

### 语义与联动

- **Layers** = 2D catalog 主树；**Sources** / **Maps** = 目录占位（本刀不接真文档模型）。
- 中心：**Map** ↔ `SceneMode::kScene2D`；**3D** ↔ `kScene3D`（`view1` 后置）。
- 左栏 Tab **不再**与中心双向强制同步（与截图一致：catalog 与视口模式独立）。
- Tab 切换：`SelectViewTab` → `GisContents::SetSceneMode` → 活动页 Attach → `Resize` → `Present`。
- 工具条：`GisContents::ActivateSceneTool(id)` → `ToolSession::activate(id, kViewIdScene)`（壳不 include `content/browser`）。
- Plugins：**Run** → `PluginHost::run_processing`；Enable/Disable 为壳内选中态闸门。
- 状态栏：订 `EventBus`（`GestureApplied` → XY；`ExtentChanged` → 粗算 scale）；CRS 默认 `EPSG:4326`；message 跟 present 模式。
- 右栏、底栏、状态栏全局，不随视口 Tab 换内容。

### 目录

| 路径 | 职责 |
| --- | --- |
| `app/mgis/` | 产品树：库 `//app/mgis:mgis_lib` + PE `//app/mgis:mgis`（`mgis.exe`），命名空间 `mgis` |
| `app/mgis/app/` | `main.cc` + cmdline 开关（参照 smartgis `app/views/app`） |
| `app/mgis/harness/` | `--harness-chrome` / `--render-check` 卫星 ViewportWindow |
| `app/mgis/layout/` | `--check-layout` 安装布局门禁 |

```
app/mgis/
  BUILD.gn                   # :mgis_lib 库 + :mgis exe
  app/
    main.cc
    cmdline/launch_switches.*
  harness/
  layout/
  view_id.h
  workbench/                 # 主窗组合、markup inflate、SelectViewTab
    workbench.{h,cc}
    workbench.ui.xml / .ui.css
    workbench_markup.{h,cc}  # embedded XML fallback
  horizon/
    map_toolbar.{h,cc}
    catalog_tabs.{h,cc}
    tool_plugin_panel.{h,cc}
    bottom_panel.{h,cc}
    status_bar.{h,cc}
    viewport_placeholder.{h,cc}  # 保留；产品中心页不再使用
  viewport/
    viewport_surface.{h,cc}
    globe_view.{h,cc}
  session/
    session.{h,cc}
```

原则：`horizon/` 只放 View 壳；`workbench/` 用 markup/Yoga 分区并接线；视口挂载与会话进 `viewport/`、`session/`。工作台**不**拥有卫星 Widget。顶层 `launcher/` 与 `app/shell/` 已删除。

产品会话走 `GisContents::AttachView` → `scene_child_hwnd_for(mode)` → 活动页 `NativeViewHost`。呈现器路径不变（Scene 注册表）。

### 本刀非目标

Atmosphere 真参数面板；Memory 分配跟踪；Sources/Maps 真文档模型；移植 smartgis View；`view1`；UE 断管热切 FlyCube。

### 与多 Widget 表述的关系

上文「二维 Widget / view0 / view1 独立 Widget」仍描述 **P2 挂接与崩溃域** 的 HWND 父级策略；**产品可见壳** 以本节单主窗为准。P1 mark 名可暂时仍由 harness 侧卫星窗满足，直至套件改写。

## Scene 呈现器注册表（方案 1）

**Status:** landed (验收 C + FlyCube 真呈现) · **Updated:** 2026-10-23  
**Diagram:** [`../diagrams/scene-presenter-registry.html`](../diagrams/scene-presenter-registry.html)  
**验收：** C — 首帧 + 导航 + 拾取 + harness / console marks；3D 回退非 GDI stub。

### 决定

| # | 决定 |
| --- | --- |
| 1 | **方案 1**：一个 `Scene` + `SceneMode`；产品用可插拔呈现器替换 `FakeScene` 绘制。不做壳内双 `ViewHost`。 |
| 2 | 2D = MapLibre（`mview_map`；失败则 mercator 软回退）。3D = **先 UE 管道**，超时/连接失败 → **FlyCube** 进程内子窗。 |
| 3 | `AttachView` 仍只接受 `kViewIdScene`。 |
| 4 | `content/public` 不含 MapLibre / UE / FlyCube 头。引擎代码只在 `content/browser`（或既有 `ipc`）。壳只 `#include content/public/browser`。 |
| 5 | `session_wire` 保持默认 `FakeScene`；产品 `mview_shell` / `mgis` 链接 `//content:product_presenters` 注册 `SceneImpl`。 |

### 分层

```
app/mgis  SelectViewTab / 双 ViewportSurface
    ↓ public
GisContents  AttachView / SetSceneMode / Pick / scene_child_hwnd_for
    ↓ browser
SceneImpl（相机、图元、拾取、草稿）按 SceneMode 选呈现器
    ├─ SCENE2D → MapLibrePresenter（mview_map；mercator soft fallback）
    └─ SCENE3D → 试 PipeClient → UePresenter；失败 → FlyCubePresenter
```

### HWND / 模式 / 回退

- 启动：创建 2D 呈现器子窗；试连 `\\.\pipe\mview`；失败 trace `presenter.ue_failed`，建 FlyCube，trace `presenter.fallback flycube`。
- 两页 `ViewportSurface`；仅活动模式 HWND `Attach`；另一页 `Detach`，**不** Destroy。
- MapLibre 失败 → mercator 软回退 + trace。
- `MapLibrePresenter` 只走 `mview_map` 导出的 **C ABI**（`mvm_*` / `mvm_hwnd`）；`MapSurface` 为 DLL 内部类型，产品侧不得直接链接其 C++ 符号。
- **`FlyCubePresenter` 真呈现**：不依赖 `app/mgis`。优先 `SetBackendHooks(kViewBackendFlyCube)`；否则 `FlyCubeHost`：旁路加载 `mview_flycube.dll`（`//render/rhi/flycube` C ABI，FlyCube DX12 → 子 HWND）；DLL 缺失或 DX12 失败则 content 内 OpenGL  lit 地球（与原 GlobeView 同系网格）。traces：`presenter.3d flycube` / `presenter.fallback flycube` / 可选 `presenter.flycube_backend flycube-dx12|flycube-gl`。无海军蓝 GDI stub。
- UE 会话中途断管：**本刀不**热切 FlyCube。
- 非目标：`view1`、多 UE 相机、把 FlyCube 链进无关 example 目标。

### 导航 / 拾取 / marks

- 导航经 `ToolSession`：SCENE2D `view.pan` / `wheel.zoom` → MapLibre 相机；SCENE3D `view3d.trackball` → UE `camera.set` 或 FlyCube Camera；`selection.point` / `edit.append.point` 在 SCENE2D|SCENE3D 可激活；错模式工具返回 false。
- 拾取 API：SCENE2D → `primitive:basemap:m0`；SCENE3D → `primitive:groundwater:g0`（本刀种子 id）。
- 选择成功时 harness mark：`pick.2d` / `pick.3d`（与 `Pick()` 字符串分开）。
- Console / harness marks：`session.ready`、`presenter.2d`、`presenter.3d`（`ue`\|`flycube`）、可选 `presenter.fallback`、每模式一次 `expect_camera`、`pick.2d` / `pick.3d`。
- `session_wire` + `FakeScene` 保持绿，不要求真实呈现器。

## 目录与安装布局

```
app/mgis/                      产品工作台 + 单 PE 入口（mgis.exe）
content/public/browser/        唯一嵌入面（含 ViewHost hooks）
content/browser/
  session/                     GisContents 组装、EventBus、TraceLog、types
  viewport/                    ViewHost 实现、map/Scene
  tooling/                     ToolSession
  ipc/                         命名管道（原 pipe/）
  script/                      glang language/exec/host + CapabilityHost
  agent/                       DebugAgent、devtools MCP
  plugin/                      PluginHost 实现
tool/                          Workspace、命令、交互
plugin/                        注册表与产品包
vista/pass/map/                二维 MapPass（vista_map.dll）
vista/pass/world/              三维 WorldPass（vista_world.dll）
third_party/vtk/mview_volume/
third_party/maplibre/mview_map/  # 可选 mview_maplibre embed
ue/
testing/tools/harness/         阶段套件
```

`app/mgis/` 只 `#include "content/public/..."`，不得包含 `content/browser/**`。产品 PE 为 `//app/mgis:mgis`。不修改 `ui/aura` 合成器。`tool/` 不依赖 `ui/views`。

## content/browser 深度重构

**Status:** landed (directory + public); object-split pending  
**Date:** 2026-10-22  
**Approach:** 一次大搬家 + 同步升 public（方案 2）。行为冻结；允许有意切断壳对 `browser/` 的违规 include。

### 合同

1. 产品壳只碰 `content/public/browser/*`。`ViewHost` / `ViewBackendHooks` 在 public；`ToolSession`、`CapabilityHost`、glang、管道仍在 browser，测试可直接 include。
2. 对外 GN 仍是 `//content:content`；内部按 lane 摆源，不强迫产品改 deps。
3. 语义不变：三 Workspace、`GestureApplied` 在 dispatch 之后、父子 HWND、`run_suite_script`。
4. 有意边界修复：删除/禁止 `content/public/view_host.h`、`content/public/types.h` 这类错误路径；mgis 改为 `content/public/browser/...`。
5. 大对象拆分（`GisContents` 组装、`ue_peer`）为紧随的第二刀；本刀以目录与 public 面为主。

### 门禁

- `session_wire` / `viewport_gate` / `harness_campaign` 绿。
- `app/mgis` 源文件不得出现 `#include "content/browser/`。

安装布局：`mgis.exe`、`Engine/`、`UI/vista_map.dll`、`UI/vista_world.dll`、`Bin/mview_volume.dll`。插件放在 `UI/plugins/<id>/`。日志在 `%LOCALAPPDATA%\MView\logs\`。`--check-layout` 挂在同一 `mgis.exe` 上。

## 产品入口合并（单 PE）

**Status:** landed · **Updated:** 2026-10-24

### 决定

| # | 决定 |
| --- | --- |
| 1 | 唯一产品 PE：`mgis.exe`（`//app/mgis:mgis`）。删除顶层 `launcher/` 与 `app/shell/`。 |
| 2 | 单进程：无独立启动器、不 `CreateProcess` 拉界面子进程。Job Object / 壳崩溃重启为后续。 |
| 3 | 默认无参起工作台；`--harness-chrome`、`--render-check`、`--check-layout` 同 PE。 |
| 4 | 目录对齐 smartgis 轻量形态：`app/mgis/app/`（main + cmdline）、`harness/`、`layout/`。 |
| 5 | 工作台静态库为 `//app/mgis:mgis_lib`；`//:shell` / `//:launcher` 为兼容别名指向 `//app/mgis:mgis`。 |

## 阶段与门控

campaign 顺序就是阶段顺序。进入下一阶段之前，前面每个套件的 `required_marks` 必须在同一次 campaign 里全部落地。某一套件失败时，不写它后面的 mark，也不启动下一套件。

每一阶段另外有一条永久门禁：`app/mgis/` 与 `plugin/` 的包含检查失败则该阶段失败。`app/mgis` 不得包含 `content/browser`、`tool/` 的内部头、UE 头、MapLibre 头。`content` 不得包含 `ui/views` 或 UE 头。`tool/` 不得包含 HWND、UE、MapLibre。

| 阶段 | 套件 | 通过条件 |
| --- | --- | --- |
| P0 边界 | `browser.empty` | `run_suite_script` 写上 `session.ready`。管道契约测试（合法帧、超长帧、错误版本、断线）不启动 UE 即通过。`tool_dispatch_test` 在没有窗口的情况下证明：未知命令不广播、`wheel.zoom` 截断滚轮、`hover.cursor` 不截断移动、`Interaction` 不能直接改选择表 |
| P1 壳 | `browser.chrome` | mark：`widget.main`、`widget.map`、`widget.view0`、`widget.view1`、`tree.layer`、`tree.entity`。主窗口 BMP 落地。此阶段不启动 UE |
| P2 挂接 | `browser.viewport` | `viewport.attach` 之后 `view0` 与 `view1` 都有子 HWND。关闭 `view1` 的 Widget 后 mark `cameras=1`，UE 进程仍在。结束壳进程期间 UE 帧计数继续增加；新壳再次 `viewport.attach` 后 mark `viewport.reattach` |
| P3 二维与导航工具 | `browser.map` | 底图加载。套件 `activate_tool(view.pan)` 后注入拖动，`expect_camera` 达到经纬度 ≤ 1e-6 度、高度 ≤ 1 米。滚轮只走 `wheel.zoom`。`pipe.log` 与地图日志里没有绕过 Workspace 的手势。关闭矢量图层时管道载荷只有图层 id。窗口 DPI 写入 mark `pixel_ratio` |
| P4 三维与实体 | `browser.world` | 两个窗口的瓦片都随各自相机变细。`view3d.trackball` 在 `map` 上执行返回 false。关闭 3D Tiles 后 UE mark `tileset.hidden`，实体树去掉对应实例。套件若把两台相机放到超过产品距离上限的两地，宿主返回 `origin_conflict` 且不移动原点 |
| P5 体与风 | `browser.field` | 64³ 合成场 mark `volume.ready`。色标消息没有体素数组。UE 写 `volume.depth_stopped`（光线在场景深度处停止的采样数 > 0）。`time.set` 只改变风场时次，`pipe.log` 里没有粒子缓冲 |
| P6 选择与插件 | `browser.pick`、`plugin.sample` | `selection.point` 在二维和 `view0` 各命中一次，两边 mark 同一 `featureId`。`entity.select` 后相机落到该经纬高（同一误差）。示例插件经 `plugin.json` 装入，贡献的命令经 `PluginHost` 执行并写上 `plugin.sample.ran`。插件包含检查通过 |
| P7 安装 | `browser.install` | 干净目录启动签名后的布局，P1 的窗口 mark 与 P2 的 `viewport.reattach` 在安装树下再跑一次 |

P3 起，导航与选择必须走「工具交互」一节的路径。只把相机或命中接在引擎回调上，而套件 mark 不到 `activate_tool`，视为该阶段失败。

## 第一版不包含

两个洲同时满精度浏览、管道传输体纹理、UE 内流体求解、把 MapLibre 或三维画面合成进 aura 的 `ui::Layer`、用 Slate 做图层树或实体树、用引擎自带操控代替 `tool::Workspace`。

## 中国底图 · 手势 · FlyCube Google Earth（方案 1）

**Status:** landed (真数据源) · **Updated:** 2026-10-25  
**Diagram:** [`../diagrams/china-map-gesture-flycube.html`](../diagrams/china-map-gesture-flycube.html)  
**Plan:** [`../plans/2026-10-25-real-map-dem-sources.md`](../plans/2026-10-25-real-map-dem-sources.md)（刀 1–3 见 [`../plans/2026-10-24-china-map-gesture-flycube.md`](../plans/2026-10-24-china-map-gesture-flycube.md)）

### 验收（已锁）

| 刀 | 验收 |
| --- | --- |
| 排期 | 规格一次写齐；实现 **2D → 手势 → 3D** |
| 2D | 默认 **OpenFreeMap Bright**（全球 OMT，中国区路网/水体可读）；MapLibre disk cache 混合缓存；`MVIEW_MAP_STYLE=offline` 才回退示意 GeoJSON；无天地图/GCJ |
| 手势 | `view.pan` + `wheel.zoom`（2D）与 `view3d.trackball`（3D）；双指→滚轮原语、右键拖→倾斜/轨道；**不扩** Interaction 表 |
| 3D | draped 椭球 + 默认 **Terrarium DEM 色** + **Esri 影像**（可关）；高程以色可读，非真 mesh 置换；无 Photorealistic |

### 架构（方案 1 · 呈现器旁路加深）

```
mgis ViewportSurface
  → SceneImpl → MapLibrePresenter (mvm_*) / FlyCubePresenter (mvf_*|GL)
  → 子 HWND WM/触摸 → InputEvent → GisContents::HandlePointer
      → SceneImpl 相机 + ToolSession::dispatch_input → GestureApplied
```

`content/public` 与壳仍不包含 MapLibre / cesium-flycube 头。引擎只经 C ABI DLL。

### 2D

- 默认 style：**OpenFreeMap Bright**（`https://tiles.openfreemap.org/styles/bright`）；首次联网后 MapLibre cache 优先。
- `MVIEW_MAP_STYLE`：空=OpenFreeMap；`offline`=示意 GeoJSON；`demotiles`；`http(s)://…`；本地 style 路径。
- 加载失败 → 回退示意 GeoJSON；mercator 软回退仍用同一示意轮廓。
- 相机概览：约 **108°E / 34°N · z≈3.8**。

### 手势

- 产品默认激活：SCENE2D → `view.pan`；切 SCENE3D → `view3d.trackball`。
- 子窗（MapSurface / FlyCubeHost / mvf child）转发指针；绝对坐标在 forwarder 内化为 **delta** 再进 `OnPointer`。
- 2D 下任意导航工具激活时滚轮仍走 zoom；右键拖映射为 pitch/bearing（仍算 pan 工具消费）。
- 触摸捏合映射为 `kInputEventWheel`（不新增 Interaction id）。

### 3D

- `WorldPass` / FlyCube：`CaptureDrapedEllipsoid` + `ProductMeshCache`；失败回退中国形假 DEM 球。
- 默认原点 **105°E / 35°N**。覆盖：`MVIEW_FLYCUBE_LON` / `MVIEW_FLYCUBE_LAT`。
- **默认**叠加：Terrarium DEM 色 TMS + Esri World Imagery（可 `MVIEW_FLYCUBE_DEM_URL` / `MVIEW_FLYCUBE_IMAGERY_URL` 覆盖）。
- `MVIEW_FLYCUBE_OFFLINE=1`：关网 overlay，回退 debug colorize；`MVIEW_FLYCUBE_DEM=0` / `MVIEW_FLYCUBE_IMAGERY=0` 可单独关掉。
- 验收：DEM **色阶**可读；**不做**真 mesh 高程置换 / Photorealistic。

### 非目标

壳 GN 直链 MapLibre / cesium-native 引擎源；天地图/高德/GCJ；扩 Interaction 表；UE 断管热切；真 GPU 纹理绑定 / mesh 置换；ion Photorealistic；把中国 planet mbtiles 提交进 git。

## gis / render 产品 ABI 门面

**Status:** landed · **Updated:** 2026-10-24  
**Diagram:** [`../diagrams/gis-render-product-abi.html`](../diagrams/gis-render-product-abi.html)

目录命名对齐 smartgis：`gis/map`、`render/rhi/flycube`（本仓顶层 peer，无 `src/` 前缀，也无独立 `native/` 树）。

### 验收（已锁）

| 项 | 决定 |
| --- | --- |
| 落点 | 顶层 `vista/`（`pass/map` + `pass/world`），与 `content/` peer；**不**塞进 `content/browser` |
| 本刀范围 | C++ `MapPass` / `WorldPass`；GN 产出 `vista_map.dll`；CMake 编 `vista_world.dll` 拷到 `out/` |
| 引擎 | MapLibre / FlyCube / cesium-flycube 仍在 `third_party`；产品不链引擎源 |
| content | `product_presenters` `deps` → `//vista/pass/map` + `//vista/pass/world`（C++ 导入库） |

细节与图：[`2026-10-24-vista-render-engine-design.md`](2026-10-24-vista-render-engine-design.md) · [`../diagrams/vista-render-engine.html`](../diagrams/vista-render-engine.html)。

### 非目标

把引擎源编进 `content`；Qt/CEF/WebView2；在 `content/public` 暴露引擎头；对外 `mvm_*`/`mvf_*` C ABI；为呈现再开 `mview::shell` 命名空间。
