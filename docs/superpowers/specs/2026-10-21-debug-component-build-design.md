<!--
Copyright (c) 2026 The Mogu Authors.
All rights reserved.
-->

Status: Active  
Date: 2026-10-21  
Diagram: `docs/superpowers/diagrams/debug-component-build.html`

# Debug component build

## Why a separate spec

Living GIS spec (`2026-10-19-ue-mview-maplibre-design.md`) owns product UX/integration. This is **build-system** scope (GN `component()` / DLL layout). A § merge would bury unrelated concerns.

## Goal

Debug incremental link is too slow with monolithic static libs. Enable Chromium-style **component build** for Debug only.

## Decisions

| Item | Choice |
| --- | --- |
| Mode | Big bang: infra + convert export-ready product libs + Skia in one land |
| Debug | `build.bat` passes `is_component_build=true` |
| Release | `is_component_build=false` (static) |
| Skia | Component DLL (`SKIA_DLL` / `SKIA_IMPLEMENTATION`) |
| Other `third_party` | Stay static |
| No export macros yet | `content` / `plugin` / `tool` / `mgis` / gtest remain `static_library` |

## Mechanism

1. `declare_args() { is_component_build }` in `build/BUILDCONFIG.gn`
2. Global `COMPONENT_BUILD` via `//build/config:default` when enabled
3. `component()` in `build/config/component.gni` → `shared_library` or `static_library`
4. Converted targets: `base`, `base_prefs`, `base_i18n`, `ui`, `views`, `views_examples_lib`, `native_theme`, `extra_views`, `skia`

## Verification

```bat
build.bat debug
```

Expect `base.dll`, `ui.dll`, `views.dll`, `skia.dll` (and siblings) under `out/Debug/`, plus product exes that load them.
