#!/usr/bin/env python3
# Copyright (c) 2026 The Mogu Authors.
# All rights reserved.

import shutil
import sys
from pathlib import Path


def main() -> int:
    if len(sys.argv) < 3 or (len(sys.argv) - 1) % 2 != 0:
        print("usage: copy_demo_data.py src dst [src dst ...]", file=sys.stderr)
        return 1
    args = sys.argv[1:]
    for i in range(0, len(args), 2):
        src = Path(args[i])
        dst = Path(args[i + 1])
        dst.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(src, dst)
    return 0


if __name__ == "__main__":
    sys.exit(main())
