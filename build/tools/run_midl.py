# Copyright (c) 2026 The Mogu Authors.
# All rights reserved.
"""Run midl.exe on ia2_api_all.idl into the GN gen dir."""

from __future__ import annotations

import os
import subprocess
import sys
from pathlib import Path


def _midl() -> str:
    program_files = os.environ.get("ProgramFiles(x86)", r"C:\Program Files (x86)")
    vswhere = Path(program_files) / "Microsoft Visual Studio" / "Installer" / "vswhere.exe"
    if vswhere.is_file():
        install = subprocess.check_output(
            [
                str(vswhere),
                "-latest",
                "-products",
                "*",
                "-requires",
                "Microsoft.VisualStudio.Component.VC.Tools.x86.x64",
                "-property",
                "installationPath",
            ],
            text=True,
        ).strip()
        hits = list(Path(install).glob("VC/Tools/MSVC/*/bin/Hostx64/x64/midl.exe"))
        if not hits:
            kit = Path(r"C:\Program Files (x86)\Windows Kits\10\bin")
            hits = sorted(kit.glob("**/x64/midl.exe"))
        if hits:
            return str(hits[-1])
    return "midl.exe"


def _vcvars() -> Path:
    program_files = os.environ.get("ProgramFiles(x86)", r"C:\Program Files (x86)")
    vswhere = Path(program_files) / "Microsoft Visual Studio" / "Installer" / "vswhere.exe"
    install = subprocess.check_output(
        [
            str(vswhere),
            "-latest",
            "-products",
            "*",
            "-requires",
            "Microsoft.VisualStudio.Component.VC.Tools.x86.x64",
            "-property",
            "installationPath",
        ],
        text=True,
    ).strip()
    return Path(install) / "VC" / "Auxiliary" / "Build" / "vcvarsall.bat"


def main(argv: list[str]) -> int:
    if len(argv) != 3:
        print("usage: run_midl.py <idl> <out-dir>", file=sys.stderr)
        return 2
    idl = Path(argv[1]).resolve()
    out = Path(argv[2]).resolve()
    out.mkdir(parents=True, exist_ok=True)
    midl = _midl()
    command = (
        f'call "{_vcvars()}" x64 >nul && "{midl}" /nologo /env x64 '
        f'/h ia2_api_all.h /iid ia2_api_all_i.c "{idl}"'
    )
    print(command)
    return subprocess.call(command, cwd=out, shell=True)


if __name__ == "__main__":
    raise SystemExit(main(sys.argv))
