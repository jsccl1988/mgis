<!--
Copyright (c) 2026 The Mogu Authors.
All rights reserved.
-->

Status: Active  
Date: 2026-10-22  
Diagram: `docs/superpowers/diagrams/docs-portal.html`

# Docs portal（MD / HTML 本地预览）

## Why a separate spec

Living GIS spec (`2026-10-19-ue-mview-maplibre-design.md`) owns product UX/integration. This is **docs tooling** (local catalog + preview), not aura/views/content. A § merge would bury unrelated concerns. smartgis already ships the same portal; mview copies the pattern.

## Goal

在仓库内一键打开本地 docs 门户：侧栏自动列出 `docs/` 下的 Markdown 与 HTML，右侧预览（MD 轻量渲染，HTML iframe 整页嵌入）。Agent 与人都走同一入口。

## Decisions

| Item | Choice |
| --- | --- |
| Approach | 从 smartgis `docs/portal/` **近乎原样拷贝**，改品牌为 mview |
| Runtime | Python 3 标准库 only（`ThreadingHTTPServer`）；零 npm/CDN |
| Entry | `docs/portal/open.bat`（`py -3` 优先，再 `python`） |
| Preferred port | `8765`（占用则自动换口） |
| Bind | `127.0.0.1` only |
| Agent skill | `.cursor/skills/docs-portal/SKILL.md`（禁止裸 `http.server` / `file://`） |
| Default open dirs | `superpowers`、`superpowers/diagrams`、`specs`、`plans` |
| Out of scope | 完整 MD 引擎、共享子模块、产品 C++、改 `build.bat` |

## Files

| Path | Role |
| --- | --- |
| `docs/portal/serve.py` | 扫 `docs/`；`/`、`/api/catalog`、`/api/raw`、静态文件 |
| `docs/portal/index.html` | UI（标题 **mview Docs Portal**） |
| `docs/portal/open.bat` | Windows 启动器 |
| `docs/portal/README.md` | 人读说明 |
| `.cursor/skills/docs-portal/SKILL.md` | Agent 打开预览流程 |

## Behavior

1. `GET /api/catalog` 每次重扫 `docs/`（跳过 `.git` / `__pycache__` / `node_modules` / 点目录；门户自身 `index.html` / `serve.py` / `open.bat` 不进目录树）。
2. MD：`/api/raw?path=` → 前端轻量渲染（标题 / 列表 / 表格 / 代码块 / 链接）。
3. HTML：同源路径 iframe（设计图可交互）。
4. 过滤：全部 / 仅 MD / 仅 HTML + 路径搜索。
5. 深链：`http://127.0.0.1:<port>/?path=<rel under docs/>`。

## Agent rules（skill）

1. 入口：仓库根执行 `docs\portal\open.bat`；额外参数原样转给 `serve.py`。
2. Agent 循环：`open.bat --no-browser` 后台跑；解析 stdout 的 `url`；用 Cursor IDE browser 打开（用户说「打开 / preview / 预览」时 `position: "side"`）。
3. 禁止 `python -m http.server`；禁止 `file://` 打开 `index.html`。
4. 先探 `http://127.0.0.1:8765/api/catalog`；已有合法 JSON 则复用；若是目录列表则杀占用 PID 后重启。
5. 确认页标题为 **mview Docs Portal**。

## Verification

```bat
docs\portal\open.bat --no-browser
```

- stdout 含 `url      http://127.0.0.1:<port>/`
- `GET /api/catalog` JSON 含 `counts`（md/html > 0）
- 打开 `superpowers/diagrams/content-wiring.html` 与任一 `specs/*.md`
