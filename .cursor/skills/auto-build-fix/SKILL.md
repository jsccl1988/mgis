---
name: auto-build-fix
description: >-
  Use when the user asks to build, compile, verify compile, fix build errors
  or warnings, "make it compile", or says 构建, 编译, 修编译错误, 修警告,
  直到成功, or 自动构建并修复直至成功为止. For this repo: build.bat
  (gn gen + ninja, out/Debug and out/Release). Not MSBuild, not gen.bat.
---

<!--
Copyright (c) 2026 The Mogu Authors.
All rights reserved.
-->

# Auto build + fix until green

**自动构建并修复直至成功为止**

## Authorization

When this skill is invoked, the agent **MUST** run the build and iterate until the done bar.

Normative allow rule: **`.cursor/rules/build/agent-may-build.mdc`**.

## Hard rules

1. **Entry:** from the **mview** repo root, run `.\build.bat debug` (or `.\build.bat debug <target>`).
2. **Engineering management is GN.** `build.bat` → `gn gen out/Debug|Release` + `ninja`. Transcripts: `out/Debug/build.log`, `out/Release/build.log`.
3. **Do not** use `gen.bat`, `gen-dll.bat`, or MSBuild as the fix loop. `sln` exits 2.
4. Product sources are **`ui/`**, **`base/`**, **`skia/`**, **`app/`**. Do not invent smartgis `//src` targets.
5. There is **no** compile lock. Do not create `out/.build.lock.on`.
6. Stay on **`master`**.

```bat
.\build.bat debug
.\build.bat debug base_widget
```

## Fix loop

1. Run `.\build.bat debug` (or one target).
2. Parse `out/Debug/build.log` for `error:` / `FAILED` / `ninja: build stopped`.
3. Hypothesis, then fix the owning source or `BUILD.gn`. Edit `BUILD.gn` directly.
4. Rebuild until the done bar.

**Done bar:** `.\build.bat debug` exit **0** (or the single target that was requested).

## Evidence

Claim green only after a fresh exit 0. Show the command and the log path.

## Hard stops

- Same error unchanged **3+** times
- `gn.exe` or `ninja.exe` missing under `build\bin`
- User asked not to compile

## LNK locked output

If the linker cannot write a PE: stop the locking process (gentle, then force; non-sandbox shell per `agent-terminal-kill.mdc`) and relink. That is not a source bug.
