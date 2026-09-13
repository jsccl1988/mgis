<!--
Copyright (c) 2026 The Mogu Authors.
All rights reserved.
-->

# testing

GN helpers for unit tests and (optional) benchmarks.

| File | Role |
| --- | --- |
| `test.gni` | `test("name")` → executable + gtest/gmock/glog/benchmark libs |
| `benchmark.gni` | `benchmark("name")` → executable + googlebenchmark |
| `main.cpp` | Shared `test()` entry (`base::Initialize` + `RUN_ALL_TESTS`) |
| `benchmark_main.cpp` | Shared `benchmark()` entry |
| `test.h` | gtest helpers (`ASSERT_THROWS_WITH`, type checks) |

## Register a test

In the module `BUILD.gn`:

```gn
import("//testing/test.gni")

test("core_test") {
  sources = [ "core_unittest.cpp" ]
  deps = [ ":core" ]
}
```

Then add `"//<module>:<name>"` to root `//:test_all` (`BUILD.gn`).

## Run

From the repo root (after `gn gen`):

```bat
build.bat te
out\core_test.exe
```

Aliases match mogu: `te` = `//:test_all`, `a` = `//:all_with_tests`, `b` = `//:benchmark_all`.

---

**最后更新：** 2026-09-12
