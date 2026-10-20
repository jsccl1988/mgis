<!--
Copyright (c) 2026 The Mogu Authors.
All rights reserved.
-->

# 桌面 GIS：UE 双视口 + mview 界面 + MapLibre Native

**Status:** draft
**Date:** 2026-10-19
**Updated:** 2026-10-21 — Content 接线：手势事实走 `EventBus`，三维子 HWND 先接 FlyCube 地球，trace 进底栏，glang 与 DebugAgent 共用套件动词。另增 `third_party/cesium-flycube`（对照 Cesium for Unreal，不进壳 GN）。
**Diagram:** [`../diagrams/third-party-pins.html`](../diagrams/third-party-pins.html) · [`../diagrams/mgis-workbench-chrome.html`](../diagrams/mgis-workbench-chrome.html) · [`../diagrams/content-wiring.html`](../diagrams/content-wiring.html)
**Scope:** 独立桌面 GIS 的进程、窗口、地理底座、体渲染与界面边界。本文是方案，不包含实现。

## Goal

做出一套可安装的 Windows 桌面 GIS：

1. 用户能看到的界面全部由本仓库的 aura/views 创建：一个二维窗口、两个三维窗口、图层树、实体树，以及时间轴和菜单。
2. 两个三维窗口显示 DEM、3D Tiles、风场，以及与场景深度正确遮挡的标量体。二维窗口用 MapLibre Native 画底图和业务矢量。
3. 地理底座使用 Cesium Native，通过 Cesium for Unreal 进入工程，不从零写 Cesium Native 胶水。
4. 窗口框、停靠、树和菜单都在 `mview_shell.exe` 里。UE 进程不出现自己的标题栏。
5. 界面进程崩溃或二维地图崩溃时，三维渲染进程继续出帧。
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
- 不把 smartgis 的 vista / scenic / gpu 呈现栈搬进本仓库。三维呈现仍是 UE 进程。
- 不让 `app/shell/` 或 `plugin/` 包含管道帧、UE 头文件或 MapLibre 头文件。
- 不为套件再做一套 opcode 中间语言。glang 只有一个进程内 CPython，套件执行 `main()`。
- 不把代理的迭代写成壳里的私有测试钩子。缺能力时只增加一个具名 `eval_*`。
- 不把 MapLibre 的手势回调或 UE 的 Enhanced Input / Pawn 操控当成产品工具。它们只提供命中和相机执行。

## Locked decisions

| # | 决定 |
| --- | --- |
| 1 | 产品进程两个：UE 打包进程、`mview_shell.exe`。另有一个无界面的 Win32 启动器。 |
| 2 | 产品窗口全部是 shell 进程里的 `views::Widget`（`aura::RootWindow` + `RootWindowHostWin`）：主窗口、二维窗口、`view0`、`view1`。UE 不创建任务栏上的顶层窗口。 |
| 3 | 引擎钉扎 Unreal Engine 5.6，插件钉扎 Cesium for Unreal 2.30 的 `CesiumForUnreal-56` 包。 |
| 4 | 一个 `UWorld`、一个 `ACesiumGeoreference`。两台相机每帧写入 `ACesiumCameraManager::AdditionalCameras`。不用已废弃的 `AddCamera`。 |
| 5 | VTK 9.4（或 9.5）编成 `mview_volume.dll`，关闭 Qt、GUI 和 VTK Rendering。只输出规则网格。画面由 UE 后处理光线步进和 Niagara 绘制。 |
| 6 | MapLibre Native 编成 `mview_map.dll`，C ABI。渲染后端为 Windows 上的 OpenGL ES 3。不编进本仓库 GN。 |
| 7 | 二维窗口和两个三维窗口的内容区都是 `views::NativeViewHost`。二维挂 MapLibre 子 HWND。三维挂 UE 在该父 HWND 下创建的子 HWND。图层树和实体树是主窗口里的 `views::TreeView`，不压在这些子 HWND 上。 |
| 8 | mview UI 线程跑 `MessageLoopForUI`。MapLibre 的 `mbgl::util::RunLoop` 在 DLL 内部线程。 |
| 9 | 命名管道 `\\.\pipe\mview`，服务端是 UE。帧为小端 `uint32` 长度 + UTF-8 JSON，单帧不超过 65536 字节。 |
| 10 | 时间轴的权威在 mview。地理坐标与三维渲染的权威在 UE。出进程的坐标只用 WGS84 经纬高（EPSG:4979）。 |
| 11 | 目录数据库只由 UE 写入。界面「打开文件」只提交路径。 |
| 12 | `content/public` 是壳的唯一嵌入面。`content` 不包含 `ui/views`，也不包含 UE 头。壳实现 `GisContentsClient`，只在浏览器进程钩子里建窗口。 |
| 13 | glang 放在 `content/browser/glang.runtime/`，顶层只有 `language/`、`exec/`、`host/`。公开入口只有 `content::run_suite_script`。 |
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
MView.exe                     Win32 启动器
├─ mview_shell.exe            唯一的产品界面进程
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

壳的入口与 `examples/base_widget` 相同：创建 `MessageLoopForUI`。第一版用户能看到的窗口都是这个进程里的 `views::Widget`。

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
| `third_party/maplibre/mview_map` | 二维绘制与 `queryRenderedFeatures`。 |

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

`DebugAgent::ExecLine` 是进程内核心，不做 smartgis 那套回环套接字、Python 工人或界面点击。内置行：`help`、`trace`、`suite <path>`、`attach <view_id> <flycube|maplibre|ue>`。其余一行交给 glang 替身。替身动词：`mark`、`command`、`activate_tool`、`pointer`。`expect_camera`、`expect_layer`、`expect_entity`、`capture` 这一刀返回明确错误，不假装通过。`command` / `activate_tool` / `pointer` 的槽由 agent 接到同一个 `ToolSession`。未知动词失败时，该文件的 mark 一个都不写。

产品窗由 `mgis::ShowWorkbench` → `StartSession` 自己登记 FlyCube 钩子，再把 `view0` 子 HWND 交给 `NativeViewHost`。二维页保持占位。`app/shell` 的卫星视口仍可直接使用 `GlobeView` 做渲染检查，那条路径不负责产品接线。

验收：`build.bat debug session_wire`。

## glang

glang 在 `content/browser/glang.runtime/`，顶层三块：

| 目录 | 职责 |
| --- | --- |
| `language/` | 套件加载与诊断 |
| `exec/` | 进程内唯一的 CPython；执行套件 `main()` |
| `host/` | 把文档、视图、工具、插件填进 `CapabilityHost` |

公开入口只有：

```cpp
namespace content {
bool run_suite_script(CapabilityHost& host,
                      std::string_view path_utf8,
                      std::string* error);
}
```

壳在 `shell_runtime` 里用 `ShellBridge` 填槽，然后调用这个函数。content 不包含 `ui/views`。

具名 `eval_*` 是能力表，不按场景增加 opcode。第一版就登记这些名字：`mark`、`expect_camera`、`expect_layer`、`expect_entity`、`activate_tool`、`pointer`、`command`、`capture`。地图和三维的差异在 `view_id`（`map` / `view0` / `view1`），不在第二套脚本语言。

管道 JSON 是 `host/` 下面的传输。代理和人写的是套件，不是管道帧。

## 插件

`content::PluginHost` 在 public 头上。注册表在 `plugin/`，由壳持有。包是进程内 DLL 或 glang 登记的命令，清单是 `plugin.json`。

插件可以贡献命令 id、交互工厂、菜单项和停靠面板。交互工厂注册进目标视图的 `Workspace`。面板控件由壳安装的回调创建，content 不包含 Views。

`run_processing` 是算法入口。第一版在壳进程内执行。插件不拿到管道，也不拿到 `mbgl::Map`。

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
- `content/browser/pipe` 直接用 RapidJSON（`Document` / `Value`），助手 API 对齐 smartgis `sdbd_json` / `args_json`（`pipe_json_*`），无自制值模型。

### 引擎类 pin（不进 `ui/` 默认图）

| 清单名 | 钉扎 | 落点 | 示例 |
| --- | --- | --- | --- |
| `maplibre-native` | `ios-v6.30.0`（与 smartgis 相同） | `.src/maplibre-native`，可联接已有检出 | `third_party/maplibre/example` |
| `unreal` | UE 5.6，`UE_5_6_ROOT` | 不进仓库 | `third_party/unreal/example` |
| `cesium-unreal` | Cesium for Unreal 2.30，`CesiumForUnreal-56` 源码包 | `.src/cesium-unreal` | `third_party/cesium-unreal/example` |
| `cesium-flycube` | 本仓库编写，不进清单 fetch | `third_party/cesium-flycube` | `example/dual_view`、`example/ellipsoid_city` |
| `cesium-native` | `v0.64.0` | `.src/cesium-native`，仅示例 | `third_party/cesium-native/example` |
| `vtk` | `v9.5.2`，Qt / Rendering / Views 关闭 | `.src/vtk` | `third_party/vtk/example` |

产品进程里的 cesium-native 只来自 Cesium for Unreal 插件。清单里的 `cesium-native` 不链进壳，也不链进 UE 模块。VTK 示例只写规则网格，不创建 OpenGL 窗口。MapLibre 示例跑上游 `mbgl-render`，像素不进 `ui::Layer`。

### Cesium for FlyCube

`third_party/cesium-flycube` 是对照 Cesium for Unreal 运行时写的 FlyCube 绑定，不进 `manifest.json`，也不进壳的 GN 图。

它保留 Unreal 插件里产品实际用到的几件事：一个 WGS84 地理参考、`AdditionalCameras`（不调用已废弃的 `AddCamera`）、椭球 / URL / ion 三种瓦片源，以及 `IPrepareRendererResources`。FlyCube 没有 Actor，相机参数直接写在 `Camera` 上。世界坐标是右手系、米、X 东 / Y 北 / Z 上。两台相机 ECEF 相距超过 100 km 时返回 `origin_conflict`，原点不动。

网格先落在 CPU，再上传到 FlyCube DX12 的 vertex / index upload buffer。不做光栅化窗口。`dual_view` 只验证锚点和双相机；`ellipsoid_city` 跑椭球瓦片并上传。壳上现有的三维子窗口仍是 `GlobeView` 钩子；本库不替换那条路径，也不替代 UE 进程里的 Cesium for Unreal。

## Mgis 工作台壳（`app/mgis`）

**Status:** draft · **Updated:** 2026-10-21  
**Diagram:** [`../diagrams/mgis-workbench-chrome.html`](../diagrams/mgis-workbench-chrome.html)

对照 smartgis（`src/ui/gis` + `src/app/views`）。本仓产品 chrome 全部在 `app/mgis`（`mgis` 命名空间），不再单独保留 `ui/gis`。产品主路径是**单主窗 IDE 布局**，不再以多个独立顶层视口 Widget 作为日常 UI。

### 布局

| 区 | 内容 |
| --- | --- |
| 顶 | 菜单占位 + 时间轴 `Slider` |
| 左 | `TabbedPane`：**图层** \| **实体**（各一棵 `TreeView`） |
| 中 | `TabbedPane`：**2D** \| **3D**；页内 `ViewportPlaceholder`（第一刀无真 HWND） |
| 右 | 工具 / 插件空壳 |
| 底 | **console** \| **trace** \| **agent** 空 Tab |

### 语义与联动

- **图层** = 2D 地图侧 catalog（Workspace `map`）。
- **实体** = 3D GIS 侧 catalog（Workspace `view0`）。
- 第一刀中心仅两页：`2D` / `3D`（`view1` 后置）。
- **双向联动**：切中心 2D|3D ⇔ 切左栏 图层|实体；同一 `SelectViewTab`，防重入。
- 右栏、底栏全局，不随视口 Tab 换内容。
- 各 catalog 的 model / 选中 / 展开按视口保留。

### 目录

| 路径 | 职责 |
| --- | --- |
| `app/mgis/` | 工作台壳：`WorkbenchContents`、四区面板、`SelectViewTab`（无管道、无 MapLibre/UE） |
| `app/shell/` | 薄入口；`ShowMainWindow` → `mgis::ShowWorkbench`；harness 仍可创建卫星 ViewportWindow 写 mark |

当前文件仍**扁平**在 `app/mgis/`（`workbench` + 四面板 + `view_id`）。下一刀按职责拆子目录（仍单一 GN 目标 `//app/mgis:mgis`，命名空间 `mgis`）：

```
app/mgis/
  BUILD.gn
  view_id.h
  workbench/                 # 主窗组合与联动
    workbench.h
    workbench.cc
  chrome/                    # 四区壳面板（无管道 / 无 MapLibre / 无 UE）
    catalog_tabs.{h,cc}
    viewport_placeholder.{h,cc}
    tool_plugin_panel.{h,cc}
    bottom_panel.{h,cc}
  viewport/                  # 预留：NativeViewHost 真 HWND（map / view0）
  session/                   # 预留：文档 / EventBus / 工具会话接线
```

原则：`chrome/` 只放 View 壳；`workbench/` 只做布局与 `SelectViewTab`；视口挂载与会话进 `viewport/`、`session/`，不回流 `ui/`。

### 第一刀非目标

不挂 MapLibre/UE 子 HWND；不接文档/`EventBus`；右栏不扫插件；底栏无 IO。下一刀再把占位换成 `NativeViewHost`。

### 与多 Widget 表述的关系

上文「二维 Widget / view0 / view1 独立 Widget」仍描述 **P2 挂接与崩溃域** 的 HWND 父级策略；**产品可见 chrome** 以本节单主窗为准。P1 mark 名可暂时仍由 harness 侧卫星窗满足，直至套件改写。

## 目录与安装布局

```
launcher/
app/shell/                     薄入口（原顶层 shell/），组合 content/public + app/mgis
app/mgis/                      产品工作台（四区壳 + chrome 面板）
content/public/
content/browser/glang.runtime/ language、exec、host
content/browser/               会话、目录、管道、ToolSession 组装
tool/                          Workspace、命令、交互
plugin/                        注册表与产品包
third_party/vtk/mview_volume/
third_party/maplibre/mview_map/
ue/
ipc/schema/
testing/tools/harness/         阶段套件
```

`app/shell/` 的 GN 目标为 `//app/shell:mview_shell`，依赖与根组里的 examples 同类，并只依赖 `content/public`（及 `app/mgis`）。不修改 `ui/aura` 合成器。`tool/` 不依赖 `ui/views`。

安装布局不变：`MView.exe`、`Engine/`、`UI/mview_shell.exe`、`UI/mview_map.dll`、`Bin/mview_volume.dll`。插件放在 `UI/plugins/<id>/`。日志在 `%LOCALAPPDATA%\MView\logs\`。

## 阶段与门控

campaign 顺序就是阶段顺序。进入下一阶段之前，前面每个套件的 `required_marks` 必须在同一次 campaign 里全部落地。某一套件失败时，不写它后面的 mark，也不启动下一套件。

每一阶段另外有一条永久门禁：`app/shell/` 与 `plugin/` 的包含检查失败则该阶段失败。`app/shell` 不得包含 `content/browser`、`tool/` 的内部头、UE 头、MapLibre 头。`content` 不得包含 `ui/views` 或 UE 头。`tool/` 不得包含 HWND、UE、MapLibre。

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
