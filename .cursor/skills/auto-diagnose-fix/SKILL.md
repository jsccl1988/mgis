---
name: auto-diagnose-fix
description: >-
  Use when diagnosing Windows native crashes, hangs, freezes, deadlocks,
  minidumps, access violations, or faulting stacks with WinDbg/cdb; also when
  the user says crash, 崩溃, 卡死, hang, freeze, deadlock, not responding,
  timeout, dmp, AV, !analyze, 自动诊断, /auto-diagnose-fix, or hands over a
  .dmp / hung PID. For this repo: cdb; PDBs beside the PE (often under out/);
  logs under out/crash/; CBM project mview. After the root-cause fix, verify
  with the same repro or auto-bug-fix. Supersedes windbg-crash-diagnose.
---

<!--
Copyright (c) 2026 The Mogu Authors.
All rights reserved.
-->

# Auto diagnose-fix (crash + hang)

**Windows 原生崩溃 / 卡死 → dump → cdb 分析 → CBM → 修根因 → 验证**

Prefer **`cdb.exe`**. Do not open the WinDbg GUI unless cdb is missing and the user explicitly asks.

**REQUIRED BACKGROUND:** superpowers:systematic-debugging.
**REQUIRED SUB-SKILL:** codebase-memory (project `mview`) before repo-wide Grep.

## Authorization

When this skill is invoked, the agent **MUST** classify → find cdb → dump/analyze → locate → fix → verify.

## Classify

| Symptom | Mode | Script |
|---------|------|--------|
| AV, crash exit, WER `.dmp` | **crash** | `analyze_dump.ps1` / `run_and_catch.ps1` |
| Freeze, deadlock, hang, timeout, PE stuck | **hang** | `dump_hang.ps1` / `run_and_hang.ps1` |
| Ambiguous | hang dump if PID still alive; else crash catch | |

## Hard rules

1. `scripts/find_cdb.ps1` first. Missing cdb → hard stop; ask for Debugging Tools for Windows.
2. Defaults: `PdbDir` = directory that holds the PE’s PDB (often `out`), `OutDir=out/crash`.
3. Use the scripts for `-c`. Do not hand-roll cdb command strings.
4. CBM project **`mview`**, root `C:/Dev/src/gis/mview`.
5. Stay on **`master`**.
6. Kill leftover cdb / target via non-sandbox shell (`agent-terminal-kill`): gentle stop first.
7. Fix root cause. Not `#if 0`, not “increase timeout” as the only fix for a deadlock.

## Commands (repo root)

```powershell
powershell -NoProfile -File .cursor/skills/auto-diagnose-fix/scripts/find_cdb.ps1
powershell -NoProfile -File .cursor/skills/auto-diagnose-fix/scripts/analyze_dump.ps1 -DumpPath out\crash\foo.dmp
powershell -NoProfile -File .cursor/skills/auto-diagnose-fix/scripts/analyze_dump.ps1 -DumpPath out\crash\foo.dmp -Mode Hang
powershell -NoProfile -File .cursor/skills/auto-diagnose-fix/scripts/run_and_catch.ps1 -ExePath <path-to.exe>
powershell -NoProfile -File .cursor/skills/auto-diagnose-fix/scripts/dump_hang.ps1 -ProcessId <pid>
powershell -NoProfile -File .cursor/skills/auto-diagnose-fix/scripts/run_and_hang.ps1 -ExePath <path-to.exe> -TimeoutSec 180
powershell -NoProfile -File .cursor/skills/auto-diagnose-fix/scripts/archive_console_diag.ps1 -InputLog 'out/crash/a-catch.log;out/crash/a-analyze.log' -Label repro
```

Read the digest under `out/crash/` next to the faulting frame. Mine `LOG(ERROR)` / `LOG(INFO)` lines from the catch log (this tree’s `base/logging.h`, not smartgis `LOGGING` macros).

## Verify

Re-run the same exe until it no longer crashes or hangs. If entered from `auto-bug-fix`, that skill’s gtest done bar still applies. Rebuild with `auto-build-fix` when PDBs are stale.

**Done bar:** analyze report (exception or hang stacks + suspected symbol) **and** the repro no longer fails.

## Hard stops

- `cdb.exe` not found
- No dump and no attachable PID
- Same failure unchanged **3+** times
- Missing PDB for the faulting module — rebuild first

Progress in **简体中文**. See [reference.md](reference.md).
