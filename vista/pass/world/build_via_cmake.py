# Copyright (c) 2026 The Mogu Authors.
# All rights reserved.

"""Build vista_world.dll via the cesium-flycube CMake tree (Release /MD).

Sources live under vista/pass/world; CMakeLists points at them. GN invokes
this so product_presenters can data-dep the DLL without re-listing the full
Cesium + vcpkg transitive link set.
"""

from __future__ import annotations

import argparse
import os
import shutil
import subprocess
import sys


def _find_cmake() -> str:
    env = os.environ.get("CMAKE")
    if env and os.path.isfile(env):
        return env
    for candidate in (
        r"C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe",
        r"C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe",
    ):
        if os.path.isfile(candidate):
            return candidate
    return "cmake"


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--cmake-build-dir", required=True)
    parser.add_argument("--out-dll", required=True)
    parser.add_argument("--out-lib", required=True)
    args = parser.parse_args()

    build_dir = os.path.abspath(args.cmake_build_dir)
    if not os.path.isdir(build_dir):
        print(
            f"error: cmake build dir missing: {build_dir}\n"
            "Configure once under third_party/cesium-flycube/build (Release).",
            file=sys.stderr,
        )
        return 1

    cmake = _find_cmake()
    # Reconfigure so target renames (mview_flycube → vista_world) pick up.
    conf = [cmake, build_dir]
    print("+", " ".join(conf), flush=True)
    subprocess.check_call(conf)
    cmd = [
        cmake,
        "--build",
        build_dir,
        "--config",
        "Release",
        "--target",
        "vista_world",
    ]
    print("+", " ".join(cmd), flush=True)
    subprocess.check_call(cmd)

    src_dll = os.path.join(build_dir, "Release", "vista_world.dll")
    src_lib = os.path.join(build_dir, "Release", "vista_world.lib")
    if not os.path.isfile(src_dll):
        print(f"error: expected {src_dll}", file=sys.stderr)
        return 1

    os.makedirs(os.path.dirname(os.path.abspath(args.out_dll)), exist_ok=True)
    shutil.copy2(src_dll, args.out_dll)
    if os.path.isfile(src_lib):
        shutil.copy2(src_lib, args.out_lib)
    print(f"copied {src_dll} -> {args.out_dll}", flush=True)
    return 0


if __name__ == "__main__":
    sys.exit(main())
