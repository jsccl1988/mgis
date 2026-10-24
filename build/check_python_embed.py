# Copyright (c) 2026 The Mogu Authors.
# All rights reserved.
"""GN probe: print a quoted GN string 'ok' or 'missing' for exec_script value."""

from __future__ import annotations

import sys
from pathlib import Path


def main(argv: list[str]) -> int:
    # GN exec_script(..., "value") needs a quoted GN string literal on stdout.
    result = "missing"
    if len(argv) == 5:
        prefix, header, lib, dll = argv[1], argv[2], argv[3], argv[4]
        if prefix and Path(header).is_file() and Path(lib).is_file() and Path(
                dll).is_file():
            result = "ok"
    # Always quote so a bare token never reaches GN.
    sys.stdout.write('"%s"\n' % result)
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
