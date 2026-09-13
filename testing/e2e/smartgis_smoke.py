#!/usr/bin/env python3
# Copyright (c) 2026 The MGIS Authors.
# All rights reserved.
"""Launch out/smartgis.exe (cwd=out), assert stay-up + world_bind.log layers=3.

  python testing/e2e/smartgis_smoke.py
  python testing/e2e/smartgis_smoke.py --exe C:\\Dev\\src\\gis\\mgis\\out\\smartgis.exe
"""

from __future__ import annotations

import argparse
import subprocess
import sys
import time
from pathlib import Path


def parse_args() -> argparse.Namespace:
    root = Path(__file__).resolve().parents[2]
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--exe",
        default=str(root / "out" / "smartgis.exe"),
        help="Path to smartgis.exe",
    )
    parser.add_argument("--hold-sec", type=float, default=3.0)
    parser.add_argument("--timeout-sec", type=float, default=12.0)
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    exe = Path(args.exe).resolve()
    cwd = exe.parent
    log_path = cwd / "world_bind.log"
    if not exe.is_file():
        print(f"missing {exe}", file=sys.stderr)
        return 2
    if log_path.exists():
        log_path.unlink()

    proc = subprocess.Popen([str(exe)], cwd=str(cwd))
    deadline = time.time() + args.timeout_sec
    log = ""
    try:
        while time.time() < deadline:
            if proc.poll() is not None:
                print(
                    f"smartgis.exe exited early code={proc.returncode}",
                    file=sys.stderr,
                )
                return 1
            if log_path.is_file():
                log = log_path.read_text(encoding="utf-8", errors="replace")
                if "layers=3" in log:
                    break
            time.sleep(0.2)
        else:
            print("timeout waiting for world_bind.log layers=3", file=sys.stderr)
            print(log, file=sys.stderr)
            return 1

        time.sleep(args.hold_sec)
        if proc.poll() is not None:
            print(
                f"smartgis.exe died after bind code={proc.returncode}",
                file=sys.stderr,
            )
            print(log, file=sys.stderr)
            return 1
        for needle in ("layers=3", "Polygon", "Line String", "Point"):
            if needle not in log:
                print(f"bind log missing {needle!r}:\n{log}", file=sys.stderr)
                return 1
        print(f"ok pid={proc.pid} stay-up {args.hold_sec}s")
        print(log.rstrip())
        return 0
    finally:
        if proc.poll() is None:
            proc.terminate()
            try:
                proc.wait(timeout=5)
            except subprocess.TimeoutExpired:
                proc.kill()
                proc.wait(timeout=5)


if __name__ == "__main__":
    sys.exit(main())
