---
name: auto-bug-fix
description: >-
  Use when the user reports a runtime bug, crash, AV, access violation,
  秒退, WER, 0xc0000005, hang-then-exit, Application Error, faulting module,
  or asks to 自动修复bug or crash，直至成功为止. Compile success is not done —
  this is the runtime counterpart of auto-build-fix. For this repo:
  WinMain desktop binary out/smartgis.exe launched with cwd out/.
---

# Auto bug/crash fix until success

**自动修复bug or crash，直至成功为止**

Runtime counterpart of **auto-build-fix**. A green `build.bat` is **not** done.

**Iron law:** reproduce → evidence → root cause → smallest fix → re-verify. No edit before a confirmed root cause.

## Authorization

When this skill is invoked, attached (`@auto-bug-fix`), or the user reports a crash/bug, the agent **MUST** run the loop until the done bar — do not hand reproduce/diagnose/fix back to the user.

## Classify first

| Observation | This skill? |
| --- | --- |
| Console / unit-test binary finishes `return 0` (expected 秒退) | **No.** Stop. Not a crash. |
| Compile / link failure | **No** until green — apply **auto-build-fix**, then resume. |
| Missing DLL / load failure (no window, or “dll was not found”) | **Yes.** Restore the dependency or load path. |
| Window appears, then AV / hang-then-exit / WER | **Yes.** Map offset; fix root cause. |

Do **not** judge by double-click console tests.

## Fix loop (evidence-driven)

1. **Reproduce** the user’s scenario with the **real** binary they care about. For mgis desktop: `out/smartgis.exe`, **cwd = `out/`** (`_tWinMain` in `gui/main.cpp`). See [reference.md](reference.md).
2. **Collect evidence** *before any edit*: exit code, process lifetime, window created vs not (title **SmartGIS**), stdout/stderr if any, WER / Application Error (faulting module, exception code, fault offset).
3. **Map** faulting module + offset to source (PDB, `dumpbin`, map file, or debugger). Do **not** guess a file from the module name alone (`content.dll` ≠ “edit anything in `content/`”).
4. **Fix the root cause** with the smallest correct change.
5. **Rebuild** via `.\build.bat` from `c:/Dev/src/gis/mgis`. If compile fails, apply **auto-build-fix**, then resume this loop.
6. **Re-run** the same reproduce steps. Confirm the original crash is gone (process stays up / scenario passes). Repeat until the **done bar** or a hard stop.

**Done bar**

| Goal | Required |
| --- | --- |
| Default | Original crash/bug gone on the real binary; process stays up or the failing scenario passes |
| Crash-after-window | Window stays; no matching WER (same module + exception + offset) |

Compile-only success is **not** done.

## Evidence before success

Claim success **only** after a fresh reproduce that meets the done bar. Show:

- Command + cwd
- Lifetime / window still up (or scenario passed)
- No matching WER for the original signature
- Root cause + files changed

Do **not** commit unless asked.

## Hard stops (stop and report)

- Same crash signature after a fix attempt (module + exception + offset unchanged)
- Needs a product / design decision
- Missing toolchain you cannot safely install
- ~8–10 iterations without clear progress

## Forbidden “fixes”

- Empty `catch`, swallow exceptions, `pause`, hide AVs
- `#if 0` or deleting the crashing path to “make it stay up”
- Treating a console `return 0` 秒退 as a crash
- Shipping a rebuild with no re-run of the real binary

## Communication

- Progress summaries in **简体中文** when the repo/user prefers Chinese
- Code, comments, and identifiers in **English**

## Additional resources

- Launch, WER, and symbol mapping: [reference.md](reference.md)
