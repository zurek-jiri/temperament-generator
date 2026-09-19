# Copyright (C) 2026 zurek-jiri and contributors.
# SPDX-License-Identifier: AGPL-3.0-only
"""Checks for desktop icons shipped in native release packages."""
from pathlib import Path
import plistlib
import struct


def check_png(path):
    data = Path(path).read_bytes()
    if data[:8] != b"\x89PNG\r\n\x1a\n" or data[12:16] != b"IHDR" or struct.unpack_from(">II", data, 16) != (512, 512):
        raise ValueError("The Linux launcher needs the exported 512x512 application icon.")


def check_mac_icon(bundle):
    contents = Path(bundle) / "Contents"
    info = plistlib.loads((contents / "Info.plist").read_bytes())
    name = info.get("CFBundleIconFile", "")
    if not name or Path(name).name != name:
        raise ValueError("Mac bundle has no valid CFBundleIconFile.")
    if not name.endswith(".icns"):
        name += ".icns"
    data = (contents / "Resources" / name).read_bytes()
    if data[:4] != b"icns" or len(data) < 16 or struct.unpack_from(">I", data, 4)[0] != len(data):
        raise ValueError("Mac application icon is missing or malformed.")
    offset, has_large = 8, False
    while offset < len(data):
        if offset + 8 > len(data):
            raise ValueError("Truncated Mac icon record.")
        kind, size = struct.unpack_from(">4sI", data, offset)
        if size <= 8 or offset + size > len(data):
            raise ValueError("Invalid Mac icon record length.")
        has_large |= kind in (b"ic08", b"ic09", b"ic10", b"ic13", b"ic14")
        offset += size
    if not has_large:
        raise ValueError("Mac application icon has no large image representation.")
    return contents / "Resources" / name
