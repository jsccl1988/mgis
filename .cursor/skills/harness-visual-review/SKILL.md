---
name: harness-visual-review
description: >-
  Closed-loop visual review for screenshots. Use when the user asks to 识 bug,
  视觉 review, review-prep, 打开截图修 bug, or closed-loop visual fix, or when
  a BMP/PNG needs inspection before code changes. This repo has no SmartGIS
  showcase runner; inspect the image the user names, then wait for confirm.
---

<!--
Copyright (c) 2026 The Mogu Authors.
All rights reserved.
-->

# Visual review (closed-loop)

## Hard rules

1. **No fix before human confirm** — unless the current user message already says to fix a listed set of bugs.
2. Prefer PNG for `Read`. If the user only has BMP and vision fails, say so.
3. Split crash / AV from visual fixes.
4. Stay on **`master`**.
5. Do **not** call smartgis `testing/tools/loop_runner.py` or `SmartGIS.exe --*-showcase`. Those are not in this repo.

## Steps

1. `Read` the PNG (or the path the user gave).
2. Emit a **numbered bug table** (severity; product bug vs missing capture).
3. **Stop and wait** for human confirm.
4. After confirm: fix → rebuild via `auto-build-fix` → ask for a new capture → `Read` again.

There is no `out/*/captures/` convention here until a spec adds one.
