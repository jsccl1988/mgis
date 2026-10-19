# Copyright (c) 2026 The Mogu Authors.
# All rights reserved.
"""Pack chrome_scaled_image bytes into a Chromium data pack (version 4).

Resource ids match build/tools/emit_grit_header.py: first occurrence of each
name="..." attribute, starting at 1000. ResourceBundle on Windows loads
ui_resources_100_percent.pak from the module directory.
"""

from __future__ import annotations

import re
import struct
import sys
from pathlib import Path

_NAME = re.compile(r'\bname="([A-Za-z0-9_]+)"')
_STRUCTURE = re.compile(
    r'<structure\b[^>]*\btype="([^"]+)"[^>]*\bname="([^"]+)"[^>]*\bfile="([^"]+)"'
    r'|<structure\b[^>]*\bname="([^"]+)"[^>]*\btype="([^"]+)"[^>]*\bfile="([^"]+)"'
    r'|<structure\b[^>]*\bfile="([^"]+)"[^>]*\btype="([^"]+)"[^>]*\bname="([^"]+)"'
)

_ID_BASE = 1000
_PACK_VERSION = 4
_ENCODING_BINARY = 0


def assign_ids(grd_text: str) -> dict[str, int]:
    ids: dict[str, int] = {}
    next_id = _ID_BASE
    for match in _NAME.finditer(grd_text):
        name = match.group(1)
        if name in ids:
            continue
        ids[name] = next_id
        next_id += 1
    return ids


def structure_files(grd_text: str) -> dict[str, tuple[str, str]]:
    found: dict[str, tuple[str, str]] = {}
    for match in _STRUCTURE.finditer(grd_text):
        if match.group(1):
            kind, name, filename = match.group(1), match.group(2), match.group(3)
        elif match.group(4):
            name, kind, filename = match.group(4), match.group(5), match.group(6)
        else:
            filename, kind, name = match.group(7), match.group(8), match.group(9)
        if name not in found:
            found[name] = (kind, filename)
    return found


def resolve_scaled(grd: Path, filename: str, scale_dir: str) -> Path | None:
    scaled = grd.parent / scale_dir / filename
    if scaled.is_file():
        return scaled
    plain = grd.parent / filename
    if plain.is_file():
        return plain
    return None


def write_pack(path: Path, blobs: list[tuple[int, bytes]]) -> None:
    blobs.sort(key=lambda item: item[0])
    count = len(blobs)
    header_len = 4 + 4 + 1
    index_len = (count + 1) * 6
    data_offset = header_len + index_len
    payload = b"".join(blob for _, blob in blobs)
    out = bytearray()
    out += struct.pack("<IIB", _PACK_VERSION, count, _ENCODING_BINARY)
    cursor = data_offset
    for resource_id, blob in blobs:
        if resource_id > 0xFFFF:
            raise SystemExit(f"resource id {resource_id} does not fit in uint16")
        out += struct.pack("<HI", resource_id, cursor)
        cursor += len(blob)
    out += struct.pack("<HI", 0, cursor)
    out += payload
    if cursor != len(out):
        raise SystemExit(
            f"pack size mismatch: index end {cursor}, buffer {len(out)}")
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(out)


def main(argv: list[str]) -> int:
    if len(argv) != 4:
        print(
            "usage: emit_resource_pak.py <grd> <out.pak> <scale-dir>",
            file=sys.stderr,
        )
        return 2
    grd = Path(argv[1])
    out_path = Path(argv[2])
    scale_dir = argv[3]
    text = grd.read_text(encoding="utf-8", errors="replace")
    ids = assign_ids(text)
    files = structure_files(text)
    blobs: list[tuple[int, bytes]] = []
    missing = 0
    for name, (kind, filename) in files.items():
        if kind != "chrome_scaled_image":
            continue
        resource_id = ids.get(name)
        if resource_id is None:
            continue
        image = resolve_scaled(grd, filename, scale_dir)
        if image is None:
            missing += 1
            continue
        blobs.append((resource_id, image.read_bytes()))
    if not blobs:
        print(f"no images packed from {grd}", file=sys.stderr)
        return 1
    write_pack(out_path, blobs)
    print(
        f"packed {len(blobs)} images, skipped {missing} missing files -> {out_path}"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv))
