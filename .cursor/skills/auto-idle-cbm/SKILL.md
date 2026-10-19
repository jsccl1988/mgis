---
name: auto-idle-cbm
description: >-
  Lightweight idle entry that refreshes only the CBM index. Use when the user
  invokes /auto-idle-cbm, or says 闲置更新CBM, 空闲时更新索引, 闲置时更新CBM.
  Arms an optional 1h Loop with sentinel AGENT_LOOP_TICK_idle_cbm. Does not run
  build, tests, or commit — only auto-cbm-gen after the idle gate.
---

<!--
Copyright (c) 2026 The Mogu Authors.
All rights reserved.
-->

# Auto idle CBM (lightweight)

**闲置时只更新 CBM 索引**（不跑编译 / 测试 / 提交）

Same idle gate as `/auto-idle-pipeline`. After idle (or `force` / `跳过闲置检查`), follow `.cursor/skills/auto-cbm-gen/SKILL.md` only.

## Idle detection

- Transcripts: `C:/Users/LENOVO/.cursor/projects/c-Dev-src-gis-mview/agent-transcripts`
- Terminals: `C:/Users/LENOVO/.cursor/projects/c-Dev-src-gis-mview/terminals/*.txt`

Active if another transcript changed in the last **~10 minutes** and the turn is still in progress. Exclude this conversation. Skip and report if ambiguous.

Do not run `auto-build-fix`, `auto-bug-fix`, or `auto-commit-push`.

CBM target: project `mview`, root `C:/Dev/src/gis/mview`.

## Arming the 1h loop

```powershell
while ($true) {
  Start-Sleep -Seconds 3600
  Write-Output 'AGENT_LOOP_TICK_idle_cbm {"prompt":"/auto-idle-cbm"}'
}
```

Do not duplicate an existing `AGENT_LOOP_TICK_idle_cbm` loop. It may coexist with the pipeline loop. Run once immediately after arming. Do not use `open_automation`.

Stay on `master`. No git writes. Progress in **简体中文**.
