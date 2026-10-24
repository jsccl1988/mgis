<!--
Copyright (c) 2026 The Mogu Authors.
All rights reserved.
-->

# `vista/` — render engine layer

Next-gen product present pathway (smartgis scenic → vista). Slim P0: C++ only.

| Path | Role | DLL |
| --- | --- | --- |
| `pass/map` | `vista::MapPass` → MapLibre / mercator | `vista_map.dll` |
| `pass/world` | `vista::WorldPass` → cesium_flycube DX12 | `vista_world.dll` |

Hosts include `vista/pass/...` headers and link the matching import libs. No `mvm_*` / `mvf_*` C ABI.

Design: [`docs/superpowers/specs/2026-10-24-vista-render-engine-design.md`](../docs/superpowers/specs/2026-10-24-vista-render-engine-design.md).
