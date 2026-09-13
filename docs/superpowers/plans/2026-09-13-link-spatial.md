# 支持「链接 mapd / 链接 sdbd」实现技术方案

> **For agentic workers:** 本文是方案，不是开工令。未获用户批准前不要改产品代码。REQUIRED 若执行：superpowers:subagent-driven-development 或 executing-plans。  
> **本文件不提交。** 不改 `content/control/window/map_window.cpp` 等其它代理正在修的崩溃 / 出图路径。  
> 合并自 `2026-09-13-link-mapd.md` + `2026-09-13-link-sdbd.md`（那两份只留指针）。键名不另造：sdbd JSON 权威在 [`smartgis/docs/superpowers/04-data-sdbd.md`](../../../../../smartgis/docs/superpowers/04-data-sdbd.md) §3；双后端对照在 [`08-sdbd-mapd-transports.md`](../../../../../smartgis/docs/superpowers/08-sdbd-mapd-transports.md)；FnRPC 帧权威在 [`07-fnrpc-asio.md`](../../../../../smartgis/docs/superpowers/07-fnrpc-asio.md)。本文不重写那三章。

**Goal:** 让 mgis（及对齐的 SG20 壳）用两条明确的「链接」分别打开 mogu **mapd**（style / 瓦片 / Path Q）和 **sdbd**（catalog / recordset）。两条链接、两份工作；空间对象共用一套 OGC 对齐的 Feature 模型。一次只激活一条 Connection。

**Architecture:** 空间要素是标准对象（OGC Feature + 声明 CRS），线上编码按守护进程选：mapd Path Q = GeoJSON（RFC 7946），sdbd `FeatureSet` = WKT（OGC SF）。渲染源（style / Path R 瓦片）不是要素库。传输：两条链接都是 **HTTP 原生客户端**；sdbd 日后可把同一 JSON 换到 FnRPC/asio；**两条都不要 GDAL 当驱动**，也不要任意 SQL。

**Tech Stack:** mogu `infra/mapd` HTTP（已落地 `:8020`）；规划中的 mogu `infra/sdbd` HTTP（`:8021`，未建）+ 进程内 `MemoryTable`/`RTreeIndex`；SG20 `sg::MapdClient`（已有）+ 目标 `sg::SdbdClient` / `sg::feature_set`；mgis `content` 会话 + 现有 `gfx2d::Bind(OGRLayer*)`。WinHTTP `NO_PROXY`。

**推荐默认（两条链接同一句）：**

```text
mapd   HTTP 原生客户端（对齐 Path R/Q）。A（sdb+asio）不对 :8020 开口。
sdbd   HTTP first，asio later（同一 catalog/recordset 合同换传输）。
两边   适配器都不要 GDAL。GDAL 只作本地文件 / 标准 WMS / COG（用户显式打开）。
```

## Global Constraints

- mapd ≠ MapD/HeavyDB。本组织 mapd = mogu 地图 HTTP 进程，口 **8020**。sdbd = 空间库 HTTP 守护，规划口 **8021**。tabled FnRPC **9030**。禁止混口。
- 不把 `mapd :8020` 的 `features/query` 当 sdbd recordset。不把 Path Q GeoJSON 当 `FeatureSet` 真源。不把 `SdbdClient` 塞进 `MapdClient::Query`。
- 不 vendor mogu、不 submodule mogu / smartgis。Windows `c:\Dev\src\mogu` **不存在**；真源在 WSL `/home/ccl/dev/src/mogu`。
- 客户端禁 `sql` / `raw_sql` / `raw_query`。body 出现这些键 → **400**（sdbd 码名 `forbidden_sql`）。
- 本阶段只读。无事务、Append / Update / Delete、无 live subscribe。编辑另开里程碑，不塞进 mapd Path Q，也不塞进 sdbd v1。
- mgis `core/sdb` 今日是空壳；不得假装已有空间库 RPC。
- health 失败 = `not_ready`，**不**回落另一条链接，**不**回落本地 shapefile。
- 文档只加本 `docs/` 路径；不碰崩溃修复代理的产品文件。
- **空间数据按标准执行：** 不另造私有 GIS 方言。Feature / CRS / 图层名复用 04 / 08 / `CONSUME_API` 已锁用词（下文 §2）。

---

## 0. 一句话认定

| 用户说法 | 认定 | 证据 |
|----------|------|------|
| 「链接 mapd」 | 连 mogu **mapd HTTP** `http://<host>:8020`，拉 style / capabilities / 瓦片 / 白名单 GeoJSON | WSL `mogu/infra/mapd/README.md`；`native/CONSUME_API.md`；`smartgis/infra/mapd_http` |
| 「链接 sdbd」 | 连 mogu **sdbd HTTP** `http://<host>:8021`，走 health / catalog / `layer.info` / `recordset.open\|fetch\|close` | `04-data-sdbd.md` §3；`08` §1.5 |
| 「GIS 数据协议」 | **两套口、一套空间对象。** mapd = 渲染 + 点选；sdbd = 目录 + 游标。Feature 都是 OGC 要素，编码不同 | `CONSUME_API.md`；`04` §3；本文 §2 |
| 「sdb+rpc（asio）」 | **只指向 sdbd/tabled 后置传输**（`:9030` + 规划 `fnw::Client`）。不是 mapd 口，不是本阶段产品真源 | `07-fnrpc-asio.md`；WSL `infra/sdbd` 与 `net/rpc/fnW/` 均不存在 |
| 「http+gdal」 | HTTP **对**（两条链接的第一根线）。GDAL 当 mapd/sdbd 驱动 **错**。GDAL 只作本地文件 / 标准 WMS / COG | `04` §0.4；`map_window.cpp` `InitMap` |

两条链接写死差别（避免执行者抄错口）：

| | 链接 mapd | 链接 sdbd |
|--|-----------|-----------|
| 口 | 已落地 `:8020` | 规划 `:8021`（进程 **未建**；`ports.md` 今日无 8021 行） |
| 发现 | `capabilities` + `style.json` | `catalog` + `layer.info` |
| 主载荷 | Path R 瓦片 + Path Q **GeoJSON** FeatureCollection | 服务端 recordset handle + **WKT** `FeatureSet` |
| 游标 | 无（一次 `Query`） | 有：`open` / `fetch` / `close` |
| 编辑 | 不做 | **本阶段仍不做**（04 只读锁） |
| 上屏最短 | Path Q → OGR MEM → `Bind` | recordset → WKT → OGR MEM → `Bind`（和/或属性表） |

---

## 1. 目标：两条标准空间链接

```text
用户
  ├─ 「链接 mapd…」  →  MapdLink   →  :8020  style / tiles / Path Q
  └─ 「链接 sdbd…」  →  SdbdLink   →  :8021  catalog / recordset
         │
         │  一次一条 Connection
         ▼
   同一套空间要素（OGC Feature + 声明 CRS）
         │
         ▼
   gfx2d::Bind(OGRLayer*)   ← 显示适配，不是查询 SoT
```

**mapd** 把「一张可漫游的地图 + 可点选的要素」搬上屏。解决：发现、渲染源、交互查询、版本/能力。不解决事务编辑、任意 SQL、跨仓 ABI。

**sdbd** 把「一个可浏览的空间库目录 + 可分页的图层游标」搬上屏和/或属性表。解决：health、编目、游标、版本/能力。不解决瓦片渲染、任意 SQL、本阶段事务编辑。

SG20 [`08`](../../../../../smartgis/docs/superpowers/08-sdbd-mapd-transports.md) 的 `sgdata://` + `DataClient` 是日后统一门面（物化汇合点 `sg::feature_set::FeatureSet`）。**本方案 M1–M3 不把门面当门闩**：mgis 先做两条独立链接 / 两个 Client。禁止一份对话框混 8020/8021。

部署（sdbd 形态已锁 04 §2.1 C，本文不重开）：

```text
WSL mogu
  out/mapd                         HTTP :8020 /mapd/…     已落地
  flow_host --plugin=libtabled_*.so  （或独立 out/sdbd）
       ├─ MemoryTable + RTreeIndex  引擎 SoT（只读）
       ├─ FnRPC :9030               已有 seek/stats；catalog/recordset 待补；本方案不连
       ├─ HTTP  :8005 /tabled/      WebUI / sql 调试；不是产品查询口
       └─ sdbd  :8021 /sdbd/api/v1/*  产品查询口（未建）
                    │
         WSL2 localhost 转发
                    │
Windows
  MapdClient ──HTTP──► 127.0.0.1:8020
  SdbdClient ──HTTP──► 127.0.0.1:8021
```

tabled README 仍写 WebUI **8009**、并称 8005 退役；`ports.md` 仍锁 tabled **8005**、flowd **8009**。产品查询**一律走 8020 / 8021**，不要站队这场文档打架。

---

## 2. 共享空间数据模型（按标准）

不发明私有「mgis 几何方言」。对象与编码跟仓内已锁名称走，并钉在公开标准上。

### 2.1 一个 Feature，两种线上编码

| 概念 | 标准 | 本栈用名 | mapd | sdbd |
|------|------|----------|------|------|
| 要素 | OGC Simple Features / GeoJSON Feature | `Feature`（08 / 04）；遗产语义 `GetFeature`，不链遗产头 | Path Q 一项 | `features[]` 一项 |
| 要素集 | GeoJSON FeatureCollection **或** 本栈 `FeatureSet` | C++ 主类型名 **`FeatureSet`**（04 §3.2） | 线上 = FeatureCollection（`application/geo+json`） | 线上 = `FeatureSet` JSON（可加 `"type":"FeatureCollection"` 外壳灌显示缓存，**主类型名仍是 `FeatureSet`**） |
| 几何 | OGC | mapd：GeoJSON `geometry`；sdbd：`geom_wkt` | RFC 7946 | WKT（OGC SF） |
| 属性 | 键值 | mapd：`properties`；sdbd：`attrs` | Path Q | `FeatureSet` |
| 图层 | 具名要素集合 + 声明 CRS | mapd：`query_layers` / style `layers[]`；sdbd：`LayerInfo` | 可查 ≠ 可画 | 可列 ≠ 已打开游标 |
| 目录 | — | sdbd：`catalog` → `LayerInfo[]`（成功体是 **JSON 数组**，不要包 `{layers:[…]}`） | 无 catalog 路由；对齐 `capabilities.query_layers` | **管** |
| 游标 | 经典 Recordset | `recordset.open\|fetch\|close`；本地 `MoveFirst` / `MoveNext` / `IsEnd` / `GetFeature` | **无** handle；一次返回整包 | **管** |
| 渲染源 | MapLibre Style / XYZ / MVT | `style.json`、`Tile`、Path R | **管** | **不管**（MVT / PMTiles **不是** `FeatureSet` SoT） |

08 若落地 `gis_data`：无论线上是 GeoJSON 还是 WKT，进程内查询 SoT 只有 `sg::feature_set::FeatureSet`（`FromGeoJson` / `FromSdbdJson`）。`FeatureCache` 继续只存 GeoJSON，注释锁死：**display adapter, not SoT**。本方案 mgis 最短上屏可以先 OGR MEM，不必等 `feature_set` 目录存在。

### 2.2 CRS：EPSG，禁止静默混用

| 面 | CRS | 规则 |
|----|-----|------|
| mapd 瓦片 / view | **EPSG:3857**（XYZ） | Path R 网格。瓦片不是 Feature |
| mapd Path Q | **EPSG:4326**（GeoJSON 默认） | `bbox` = `[min_lon, min_lat, max_lon, max_lat]` |
| sdbd 层 | `LayerInfo.crs`（演示锁 **`EPSG:4326`**） | `OpenRequest.crs` 若出现且与层不同 → `bad_request`（**不重投影**）。`FeatureSet.crs` 从图层拷贝 |
| 显示 | 进 GDI LP 前必须变换或声明 | 建议：该 Connection 的内容层按声明 CRS 写死一种（演示 4326，LP=经纬度）。**不得**把 shp 投影坐标与 4326 叠在同一 Bind |

权威写法：EPSG 码字符串（`EPSG:4326`），不要「WGS84 / WebMercator」口头别名当 JSON 键。

### 2.3 bbox / limit / 谓词（两套黄金不得混）

| | mapd Path Q（`sg::QueryRequest`） | sdbd（`OpenRequest`，04 §3.2） |
|--|----------------------------------|--------------------------------|
| bbox | **四元数组** `[min_lon,min_lat,max_lon,max_lat]` | **对象** `{min_x,min_y,max_x,max_y}` |
| 谓词 | 无 `predicate` 键（服务端即 intersects） | 仅 `"predicate":"intersects"`（MBR）；其它 → `unsupported_predicate` |
| limit | 超硬上限 **clamp**，不 400 | 超硬顶 **400 `limit_exceeded`**（建议硬顶 100000；单次 fetch 上限 10000，默认 1000） |
| 黄金层 | 试点 `basemap`；示例框 `[116.3,39.8,116.5,40.0]` | `t_rtree_demo`；窄框 `(100,100)-(200,200)` → `count==1` 且 `id==2` |

门面若日后统一 `OpenRequest`，必须在 `LayerInfo` / 日志暴露 `limit_policy=clamp|reject`，禁止把 mapd clamp 当成 sdbd 语义。

### 2.4 本模型明确不含

- 任意 SQL、WFS / OGC API Features（阶段 2+ 另文，01 已列）
- 私有 `sdbd:` / `mapd:` GDAL driver、把链接串当 `GDALOpenEx` 连接串
- 静默重投影、静默混 CRS
- 把 Path R MVT / `style.json` 当要素 SoT
- protobuf 作为本阶段发现面

---

## 3. mapd 协议（style / 瓦片 / Path Q）

独立 **地图 HTTP** 进程。不是 `flow_host`，不是 `tabled`，不是 per-tile Flow。

| 项 | 值 | 路径 |
|----|-----|------|
| 仓 | WSL `/home/ccl/dev/src/mogu` | Windows `c:\Dev\src\mogu` 缺失 |
| 二进制 | `/home/ccl/dev/src/mogu/out/mapd` | `infra/mapd` |
| 启动 | `bash infra/mapd/scripts/run.sh start` | 同上 |
| 端口 | **8020**（独占） | `mogu/tools/dashboard/conf/ports.md` |
| 数据根 | `MAPD_DATA_ROOT` 默认 `/tmp/mapd-data`（`manifest.json` + `style.json` + 烘焙瓦片） | `infra/mapd/README.md` |
| 消费契约 | `mogu_contracts_version` 2026-09-12.1 | `infra/mapd/native/CONSUME_API.md` |
| SG20 客户端 | 已落地，HTTP only | `smartgis/infra/mapd_http/include/sg/clients/mapd_client.h` |
| mgis | **无** HTTP 客户端、无 mapd 会话 | `content/` 无 WinHTTP 产品调用（`cpp-httplib` 已 cmake 但未用） |

双路径 SLA（服务端已写死）：

| 路径 | 作用 | 运行时 | 缓存 |
|------|------|--------|------|
| **R** | 预烘焙瓦片（webp / MVT / terrain-RGB / 可选卫星代理） | 本地 `$data_root` mmap；**零 SQL** | `public, max-age=86400, immutable` |
| **Q** | 白名单要素查询 + 轻分析 | `PostgisPool`（DSN 可选）；禁任意 SQL | `no-store` |
| **S** | 卫星代理（capabilities `satellite`） | libcurl 上游 | mem+disk LRU |

`GET /mapd/api/v1/capabilities` 无 `manifest.json` 且卫星关闭 → **503**。Path Q 无 DSN → 查询 **503**；DSN 有但 PostGIS `:5432` 挂 → **502**。Path R 与 Path Q 队列隔离：慢查询不得堵瓦片。

### 3.1 实体（链接 mapd 持有这些）

| 实体 | mapd 上的形状 | 客户端持有 |
|------|---------------|------------|
| **Workspace / 链接** | 一个 HTTP 基址（一个 mapd 进程 + 一份 `data_root`） | `MapdLink`：`base_url` + 传输 |
| **Map** | `GET /mapd/style.json`（MapLibre Style Spec **子集**：`version`/`sources`/`layers`） | style 文档；Native / GL JS 同源 |
| **Layer** | style `layers[]` + capabilities `query_layers`（试点 `basemap`） | 图层树节点；可查 ≠ 可画 |
| **Tile** | Path R `GET /mapd/tiles/{raster\|vector\|terrain\|satellite}/…` | URL 模板，不进要素库 |
| **Feature** | Path Q → GeoJSON FeatureCollection，**EPSG:4326** | 临时 overlay / 属性表 |
| **Capability** | `GET /mapd/api/v1/capabilities` | 开不开 R/Q/S；`data_version` 是否失效缓存 |

### 3.2 线格式 / 发现

| 面 | 格式 | 版本 |
|----|------|------|
| 元数据 / Q | JSON（UTF-8） | URL 前缀锁 `/mapd/api/v1/`；capabilities 带 `data_version`（golden：`contract-2026-09-12.1`） |
| Style | JSON，MapLibre `version==8` 子集 | 无 style → **404** |
| Path R 栅格 | `image/webp`（卫星常 jpeg；terrain png） | 文件名即缓存键 |
| Path R 矢量 | MVT 或整包 PMTiles | 与 style `sources` 对齐 |
| 能力发现 | `GET /mapd/api/v1/health` 再 `GET .../capabilities` | schema：`smartgis/infra/contracts/schema/capabilities.schema.json` |
| 禁止 | 请求体 `sql`/`raw_query`/`raw_sql` | **400** |

Wire **不**用 protobuf。asio 帧不是 mapd 的发现面。

`Capabilities` 客户端子集（已解析，见 `mapd_client.h`）：`crs`、`data_version`、`path_q_features`、`path_q_analysis`、`postgis_configured`、`primary_store`（`table`\|`postgis`）、`path_r_raster`、`path_r_vector`、`query_layers`。

### 3.3 链接 URL

解析后都变成 `base_url`（去尾斜杠）+ 可选显示名：

```text
http://127.0.0.1:8020
https://map.example.com:8020
mapd://127.0.0.1:8020          → 规范成 http://127.0.0.1:8020
mapd://127.0.0.1:8020/?layer=basemap
```

| 变量 | 默认 | 用途 |
|------|------|------|
| `SG_MAPD_BASE` | `http://127.0.0.1:8020` | 基址（WSL localhost 转发） |
| `SG_MAPD_REQUIRE=1` | 关 | health 失败则硬失败 |
| `SG_MAPD_SKIP=1` | 关 | 测试跳过 live |

WinHTTP **必须** `WINHTTP_ACCESS_TYPE_NO_PROXY`（`smartgis/infra/mapd_http/http_transport_win.cc`）。

失败：health 失败 = `not_ready`，Bind([])，提示「mapd 未就绪」。**不**回落 shp，**不**回落 sdbd。空图安全是另一代理的崩溃修复，本方案不改那份代码。

08 的 `sgdata://mapd/http/127.0.0.1:8020/basemap` 是 SG20 `DataUri`，不是本对话框的必填格式。允许日后 `DataUri::FromHttpBase("mapd", base_url, layer)`。`sgdata://mapd/fnrpc/…` **非法**（mapd 无 FnRPC 面）。

### 3.4 HTTP 面（服务端已有）

| 方法 | 路径 | 谁实现 |
|------|------|--------|
| GET | `/mapd/api/v1/health` | mapd 已有；客户端探活 |
| GET | `/mapd/api/v1/capabilities` | mapd 已有；客户端解析 |
| GET | `/mapd/style.json` | mapd 已有 |
| GET | `/mapd/tiles/raster/{z}/{x}/{y}.webp` | `MapdClient::TileUrl(kRaster,…)` |
| GET | `/mapd/tiles/vector/{z}/{x}/{y}.mvt` | 已有 |
| GET | `/mapd/tiles/terrain/{z}/{x}/{y}.png` | 缺文件 404 |
| GET | `/mapd/tiles/satellite/{z}/{x}/{y}` | Path S |
| GET | `/mapd/tiles/vector/basemap.pmtiles` | 可选整包 |
| POST | `/mapd/api/v1/features/query` | body = `sg::QueryRequest`，无 SQL 字段 |
| POST | `/mapd/api/v1/analysis/{op}` | `buffer`/`intersects`/`within`/`aggregate` |
| POST | `/mapd/api/v1/jobs` | 试点 **501** `not_implemented` |

```json
{
  "layer": "basemap",
  "bbox": [116.3, 39.8, 116.5, 40.0],
  "limit": 100
}
```

可选：`id`；`lon`/`lat`/`buffer_m`（点选糖，默认缓冲 40 m）；`filters: [{column, op, value}]`，列白名单 `id`|`osm_id`|`geom_type`，`op` ∈ `eq|ne|lt|le|gt|ge`。

### 3.5 职责

**mapd（已有，本方案不改服务端合同）：** 读 `data_root`；Path R 从盘出瓦片、禁止在瓦片处理里碰 libpq；Path Q 参数化查 PostGIS（或 table-primary 能力位）；拒 SQL；双队列 / 超时 / 连接池。

**mgis content：** 解析 `MapdLink`、持 `MapdSession`、后台线程 HTTP；health → capabilities → style；「要画的层」交给 gfx，「要点的层」走 Path Q。GUI 线程只收完成消息，不 `Send` 阻塞。

**mgis gfx：** 只认几何/栅格缓冲，不认 URL。最小上屏：Path Q GeoJSON → OGR MEM → 现有 `Bind`。Path R 栅格需要 **新的** 贴图/StretchBlt API（今日 GDI 是矢量 `RenderFeature`），放到 mapd-M2b，不挡 mapd-M2a。

**mgis gui：** 「链接 mapd…」；不直接 `GDALOpenEx`、不直接 curl。

**SG20：** `sg::MapdClient`：`GetStyle` / `GetCapabilities` / `Query` / `TileUrl` / `DualPathQueue`。pin 未就绪时允许 mgis 合同兼容薄 `core/mapd_client`（同一 URL/JSON），禁止第二套字段名。

---

## 4. sdbd 协议（catalog / recordset）

独立 **空间库 HTTP** 守护（产品查询面）。不是 mapd，不是 `flow_host` 对外身份，不是 tabled WebUI / `sql/query`。

| 项 | 值 | 路径 / 证据 |
|----|-----|-------------|
| 规划目录 | `mogu/infra/sdbd/` | WSL `ls infra/sdbd` → **No such file**（2026-09-13） |
| 规划二进制 | `out/sdbd` 或 tabled plugin 同进程挂载 HTTP 线程 | `04` §2.1 方案 C |
| 规划端口 | **8021**（独占） | `04` 要求先改 `ports.md` 再写默认值。**今日无 8021 行** |
| 已占用对照 | mapd **8020**；tabled host HTTP **8005**；tabled FnRPC **9030**；flowd **8009** | `ports.md` |
| 引擎 | `MemoryTable` + `RTreeIndex::query_intersects(Box2d)` | WSL `common/table/…` |
| tabled 已有 RPC | `seek` / `stats` / `size` / `sql_query` | `rpc.h`。**无** `catalog` / `recordset` 名 |
| 消费契约 | 规划 `/sdbd/api/v1/*` + JSON；`mogu_contracts` 式黄金尚未落盘 | `04` §3；`http_contract.md` **不存在** |
| SG20 客户端 | **未建** `infra/sdbd_http`、`common/feature_set` | `02` 现状树 |
| mgis | **无** sdbd 会话 | `core/sdb` 空壳 |

**一句话：** 今日「sdbd」几乎全是文档。引擎和对照守护（mapd HTTP）在；**sdbd 进程、8021 登记、HTTP 合同文件、SdbdClient、feature_set、fnW 都不在。** 端到端必须先有人对着 `:8021` 听——服务端 P0 是 **前置依赖**，不假装客户端能单独 live。

### 4.1 实体（链接 sdbd 持有这些）

| 实体 | sdbd 上的形状 | 客户端持有 |
|------|---------------|------------|
| **Workspace / 链接** | 一个 HTTP 基址 + 一份进程内表目录 | `SdbdLink`：`base_url` + 传输 |
| **Catalog** | `GET /sdbd/api/v1/catalog` → `LayerInfo[]` | 图层树 |
| **Layer** / **Schema** | `GET /sdbd/api/v1/layers/{layer}` → `LayerInfo`；`fields[]`=`{name,type}` | 当前层名 / CRS / extent / 属性表列头 |
| **Recordset** | 服务端 handle + `matched`；`fetch` 切片；`close` 丢 handle | 物化 `FeatureSet`（本地 `MoveFirst/Next/IsEnd`） |
| **Feature** | `{id, geom_wkt, attrs}` | 上屏几何 + 属性行 |
| **Capability** | health `{"ok":true,"service":"sdbd"}`；catalog 即发现面 | 无独立 capabilities schema 文件（尚未落地） |

领域三件套（04 §0，传输可换、键不换）：

| DB 概念 | 本栈 | 本阶段落地 |
|---------|------|------------|
| **Connection** | 发现 / 会话 / health | `SdbdClient` + `Health()` → `GET /sdbd/api/v1/health` |
| **Execution** | catalog / `layer.info` / MBR / 开游标 | `ListCatalog` / `LayerInfoOf` / `recordset.open`（或快捷 `query`） |
| **Recordset** | open / fetch / close → 内存 `FeatureSet` | 服务端 handle；客户端 `MoveFirst/Next/IsEnd/GetFeature` |

本阶段 **不要**把 C/E/R 拆成三个公开 C++ 类。锁在 `SdbdClient` + `FeatureSet`。

### 4.2 链接 URL

```text
http://127.0.0.1:8021
https://sdb.example.com:8021
sdbd://127.0.0.1:8021                 → 规范成 http://127.0.0.1:8021
sdbd://127.0.0.1:8021/?layer=t_rtree_demo
```

**不要**把 `sdbd://` 解成 FnRPC `:9030`。asio 后置用单独 host:port / `FNW_TABLED_*`。误把 `http://127.0.0.1:8020` 填进本对话框 → 明确「这是 mapd 口，请用链接 mapd」，**不得**当 catalog 解析成功。

| 变量 | 默认 | 用途 |
|------|------|------|
| `SG_SDBD_BASE` | `http://127.0.0.1:8021` | 基址 |
| `SG_SDBD_REQUIRE=1` | 关 | health 失败则硬失败 |
| `SG_SDBD_SKIP=1` | 关 | 测试跳过 live |

WinHTTP 同样 `NO_PROXY`。health 失败 = `not_ready`，**不**回落 shp，**不**回落 mapd Path Q。

08 的 `sgdata://sdbd/http/127.0.0.1:8021/t_rtree_demo` 是 SG20 `DataUri`。`sgdata://sdbd/http/…:8020/…` 与 `…/http/…:9030/…` **非法**（口与传输不一致）。

### 4.3 HTTP 面（客户端必须实现 / 服务端必须建）

权威字段在 `04` §3.2。此处只锁路径，禁止临场改名。

| 方法 | 路径 | 谁实现 |
|------|------|--------|
| GET | `/sdbd/api/v1/health` | **必须新建** |
| GET | `/sdbd/api/v1/catalog` | **必须新建**；`LayerInfo` 数组 |
| GET | `/sdbd/api/v1/layers/{layer}` | **必须新建** |
| POST | `/sdbd/api/v1/recordset/open` | body = `OpenRequest` |
| POST | `/sdbd/api/v1/recordset/fetch` | `handle` + `offset` + `limit` |
| POST | `/sdbd/api/v1/recordset/close` | **必须新建** |
| POST | `/sdbd/api/v1/recordset/query` | 可选快捷：open→fetch*→close；给 `QueryBBox` |

```json
{
  "layer": "t_rtree_demo",
  "columns": ["id", "name", "geom"],
  "bbox": { "min_x": 100, "min_y": 100, "max_x": 200, "max_y": 200 },
  "predicate": "intersects",
  "crs": "EPSG:4326",
  "limit": 10000
}
```

`LayerInfo`：`name` / `crs` / `geom_type` / `geom_column` / `extent{min_x,min_y,max_x,max_y}` / `fields[{name,type}]`。  
`open` 响应：`{"ok":true,"handle":"1","schema":{…LayerInfo…},"matched":3}`。  
`FeatureSet` 切片：`layer` / `crs` / `count` / `offset` / `features[{id,geom_wkt,attrs}]`。

| `error` | HTTP | 何时 |
|---------|------|------|
| `not_ready` / `not_connected` | 503 | 引擎未加载 / health 失败 |
| `bad_request` | 400 | JSON 坏、缺 layer、bbox 非法、CRS 不一致 |
| `forbidden_sql` | 400 | body 出现 sql 键 |
| `layer_not_found` | 404 | catalog 无此层 |
| `handle_not_found` | 404 | fetch/close 未知 handle |
| `unsupported_predicate` | 400 | `predicate` ≠ `intersects` |
| `limit_exceeded` | 400 | 超硬顶（不 clamp） |
| `unsupported` | 501 | 编辑 / 事务 / 未实现谓词 |

本协议不含：mapd 瓦片 / `style.json` / Path Q / `analysis/{op}` / jobs；tabled `sql/query`；本阶段属性过滤与精确几何相交。

### 4.4 职责

**sdbd（必须新建，不在 mgis 里写服务端）：** 听 `SDBD_HTTP_BIND:SDBD_HTTP_PORT`（默认 `0.0.0.0:8021`）；进程内调 table / `RTreeIndex::query_intersects`；**禁止** sdbd→tabled 再走一遍 FnRPC；持 recordset handle（建议空闲 5 min 丢）；拒 SQL；对外身份只认 URL。

**mgis content：** 持 `SdbdSession`；health → catalog →（可选 `layer.info`）→ recordset；WKT → OGR MEM 交给 gfx；`attrs` 交给属性表。销毁 Session 必须 `recordset/close`（best-effort）。

**mgis gfx：** 只认几何缓冲。最小上屏：`OGRGeometry::importFromWkt` → MEM → `Bind`。

**mgis gui：** 「链接 sdbd…」；不拼 recordset JSON。

**SG20：** 目标 `sg::SdbdClient`：`Health` / `ListCatalog` / `LayerInfoOf` / `QueryBBox`；目标 `sg::feature_set::FeatureSet`。pin 未就绪时允许薄 `core/sdb_client`。**不要**在空的 `core/sdb/sdb.h` 上编 asio 服务。

---

## 5. 传输：HTTP vs asio vs GDAL（两条链接一张表）

用户点名 A = sdb+asio、B = HTTP+GDAL。对两条链接都不完全贴：HTTP 传输对，GDAL 驱动错；asio 方法表只对 sdbd，且现在做不赢。

| | 链接 mapd | 链接 sdbd |
|--|-----------|-----------|
| **默认** | HTTP 原生（B 的传输，不要 B 的 GDAL） | HTTP 原生（**HTTP first**） |
| **A（asio / FnRPC）** | **不赢 / 不对 :8020 开口。** mapd 现网只有 httplib | 方法表对，**现在做不赢**。保留为 M3 之后换传输（**asio later**）。JSON 键与 HTTP 同构 |
| **GDAL 当驱动** | 打不开 capabilities / style / Path Q 白名单 / 双队列。最多当栅格瓦片抓取器 | 打不开 catalog handle / 游标 / `forbidden_sql` / 分页 offset。最多当 WKT/GeoJSON 抓取器 |
| **GDAL 允许做什么** | 本地文件（今日 `InitMap`）/ 标准 WMS / COG / vsicurl——**非 mapd URL**；OGR MEM 显示适配 | 同上，**非 sdbd URL**；OGR MEM / `FromOgr` 显示适配（08 §3） |
| **失败不回落** | 不回落 shp / sdbd | 不回落 shp / mapd Path Q |

```text
默认 mapd     HTTP 原生  == 已落地 Path R/Q
默认 sdbd     HTTP 原生  == 规划 catalog/recordset
后置 sdbd     tabled FnRPC :9030 + fnw::Client   == 同一合同换传输
回退（显式）  本地文件 GDAL / 标准 WMS / COG     == 用户选文件或非本守护 URL
对照          另一条链接                         == 禁止当本链接成功
```

**为什么不是纯 A、不是纯 B：**

1. 组织文档已锁：mapd HTTP only；sdbd 本阶段真源 HTTP `:8021`；FnRPC 后置；GDAL 非 SoT（`02` / `04` / `07` / `08`）。  
2. mapd 端到端最短：HTTP + `style.json` + 一块 Path Q GeoJSON；`MapdClient` 已实现。  
3. sdbd 端到端最短：HTTP catalog + 一次 `recordset/query` + WKT → OGR MEM → 现有 `Bind`。不必等 `fnW`，也不必写 GDAL driver。  
4. A 的 RPC 名已经写在 HTTP 对照表里——后置换传输时 `QueryBBox` 入口不变。全切 Windows `fn_server` **不可行**。  
5. gfx 已能画 `OGRLayer*`。属性表可绑 `attrs` / GeoJSON `properties`，不必先做瓦片。

A 对照（只用于 sdbd 后置；**不是** mapd）：

| 领域 | HTTP（本阶段真源） | 后置 FnRPC 名 |
|------|-------------------|---------------|
| health | `GET /sdbd/api/v1/health` | 复用 tabled `stats` / `size`（probe） |
| 目录 | `GET /sdbd/api/v1/catalog` | `infra.tabled.catalog.list` |
| 层信息 | `GET /sdbd/api/v1/layers/{layer}` | `infra.tabled.layer.info` |
| 开游标 | `POST /sdbd/api/v1/recordset/open` | `infra.tabled.recordset.open` |
| 取页 | `POST .../recordset/fetch` | `…fetch` |
| 关 | `POST .../recordset/close` | `…close` |
| 订阅 | **无** | 不要先发明 |

asio 会话：`fnw::Client::Call` 同步；调用线程 `io_context::run_for(timeout)`；**禁止** GUI 无线 `run()`。每 Client 自有 `io_context`。载荷锁 **单个 JSON 字符串**（跨 GCC/MSVC）。仓内 `fnA`/`fnC` 是 length-prefix，**不能**连 tabled `:9030`（生产是 `line_codec` CRLF）。

A 现在做不赢的证据：`core/sdb` 空；`infra/sdbd` 不存在；`ports.md` 无 8021；`rpc.h` 无 catalog/recordset；smartgis 无 ASIO 包、`infra/fnrpc_asio` 不存在；`mogu/net/rpc/fnW/` 不存在。

---

## 6. 客户端 / 模块边界（mgis）

```text
gui:smartgis
  ├─ 「链接 mapd」  只传 MapdLink 字符串
  └─ 「链接 sdbd」  只传 SdbdLink 字符串
        │
        ▼
content::MapWindow / Environment
  └─ 持 MapdSession 或 SdbdSession（一次一条）
        │  不在 GUI 线程做 HTTP
        ▼
core/mapd_client 或 pin sg::MapdClient     ──HTTP NO_PROXY──►  mapd :8020
core/sdb_client  或 pin sg::SdbdClient     ──HTTP NO_PROXY──►  sdbd :8021
        │
        ├─ Path R ──► （mapd-M2b）gfx 新栅格缓冲
        ├─ Path Q GeoJSON ──► OGR MEM ──► gfx2d::Bind
        └─ FeatureSet WKT ──► OGR MEM ──► Bind
                              attrs   ──► 属性表
```

| 模块 | 允许 | 禁止 |
|------|------|------|
| `gui/` | 打开对应链接 UI | `GDALOpenEx`、拼 Path Q / recordset JSON、直接 curl、一份框混两口 |
| `content/` | 会话、把 MEM 层交给 renderer、点选发 Query / `QueryBBox`、attrs → 属性表 | `OnPaint` 里同步 HTTP；health 失败改开 shp / 另一条链接 |
| `core/mapd_client` | 薄 HTTP / 日后 pin | recordset 塞进 `Query` |
| `core/sdb_client` | 薄 HTTP / 日后 pin | 把 `MemoryTable` 拷进来 |
| `core/sdb` | 保持空，或日后改名为 pin 入口 | 在空壳上编 asio 服务 |
| `gfx/2d/` | 画已绑定的 OGR / 日后瓦片位图 | 知道 `:8020`/`:8021`、解析 capabilities/catalog、持 handle |
| SG20 `infra/mapd_http` | 合同与实现真源 | 被 mgis `#include` 源码树（应 pin 制品） |
| SG20 `infra/sdbd_http` | 合同与客户端真源（待建） | 被 mgis `#include` 源码树；塞进 `MapdClient` |
| SG20 `infra/fnrpc_asio` | 后置 A | 本方案 M1–M3 门闩；塞进 `HttpTransport` |
| mogu `infra/mapd` | 服务端（已有） | 被 Windows 工程链接 |
| mogu `infra/sdbd` | 服务端（待建） | 被 Windows 工程链接；再跳一跳 `:9030` |
| mogu `infra/tabled` | 引擎 + 已有 FnRPC | 产品路径 `sql_query` |

依赖只向下：`gui` → `content` → `core` + `gfx`。`gfx` 不依赖 `core/mapd_client` / `core/sdb_client`。

一次只激活**一条** Connection：sdbd **或** mapd **或** 本地文件。切换必须显式。失败不隐式回落。取消：丢弃过期序号，禁止把旧 Bind 盖到新链接上。

### 6.1 数据流（两条最短上屏）

**mapd-M2a（默认先做）**

```text
粘贴 http://127.0.0.1:8020
  → GET /mapd/api/v1/health          非 2xx → not_ready，Bind([])，不崩
  → GET /mapd/api/v1/capabilities    crs / query_layers / path_q.features
  → GET /mapd/style.json             可选；M2a 可只用于图层名
  → 视口包络 → EPSG:4326
  → POST /mapd/api/v1/features/query {layer,bbox,limit}
  → GeoJSON → OGR MEM → Bind → ZoomToRect → GDI RenderFeature
```

**mapd-M2b（底图瓦片）** — `TileUrl(kRaster,z,x,y)` → 工作线程 GET webp → 解码 → gfx 新 `DrawRaster(LP)`（今日不存在）。不要用 GDAL WMS XML 当产品路径。

**mapd 点选** — 屏幕点 → DPToLP → 经纬度 → `QueryRequest { lon, lat, buffer_m:40, layer }` → DYNAMIC overlay。

**sdbd-M2**

```text
粘贴 http://127.0.0.1:8021  或  sdbd://…/?layer=t_rtree_demo
  → GET /sdbd/api/v1/health          非 2xx → not_ready，不改开 shp/mapd
  → GET /sdbd/api/v1/catalog         图层树；无层 → 「目录空」（health 200 = 链接成功）
  → （可选）GET /sdbd/api/v1/layers/{layer}
  → POST /sdbd/api/v1/recordset/query   {layer, bbox, predicate:"intersects"}
        或 open → fetch* → close
  → FeatureSet
        ├─ importFromWkt → OGR MEM → Bind → ZoomToRect
        └─ attrs → 属性表（gfx 不读 attrs）
```

**不要**用 mapd `features/query` 冒充 sdbd 流。  
**不要**在 GUI 线程跑 `HttpTransport::Send` 或 `fnw::Client::Call`。

---

## 7. 里程碑

两条链接并行、互不挡。A（asio）**不进入**任一门闩。sdbd live 另依赖 mogu 服务端 P0。

### mapd-M1 — 协议桩（不对屏）

- 解析 `http(s)://` / `mapd://` → `base_url`
- `Health` / `GetCapabilities` / `ParseCapabilities`（字段与 `capabilities.schema.json` 对齐）
- Fake 传输 + golden JSON（可复制 `smartgis/infra/contracts/golden/capabilities_table_primary.json`）
- live：`SG_MAPD_BASE`；health 失败 skip（除非 `SG_MAPD_REQUIRE=1`）
- **验收：** mock 绿；不改 mapd 服务端

### mapd-M2 — 第一层上屏

- **M2a（默认）：** Path Q → OGR MEM → 现有 `Bind`
- **M2b（可选）：** Path R raster blit（gfx 增量）
- GUI：「链接 mapd」；失败不回落 shp
- **验收：** mapd 已起且 Path Q 200 时看到 `basemap` 几何；宕机 → 空图 + 日志 `not_ready`

### mapd-M3 — 查询 / 轻分析（只读）

- 点选 / 拉框走 Path Q；`analysis/{op}` 可选；属性表绑 GeoJSON `properties`
- **不**做编辑；jobs 保持 501

### sdbd-M1 — 协议桩 + 发现（不对屏）

- mogu `ports.md` 登记 **8021**（本 mgis 文档只引用，不代替那一行）
- 解析 `http(s)://` / `sdbd://` → `base_url` + 可选 `layer`
- `Health` / `ListCatalog` / `ParseLayerInfo`（字段与 `04` §3.2 对齐）
- Fake + 黄金 JSON（`t_rtree_demo`；窄框 `count==1` `id==2`）
- live：`SG_SDBD_BASE`；负例：打 `:8020` Path Q **不得**当 catalog 成功；body 带 `sql` 必须失败
- **验收：** mock 绿。live 仅当 WSL `:8021` 已听。不改 mapd 合同

服务端 P0（mogu，前置，不是 mgis 文件）：`infra/sdbd` HTTP + 演示层。无此则 M1 只能 mock，不得宣称「已链接 sdbd」。

### sdbd-M2 — catalog → recordset → 上屏 / 属性表

- **M2a（默认）：** WKT → OGR MEM → `Bind` + `ZoomToRect`
- **M2b（可并行）：** `attrs` → 属性表。mgis `content/` 今日**无**属性表符号——允许先用简单 ListView
- GUI：「链接 sdbd」；失败不回落 shp / mapd
- **验收：** sdbd 已起时看到 `t_rtree_demo` 几何（和/或一行 `id==2`）；宕机 → 空图 + `not_ready`

### sdbd-M3 — 视口刷新 / 点选仍走 recordset（只读）

- 平移缩放后按视口 bbox 再 query（工作线程；取消过期请求）
- 点选 / 拉框 = 同一 `intersects` MBR，**不是** mapd Path Q
- **不**做编辑；**不**接 `fnw::Client` 到产品入口（那是 `04` §6 F2/F3 + `07`）

后置切换时：同一 `QueryBBox`，传输从 `HttpTransport` 换成 `fnw::Client::Call`；JSON 键不得分叉。

---

## 8. 已有 vs 必须建

| 已有 | 必须建 | 明确不要建 |
|------|--------|------------|
| mogu mapd 进程与 HTTP 合同 | mgis `MapdLink` + `MapdSession` | mapd 的 asio/FnRPC 口 |
| SG20 `MapdClient` / `QueryRequest` / `DualPathQueue` | mgis 侧调用（pin 或合同兼容薄客户端） | 把 recordset 塞进 `MapdClient::Query` |
| capabilities schema + golden | mapd mock/live 测试 | 第二套 JSON 字段 |
| table 引擎 `MemoryTable` / `RTreeIndex`（WSL） | mogu `infra/sdbd` HTTP + `ports.md` 8021 | 在 mgis 里再实现一套引擎 |
| tabled `:9030` seek/stats/sql_query | catalog/recordset 路由与 handle 表 | 产品路径暴露 `sql_query` |
| SG20 `HttpTransport` / WinHTTP `NO_PROXY` | `SdbdClient` + `feature_set` | 第二套字段名；塞进 `MapdClient` |
| 遗产游标语义（`gis_sde.h` `MoveFirst` / `SmtLayerInfo`） | 同名方法落在 `feature_set` | `#include` 遗产头 / COM ADO |
| mgis GDI `Bind(OGRLayer*)` | GeoJSON / WKT → OGR MEM（content 或 core） | gfx 里写 HTTP |
| mgis 本地 shp + GDAL | 仅作**显式**文件打开 | `/vsicurl/` 当 mapd/sdbd SoT；health 失败自动 shp |
| `core/sdb` 空壳 + `gdal.hpp` | 保持空，另开 `sdb_client` / `mapd_client` | 空 sdb 上编 asio 服务 |
| `07` 的 A 分层 | 后置再做；本方案只引用 | 本方案 M1 先写 `fnrpc_asio` |
| 08 `sgdata://` / `DataClient` | 不挡本方案两条链接 | 把两条链接先揉成一个 Client 当 M1 门闩 |

---

## 9. 风险

| 风险 | 缓解 |
|------|------|
| 与崩溃修复代理抢 `MapWindow` | 本方案只写 docs；落地时在 `InitMap` **旁**加链接入口，不回滚他们的空图安全 |
| Path Q 503/502（无 DSN / PG 挂）而 Path R 仍可用 | capabilities 分流：无 `path_q.features` 则 mapd-M2b 或明确「仅底图」 |
| 服务端 sdbd 不存在，客户端先宣称 live | sdbd-M1 mock / M2 live 分门闩；无 `:8021` listen 不得写「已链接」 |
| 把 8020 / tabled sql/query 当 sdbd | 合同 + live 负例；两个 Client 分家；sdbd 对话框拒 8020 |
| CRS：Q=4326 / 瓦片=3857 / 层声明 crs / 今日 shp 投影 | 一次一条 Connection；进 LP 前显式变换或声明；sdbd 不重投影 |
| `limit` 误抄 | mapd clamp；sdbd 400。写进日志 `limit_policy` |
| WinHTTP 代理黑洞 WSL | 复制 `NO_PROXY`；8021 与 8020 同一套网络假设 |
| `ports.md` 无 8021 却写死默认口 | 先改 mogu `ports.md`（04 已锁） |
| 8005 vs 8009 文档打架 | 产品查询只认 8020 / 8021 |
| 演示 SO 无空间表 | `t_rtree_demo` 必须在探活用 plugin 里 |
| 双真源（shp + mapd + sdbd） | 一次只激活一条；失败不隐式回落 |
| 合同分叉 | mapd 字段只从 `CONSUME_API` / `QueryRequest` 抄；sdbd 只从 `04` §3 抄 |
| 提前做 A 挡 HTTP | M1–M3 禁止 `fnw` 进产品入口；A 不对 mapd 开口 |
| 编辑被塞进 v1 | 501 `unsupported`；另开里程碑 |
| webp 解码（mapd-M2b） | M2a 先走矢量 |

---

## 10. 需要用户拍板的开放问题

已拍板、不要再讨论：两条链接两套口、一套空间 Feature 模型；mapd HTTP only；sdbd HTTP first / asio later；GDAL 非 SoT；禁 SQL；禁静默回落；不混 Path Q 与 recordset；不在本文件改崩溃修复代码。

1. **mapd 第一层上屏选 M2a（Path Q→OGR MEM）还是 M2b（Path R 瓦片）？** 默认 M2a。演示必须「看底图」则并行，但 gfx 要加栅格。  
2. **sdbd 是否包含在 mogu 落地 `infra/sdbd` P0？** 默认 **包含为前置**。若只做 mgis 客户端，sdbd-M1 只能 mock。  
3. **mgis 客户端形态：** pin SG20 制品，还是先在 `gis/mgis/core` 写合同兼容薄客户端？pin 更干净（04 §7），但 `infra/sdbd_http` 今日不存在；`mapd_http` 已存在。  
4. **sdbd 第一屏：M2a 地图 Bind、M2b 属性表、还是两者并行？** 默认 **M2a + 尽量并行 M2b**。  
5. **链接 UI：** 仅环境变量 / 命令行，还是要对话框？两条链接的 M1 都可以只有 env。  
6. **无 Path Q 时是否允许 mapd「仅瓦片」？** 建议允许，但不得回落 shp。  
7. **无 catalog 层时是否允许 sdbd 空图成功？** 建议：health 200 + 空 catalog = 链接成功、目录空；health 失败才是 `not_ready`。  
8. **M3 是否纳入编辑？** 建议 **否**（mapd Q 只读；sdbd 与 04 只读锁一致）。  
9. **standalone ASIO pin 的精确版本**（仅当启动 A 时）：`07` §12 仍开放；不挡 HTTP。  
10. **日后是否把两条链接收进 08 的 `sgdata://` + `DataClient`？** 不挡 M1–M3。若收，URI 校验必须沿用 08 错口表，不得新发明第四套 scheme。

---

## 11. 证据路径（检索记录）

codebase-memory-mcp / `search_graph`：**本会话未连接**。检索限制在已知子树 + 两份被合并原稿引用的 WSL / smartgis 路径。

| 路径 | 用来证明 |
|------|----------|
| 原稿 `2026-09-13-link-mapd.md` / `2026-09-13-link-sdbd.md` | 合并来源（现为指针 stub） |
| `/home/ccl/dev/src/mogu/infra/mapd/README.md` | mapd 进程、8020、Path R/Q、非 tabled |
| `/home/ccl/dev/src/mogu/infra/mapd/native/CONSUME_API.md` | HTTP 面、CRS、客户端用法 |
| `/home/ccl/dev/src/mogu/tools/dashboard/conf/ports.md` | 8020 / 8005 / 8009 / 9030；**无 8021** |
| WSL `infra/sdbd` 不存在 | sdbd 进程未建 |
| WSL `infra/tabled/serving/protocol/rpc.h` | 无 catalog/recordset；有 sql_query |
| WSL `common/table/index/spatial/rtree_index.h` | `query_intersects` |
| WSL `net/rpc/` | 有 fn/fnA/fnC，**无 fnW** |
| `c:\Dev\src\mogu` 不存在 | Windows mogu 不是真源 |
| `c:\Dev\src\smartgis\infra\mapd_http\include\sg\clients\mapd_client.h` | 已落地客户端 |
| `c:\Dev\src\smartgis\infra\mapd_http\include\sg\clients\query_request.h` | Path Q body；无 SQL 字段 |
| `c:\Dev\src\smartgis\infra\contracts\schema\capabilities.schema.json` | 发现合同 |
| `c:\Dev\src\smartgis\docs\superpowers\04-data-sdbd.md` | C/E/R、`LayerInfo` / `FeatureSet` / `geom_wkt`、HTTP `:8021`、禁 GDAL SoT |
| `c:\Dev\src\smartgis\docs\superpowers\08-sdbd-mapd-transports.md` | 一套 Feature 模型、两套口、`sgdata://`、栈 A/B |
| `c:\Dev\src\smartgis\docs\superpowers\07-fnrpc-asio.md` | asio 后置；禁 mapd 当 FnRPC |
| `c:\Dev\src\smartgis\docs\superpowers\02-architecture.md` | 双口图；目标叶未建 |
| `c:\Dev\src\smartgis\docs\superpowers\03-lineage-mgis.md` | mgis 模块；`core/sdb` 空；遗产游标语义 |
| `c:\Dev\src\gis\mgis\core\sdb\sdb.h` / `sdb.cpp` | sdb 空壳 |
| `c:\Dev\src\gis\mgis\content\control\window\map_window.cpp` | InitMap GDAL shp；InitRenderer Bind |
| `c:\Dev\src\gis\mgis\gfx\2d\renderer\render_device.h` | 仅 OGR 矢量 Bind |
| `c:\Dev\src\gis\smartgis\src\SmtGisCore\gis_sde.h` | `SmtGQueryDesc` / `SmtLayerInfo` / `MoveFirst` 语义源 |

未写产品代码。未 commit。
