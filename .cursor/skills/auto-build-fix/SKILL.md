---
name: auto-build-fix
description: >-
  Use when the user asks to build, compile, verify compile, fix build errors
  or warnings, “make it compile”, clean `-Werror`/warnings, or ship with a
  green build; also when they say 构建, 编译, 修编译错误, 修警告, 直到成功,
  or 自动构建并修复直至成功为止. For this repo: Windows GN via build.bat.
---

# Auto build + fix until green

**自动构建并修复直至成功为止**

Drive the loop through **`build.bat`** at the repo root. Canonical transcript: **`out/build.log`**.

## Authorization

When this skill is invoked, attached (`@auto-build-fix`), or followed, the agent **MUST** run builds and iterate until the done bar — do not defer compile to the user.

## Hard rules — how to build

1. **Entry:** from the **mgis** repo root, run `.\build.bat` (or `cmd /c build.bat`).
2. **Do not** invent bare `gn gen` / `ninja -C out` as the primary loop unless `build.bat` is missing/broken and you are fixing the entry itself.
3. **UNC / WSL paths:** CMD cannot use `\\wsl.localhost\...` as cwd. Prefer:
   - running `build.bat` (it `pushd "%~dp0"`), or
   - `cmd /c "pushd <repo-root> && build.bat"`, or
   - a `subst` drive letter / native Windows checkout.
4. **Tools:** `build.bat` prefers `build\bin\gn.exe` (+ ninja on PATH/`build\bin`), then `D:\Dev\buildtools` + `D:\Dev\depot_tools`, else `python build\fetch_binaries.py`. If tools are missing, fetch or set `BUILDTOOLS_PATH` — do not silently build from `C:\Windows`.

```bat
.\build.bat
.\build.bat te
```

Optional first arg: mogu-style aliases `m` / `te` / `a` / `b`, or a raw ninja target. Optional: set `BUILDTOOLS_PATH` before invoke if tools live outside the defaults.

## Fix loop (log-driven)

1. Run `.\build.bat` from repo root.
2. Parse **`out/build.log`** (and bat/console stderr for tool-missing / UNC failures):
   - `FAILED:` / `ninja: build stopped`
   - every real **`error:`** / `undefined reference` / fatal diagnostic
   - actionable **`warning:`** and **`-Werror`** promotions
3. Fix **root cause** in source (not `#if 0`, not deleting tests, not `-Wno-*` / `#pragma` silencing unless the user asked).
4. Rebuild via the same `.\build.bat`.
5. Repeat until the **done bar**.
6. Widen scope only if the user asked for a larger build.

**Done bar**

| Goal | Required |
| --- | --- |
| Default green | Exit **0**, no `FAILED:`, no remaining **`error:`** / linker failures in **`out/build.log`** |
| Clean warnings / 零警告 / `-Werror`-clean | Same, **plus** no remaining actionable **`warning:`** |

Exit 0 with leftover actionable warnings is **not** done when warning-clean was requested.

```bash
# From repo root (WSL or ripgrep on Windows)
rg -n 'FAILED:|^ninja: build stopped|error:|warning:|-Werror' out/build.log | head -80
```

Prefer the **first** actionable diagnostic; later lines are often fallout.

## Evidence before success

Claim green **only** after a fresh successful `.\build.bat` that meets the done bar. Show:

- The command run
- Exit code `0`
- Brief confirmation from **`out/build.log`** (no `FAILED:` / no remaining `error:`; and no remaining actionable `warning:` when requested)

## Hard stops (stop and report)

- Missing tools / drive (`D:` gone) / network fetch failure the user must fix
- Ambiguous requirement that blocks a correct fix
- Same error unchanged **3+** times with no progress
- Disk-space / OOM needing host config changes

## Communication

- Progress summaries in **简体中文** when the repo/user prefers Chinese
- Code, comments, and identifiers in **English**
