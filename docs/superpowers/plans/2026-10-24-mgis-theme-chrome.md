<!--
Copyright (c) 2026 The Mogu Authors.
All rights reserved.
-->

# Mgis 主题 + chrome 视觉 Implementation Plan

> **For agentic workers:** 本计划对应 living §「主题与 chrome 视觉（1+2）」。

**Goal:** `ui/views` 引入与 SmartGIS 同形的主题服务；`app/mgis` chrome 去硬编码并支持 Light/Dark 切换与持久化。

**Architecture:** `Theme` + `ThemeService` 在 `ui/views/theme/`；Widget 经 `ShellThemeBridge` 订 `ThemeObserver`；horizon `OnPaint`/`OnThemeChanged` 读 `Theme::Current()`；markup CSS `theme(token)`。

**Tech Stack:** Chromium `ui/views`、`build.bat debug`。

**Spec:** `docs/superpowers/specs/2026-10-19-ue-mview-maplibre-design.md` § 主题与 chrome 视觉。

---

### Task 1: Theme 内核

- [x] `ui/views/theme/theme.{h,cc}` / `theme_service.{h,cc}`
- [x] 默认 light；builtin dark；persist `%LocalAppData%\MView\ui_theme_id.txt`

### Task 2: Widget + markup 双轨

- [x] Widget `ShellThemeBridge` → `ThemeChanged`
- [x] `CssParser` 解析 `theme(shell_bg)` 等
- [x] `workbench.ui.css` / embedded markup 接 token

### Task 3: mgis chrome

- [x] horizon 去硬编码 SkColor；`OnThemeChanged`
- [x] View → Theme 菜单；`LoadPersisted` 于 `ShowWorkbench`
- [x] `WorkbenchContents::ApplyChromeThemeColors`

### Task 4: 验证

- [x] `build.bat debug views` / `markup_unittests`（Theme 单测绿）
- [x] `build.bat debug mgis`
