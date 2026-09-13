# mgis 工程管理对齐 mogu

> Internal plan; execute in this session. Do not commit.

**Goal:** Adopt mogu’s GN/project-management patterns in `gis/mgis` while keeping mgis names, targets, and product code.

**Source of truth:** WSL `/home/ccl/dev/src/mogu` (Windows `c:\Dev\src\mogu` is docs-only stubs).

**Architecture:** Smallest overlay: root groups + aliases, complete `third_party_all`, benchmark template, build.bat target dispatch, developer docs, shared Cursor PM/style rules. Do not migrate mogu’s Linux `.install`/Bazel/incubator stack.

## Do

- Root `all` / `all_with_tests` / `test_all` / `benchmark_all` + `m`/`a`/`te`/`b`
- Module `*_all` groups; register existing `*_test` targets in `test_all`
- `third_party_all` lists every cmake/GN third-party target
- `testing/benchmark.gni` + `benchmark_main.cpp`
- Named `//build/config:warnings` (do not flip default `/W3`)
- `build.bat` optional target aliases
- `.clang-format`, `.editorconfig`, richer `.gitignore`
- Root + `build/` README
- Cursor rules: build-output (mgis `out/third_party`), copyright, no-coauthor, root-readme, master-only, style/*

## Skip

- Bazel, `third_party/.install`, incubator/tcmalloc/mysql args, Linux sanitizer wiring
- `agent-no-auto-build` (conflicts with existing `auto-build-fix`)
- Product rules (`infra/*`, `common/table`, app/kode, …)
- `/W3` → `/W4` (would flood warnings while another agent is fixing compiles)

## Verify

`gn gen out` then `gn desc` / `gn ls` on new groups. Full product ninja only if it does not contend on `out/`.
