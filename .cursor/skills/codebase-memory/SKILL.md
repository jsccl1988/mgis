---
name: codebase-memory
description: >-
  Graph-first code lookup for this mview repo via codebase-memory-mcp.
  Use when exploring architecture, finding functions, tracing callers,
  searching code, 查找代码, Grep/Glob, impact, or dead code. Must use CBM before
  repo-wide Grep.
---

<!--
Copyright (c) 2026 The Mogu Authors.
All rights reserved.
-->

# mview CBM

Code lookup **must** use CBM. Namespace: `user-codebase-memory-mcp`.

| | |
|---|---|
| Project | `mview` |
| Root | `C:/Dev/src/gis/mview` |
| Windows exe | `C:/Users/LENOVO/.local/bin/codebase-memory-mcp.exe` |

WSL CBM is another store. Do not query WSL `smartgis` for this tree.

## Workflow

1. `list_projects` — confirm `root_path` `C:/Dev/src/gis/mview`.
2. `search_graph(project="mview", name_pattern="...")`.
3. `get_code_snippet` / `trace_path(direction="both")`.
4. `check_index_coverage` on cited paths when a negative or exhaustive claim needs it.

If `mview` is missing from `list_projects`, say so. Index only when the user asks or `/auto-cbm-gen` runs (`index_repository` `name=mview`, `persistence=false`). Do not auto-reindex after ordinary edits.

Grep/Glob only if CBM is down, the user gave an exact path, or the search is already scoped (`path` required).

## Path filters

- `search_graph(file_pattern=…)` is a **substring** on `file_path`, not regex. Use `ui/views` or `ui/aura`. Do not pass `.*….*`.
- `search_code(path_filter=…)` **is** regex.
- Prefer `qn_pattern` (regex) after moves.

Vendor matrix: user skill `cbm-code-discovery`.
