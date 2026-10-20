# Copyright (c) 2026 The Mogu Authors.
# All rights reserved.
"""Sync smartgis third_party pins/GN into mview (scheme B)."""

from __future__ import annotations

import json
import shutil
import subprocess
from pathlib import Path

MVIEW = Path(r"C:\Dev\src\gis\mview")
SMART = Path(r"C:\Dev\src\gis\smartgis")
MTP = MVIEW / "third_party"
STP = SMART / "third_party"

# Shell UI stacks — never copy / junction / manifest.
EXCLUDE_DIRS = {
    "bcg",
    "cef",
    "webview2",
    "windows_app_sdk",
    "chromium",
    "python",
    ".build",
    ".src",
    ".install",
    ".tools",
    "tools",
    "gn",
}

# Do not overwrite these .src trees (Chromium / already pinned).
SRC_KEEP = {
    "zlib",
    "skia",
    "libpng",
    "libjpeg_turbo",
    "modp_b64",
    "wtl",
    "yasm",
    "jstemplate",
    "iaccessible2",
    "gtest",
    "gmock",
    "maplibre-native",
    "vtk",
    "cesium-native",
    "cesium-unreal",
}

THIN_COPY = [
    "antlr4",
    "assimp",
    "CLI11",
    "concurrentqueue",
    "CxImage",
    "cximage_pub",
    "ed25519",
    "flycube",
    "khronos_gl",
    "laszip",
    "octree",
    "protobuf",
    "pugixml",
    "rapidjson",
    "tinygltf",
    "vir_simd",
    "yoga",
]


def run(cmd: list[str]) -> None:
    print("+", " ".join(cmd))
    subprocess.check_call(cmd)


def merge_manifest() -> None:
    m = json.loads((MTP / "manifest.json").read_text(encoding="utf-8"))
    s = json.loads((STP / "manifest.json").read_text(encoding="utf-8"))
    by_name = {p["name"]: p for p in m["packages"]}
    added = []
    for pkg in s["packages"]:
        name = pkg["name"]
        if name in EXCLUDE_DIRS:
            continue
        if name == "zlib":
            continue
        if name == "maplibre-native":
            continue
        if name in by_name and by_name[name].get("source_policy") == "vendored":
            continue
        note = pkg.get("note", "")
        pkg = dict(pkg)
        if "Synced from smartgis" not in note:
            pkg["note"] = (
                (note + " " if note else "")
                + "Synced from smartgis for on-demand use in mview. "
                "Default product GN graph does not link this."
            ).strip()
        by_name[name] = pkg
        added.append(name)

    mview_names = [p["name"] for p in m["packages"]]
    out = []
    seen = set()
    for n in mview_names:
        if n in by_name:
            out.append(by_name[n])
            seen.add(n)
    for pkg in s["packages"]:
        n = pkg["name"]
        if n in seen or n not in by_name:
            continue
        out.append(by_name[n])
        seen.add(n)

    m["packages"] = out
    m["description"] = (
        "mview third_party pins. Chromium snapshots are source_policy:vendored. "
        "GIS/utility packages synced from smartgis "
        "(CEF/WebView2/WinAppSDK/BCG excluded). "
        "Opt-in fetch via build.bat t / tools/fetch.py; "
        "local junctions to smartgis .src allowed."
    )
    if "gitea_mirror" not in m and "gitea_mirror" in s:
        m["gitea_mirror"] = s["gitea_mirror"]
    (MTP / "manifest.json").write_text(
        json.dumps(m, indent=2, ensure_ascii=False) + "\n", encoding="utf-8"
    )
    print("manifest added/updated:", ", ".join(added))


def copy_thin() -> None:
    for name in THIN_COPY:
        src = STP / name
        if not src.is_dir():
            print("missing thin", name)
            continue
        dst = MTP / name
        if dst.exists():
            shutil.rmtree(dst)
        shutil.copytree(
            src,
            dst,
            ignore=shutil.ignore_patterns(".git", "__pycache__", "*.pyc"),
        )
        print("copied thin", name)


def copy_gn() -> None:
    gn_dst = MTP / "gn"
    gn_dst.mkdir(parents=True, exist_ok=True)
    for fname in ("BUILD.gn", "deps.gni", "dll_search.cc", "dll_search.h", "tp.gni"):
        src = STP / "gn" / fname
        if not src.exists():
            continue
        raw = src.read_bytes()
        try:
            text = raw.decode("utf-8")
        except UnicodeDecodeError:
            text = raw.decode("utf-8", errors="replace")
        if fname == "BUILD.gn":
            text = text.replace(
                'import("//build/smartgis.gni")',
                'import("//third_party/gn/presence.gni")',
            )
            text = text.replace(
                'group("bcg") {\n  public_deps = [ "//third_party/bcg:bcg" ]\n}\n\n',
                "",
            )
            # Drop python/chromium facades — not used in mview shell.
            text = text.replace(
                'config("chromium_cfg") {\n'
                '  include_dirs += [ "//third_party/.src/chromium" ]\n'
                "}\n\n",
                "",
            )
            text = text.replace(
                'config("python_cfg") {\n'
                '  include_dirs += [ "//third_party/python/include" ]\n'
                '  lib_dirs = [ "//third_party/python/libs" ]\n'
                '  libs += [ "python312.lib" ]\n'
                "}\n\n",
                "",
            )
            text = text.replace(
                'group("chromium") {\n  public_configs = [ ":chromium_cfg" ]\n}\n\n',
                "",
            )
            text = text.replace(
                'group("python") {\n  public_configs = [ ":python_cfg" ]\n}\n\n',
                "",
            )
        (gn_dst / fname).write_text(text, encoding="utf-8")
        print("wrote gn/", fname)

    # presence.gni is maintained in-tree (file_exists, no read_file on missing).
    print("keep gn/presence.gni (in-tree)")


def write_top_build_gn() -> None:
    raw = (STP / "BUILD.gn").read_bytes()
    try:
        text = raw.decode("utf-8")
    except UnicodeDecodeError:
        text = raw.decode("utf-8", errors="replace")
    text = text.replace(
        'group("bcg") {\n  public_deps = [ "//third_party/gn:bcg" ]\n}\n\n',
        "",
    )
    text = text.replace(
        'group("chromium") {\n  public_deps = [ "//third_party/gn:chromium" ]\n}\n\n',
        "",
    )
    text = text.replace(
        'group("python") {\n  public_deps = [ "//third_party/gn:python" ]\n}\n\n',
        "",
    )
    marker = 'group("third_party_all")'
    idx = text.find(marker)
    if idx >= 0:
        text = text[:idx] + """group("third_party_all") {
  # Chromium snapshots always available. GIS/utility packages are opt-in
  # via //third_party:<name> after fetch/install or local junctions.
  public_deps = [
    "//third_party/.src/libjpeg_turbo:libjpeg",
    "//third_party/.src/libpng:libpng",
    "//third_party/.src/modp_b64:modp_b64",
    "//third_party/.src/zlib:zlib",
  ]
}
"""
    (MTP / "BUILD.gn").write_text(text, encoding="utf-8")
    print("wrote third_party/BUILD.gn")


def junction_srcs() -> None:
    src_root = STP / ".src"
    dst_root = MTP / ".src"
    dst_root.mkdir(parents=True, exist_ok=True)
    results = []
    for entry in sorted(src_root.iterdir()):
        name = entry.name
        if name.startswith("_") or name.endswith(".tar.gz") or name.endswith(".zip"):
            continue
        if name in SRC_KEEP:
            results.append(f"keep {name}")
            continue
        if name in EXCLUDE_DIRS:
            results.append(f"exclude {name}")
            continue
        link = dst_root / name
        if link.exists():
            results.append(f"exists {name}")
            continue
        try:
            run(["cmd", "/c", "mklink", "/J", str(link), str(entry.resolve())])
            results.append(f"ok {name}")
        except subprocess.CalledProcessError as e:
            results.append(f"fail {name}: {e}")
    print("src junctions:")
    for r in results:
        print(" ", r)


def junction_install_hint() -> None:
    mi = MTP / ".install"
    si = STP / ".install"
    marker = MTP / ".install_SMARTGIS_JUNCTION.txt"
    marker.write_text(
        "mview already has third_party/.install. To reuse smartgis install "
        "products for a missing package, junction individual include/lib/bin "
        f"entries from:\n  {si}\n"
        "Do not replace the whole .install tree (Chromium / existing cmake "
        "outputs may already live here).\n"
        "Excluded: cef, webview2, windows_app_sdk, bcg.\n",
        encoding="utf-8",
    )
    print(
        "wrote",
        marker,
        "mview .install exists=",
        mi.exists(),
        "smartgis=",
        si.exists(),
    )


def main() -> None:
    merge_manifest()
    copy_thin()
    copy_gn()
    write_top_build_gn()
    junction_srcs()
    junction_install_hint()
    print("DONE sync")


if __name__ == "__main__":
    main()
