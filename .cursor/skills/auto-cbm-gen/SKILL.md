---
name: auto-cbm-gen
description: >-
  Refresh the codebase-memory (CBM) index for this repo. Use when the user
  invokes /auto-cbm-gen, or says CBM index, 更新索引, codebase memory index
  update, or when auto-idle-pipeline reaches the CBM stage. MCP
  user-codebase-memory-mcp; project mgis; root C:/Dev/src/gis/mgis.
---

<!--
Copyright (c) 2026 The Mogu Authors.
All rights reserved.
-->

# Auto CBM index update

**更新本仓库 codebase-memory 索引**

Independently callable (`/auto-cbm-gen`) and also stage 3 of `/auto-idle-pipeline`.

## Target

| | |
|---|---|
| MCP namespace | `user-codebase-memory-mcp` |
| Project | `mgis` |
| Root | `C:/Dev/src/gis/mgis` |

Windows Cursor store only. Do **not** index the WSL store or the `smartgis` project.

## Authorization

When this skill is invoked, the agent **MUST** run the index steps below.

## Steps

1. Discover tools if needed (`GetDynamicTools` on `user-codebase-memory-mcp`), then `CallDynamicTool`.
2. `list_projects` — confirm project `mgis` with `root_path` `C:/Dev/src/gis/mgis`.
3. `index_repository` with `repo_path` `C:/Dev/src/gis/mgis`, `name` `mgis`, `mode` `full` (or `moderate` if full times out), `persistence` `false`.
4. `index_status` for `mgis` and report in **简体中文**.

## Done bar

- `index_repository` completed without a hard error.
- `index_status` reported.

## Hard stops

- MCP missing or stuck in `needsAuth` / `error` after one `mcp_auth` — report and stop.
- `index_repository` fails repeatedly — report; do not invent an index.
- Wrong root (smartgis or a WSL tree) — stop.

Do not auto-reindex outside this skill, `/auto-idle-pipeline` stage 3, or an explicit user ask. Do not change product C++ just to refresh the graph.
