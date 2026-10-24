<!--
Copyright (c) 2026 The Mogu Authors.
All rights reserved.
-->

# Views declarative markup Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Ship Chromium-adapted declarative markup (`ui/views/markup`) plus independent `UiDesigner.exe`, using pinned pugixml/yoga.

**Architecture:** Port smartgis markup responsibilities into `views` namespace with PascalCase APIs and mview XML dialect; bind tags to `ui/views/controls`; designer under `app/ui_designer/`.

**Tech Stack:** C++23/MSVC, GN/Ninja, pugixml, Yoga, gtest, Chromium views.

## Global Constraints

- Stay on `master`; no Qt/WebView2/CEF.
- Comments in English; user-facing docs in Chinese where applicable.
- Use `VIEWS_EXPORT`, `views::` namespace, `base::Lock` for new locks.
- Do not force-link markup into default product shell.

---

## Task 1: Markup runtime library

**Files:** `ui/views/markup/**`, `ui/views/BUILD.gn`

- [x] Add document/style/factory/layout/loader sources (Chromium-adapted).
- [x] Register FlexRow/FlexColumn/Label/LabelButton/Textfield/… creators.
- [x] Wire GN deps on `//third_party:pugixml` and `//third_party:yoga`.
- [x] Add `markup_unittests` and run via `build.bat debug` (7/7 PASSED).

## Task 2: UiDesigner executable

**Files:** `app/ui_designer/**`

- [x] Independent win32/exe target `UiDesigner`.
- [x] Live canvas via `LoadMarkup`; open/save/hot-reload.
- [x] Palette / hierarchy / properties (insert under selection). Drag reorder still open.

## Task 3: Sample assets

**Files:** `ui/views/markup/testdata/*`

- [x] mview-dialect `preview_sample.ui.xml` + `.ui.css`.
