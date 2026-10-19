---
name: auto-idle-pipeline
description: >-
  Primary entrypoint for the idle finish pipeline. Use when the user invokes
  /auto-idle-pipeline, or says 空闲流水线, 对话都完成后, 每小时巡检收尾, 四阶段一键收尾,
  or asks to run auto-build-fix → auto-bug-fix → auto-cbm-gen → auto-commit-push
  after other project conversations are idle. Arms an optional 1h Loop with
  sentinel AGENT_LOOP_TICK_idle_pipeline.
---

<!--
Copyright (c) 2026 The Mogu Authors.
All rights reserved.
-->

# Auto idle pipeline (umbrella)

**空闲时一键收尾：编译修复 → 测试 → CBM 索引 → 提交并 push**

Preferred entry: `/auto-idle-pipeline`. Stage skills stay separately callable. Read and follow each stage’s `SKILL.md`. Do not duplicate their internals.

## Preconditions

1. Other agent conversations for **this** project must be idle, unless the user says `force` / `跳过闲置检查` / `强制收尾`.
2. Work only on **`master`**. No Cursor Automation.
3. NEVER set `git user.name` / `user.email`.

## Idle detection

- Transcripts: `C:/Users/LENOVO/.cursor/projects/c-Dev-src-gis-mview/agent-transcripts`
- Terminals: `C:/Users/LENOVO/.cursor/projects/c-Dev-src-gis-mview/terminals/*.txt`

Prefer skipping over colliding with an active agent:

- Active if a transcript was modified in the last **~10 minutes** and the latest turn is still in progress.
- Exclude **this** conversation.
- A terminal with a running command counts as active, except this loop’s own `AGENT_LOOP_TICK_idle_pipeline` / `AGENT_LOOP_TICK_idle_cbm` shells.
- If ambiguous, skip and report **跳过（其他对话仍在进行）**.

## Stage order

| # | Skill | Must achieve |
|---|-------|----------------|
| 1 | `auto-build-fix` | `build.bat debug` exit 0 |
| 2 | `auto-bug-fix` | gtest exit 0, or an explicit “no test binary” skip reported as a stop |
| 3 | `auto-cbm-gen` | index for project `mview` |
| 4 | `auto-commit-push` | commit on `master` + push, or clean skip |

Continue only after the current stage’s done bar.

| Situation | Action |
|-----------|--------|
| Idle check fails | Skip all stages |
| Stage 2: no gtest binary | Stop; do not pretend harness is green |
| Stage 4: clean tree | Skip commit; pipeline done if 1–3 passed |
| Stage 1, 2, or 3 fails | Stop; do not run later stages |

## Arming the 1h loop

```powershell
while ($true) {
  Start-Sleep -Seconds 3600
  Write-Output 'AGENT_LOOP_TICK_idle_pipeline {"prompt":"/auto-idle-pipeline"}'
}
```

Start it in the background once. Do not duplicate an existing loop. Run the pipeline once immediately after arming. Do not use `open_automation`.

Stay on `master`. No force-push. No Cursor `Co-authored-by`. Progress in **简体中文**.
