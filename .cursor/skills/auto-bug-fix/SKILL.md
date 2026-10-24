---
name: auto-bug-fix
description: >-
  Use when the user asks to run tests, fix test failures, reproduce a runtime
  bug until tests pass, or invoke /auto-bug-fix; also when they say 跑测试,
  修测试, 冒烟, 直到测试绿, or 自动修 bug. For this repo: gtest targets
  from testing/BUILD.gn after build.bat. Not smartgis harness/te.
  Not compile-only (that is auto-build-fix).
---

<!--
Copyright (c) 2026 The Mogu Authors.
All rights reserved.
-->

# Auto bug-fix until tests are green

**自动跑测试并修根因直至绿**

`auto-build-fix` = compile loop. This skill = **run gtest binaries, reproduce failures, fix root cause, re-run**.

**REQUIRED BACKGROUND:** superpowers:systematic-debugging.
**REQUIRED SUB-SKILL:** codebase-memory (project `mgis`) before repo-wide Grep.

## Authorization

When this skill is invoked, the agent **MUST** run the tests and iterate. Do not hand the loop back to the user.

## Hard rules

1. Build the test target with `.\build.bat debug <gtest_target>` (same entry as `auto-build-fix`).
2. Tests are **gtest** (`testing/BUILD.gn`). There is **no** `SmartGIS.exe --harness` and no `build.bat debug te`.
3. Run the gtest executable the GN target produces. Pass `--gtest_filter` when isolating one case.
4. **CBM first** (project `mgis`, root `C:/Dev/src/gis/mgis`).
5. Stay on **`master`**.

## Fix loop

1. Build the failing test project.
2. Run it. Parse `FAILED` / assertion text / non-zero exit.
3. Hypothesis. Fix root cause in the product or the test. Do not delete tests or wrap them in `#if 0`.
4. Re-run the same binary until the done bar.

**Native crash / hang / `.dmp`:** follow `auto-diagnose-fix` first, then return here.

**Done bar:** the requested gtest binary exits **0**. A green compile with no test run is not enough. If the tree has no runnable test binary for the area, say so and stop — do not invent a harness.

## Evidence

Show the test command, exit code, and a short pass/fail tail before claiming green.

## Hard stops

- Same failure unchanged **3+** times
- No test target exists for the area (report; do not stub a fake harness)
