---
name: auto-commit-push
description: >-
  Auto commit on master and push to the tracking remote. Use when the user
  invokes /auto-commit-push, or says 自动提交并push, or when auto-idle-pipeline
  reaches the commit stage. Invoking this skill authorizes commit + push for
  the current turn. Never sets git config; stops if user.name/email is missing.
---

<!--
Copyright (c) 2026 The Mogu Authors.
All rights reserved.
-->

# Auto commit and push

**自动提交并 push**

Independently callable (`/auto-commit-push`) and also stage 4 of `/auto-idle-pipeline`.

Invoking this skill **authorizes** `git commit` and `git push` for this turn only.

## Hard rules

1. Stay on **`master`**. Do not create or switch branches.
2. NEVER update git config. NEVER force-push. NEVER `--no-verify` / `--no-gpg-sign`.
3. NEVER add Cursor `Co-authored-by` trailers.
4. Skip empty commits.
5. Exclude secrets (`.env`, credentials, keys, tokens).

## Identity gate

```powershell
git config user.name
git config user.email
```

If either is empty: **stop**. Report in **简体中文**. Do not invent values and do not run `git config` to fix it.

## Steps

1. Confirm branch is `master`. If not: stop.
2. Identity gate.
3. In parallel: `git status`, `git diff`, `git diff --staged`, `git log -5 --oneline`.
4. If nothing to commit: report and stop. No empty commit, no push.
5. Draft a 1–2 sentence message focused on **why**.
6. `git add` relevant files. Exclude secrets.
7. Commit:

```powershell
$msg = @'
Commit message here.

'@
git commit -m $msg
```

8. Verify `git log -1 --format=%B` has no Cursor co-author trailer.
9. `git push` to the tracking remote. If no upstream: stop. Do not invent remotes.
10. Report in **简体中文**.

## Done bar

Clean skip, **or** a commit on `master` without a Cursor trailer **and** a successful `git push`.

## Hard stops

- Not on `master`
- Missing `user.name` / `user.email`
- Only secrets would be committed
- No upstream / push rejected
- Hook rejects the commit — fix if trivial and create a **new** commit
