#!/usr/bin/env python3
"""Verify the firmware version is consistent across the two places that declare it.

`custom_prog_version` in platformio.ini names the output binary (via
name_firmware.py); `VERSION_152` in include/FCS152_KDU.h is what the radio shows
on screen. Nothing keeps them in step, so a release can ship a binary whose
filename disagrees with its own about-screen.

The two use different punctuation on purpose ("Rev-2.2.5507" vs "Rev 2.2.5507"),
so only the dotted numeric part is compared.

Why this does not just grep for VERSION_152: the header defines it *twice* --
once in the `#ifndef __NEW__` branch (the dead STM32-era value, "Rev 1.0.0000")
and once in the `#else` (the live one). A naive regex picks the first match and
reports a false mismatch. This script locates the `#else` branch explicitly and
refuses to guess if the structure is not what it expects.

Exit codes: 0 in sync, 1 out of sync, 2 could not parse (treated as failure).
"""
from __future__ import annotations

import re
import sys
from pathlib import Path

REPO = Path(__file__).resolve().parents[2]
INI = REPO / "platformio.ini"
HEADER = REPO / "include" / "FCS152_KDU.h"

# The guard is only meaningful while the header keeps this shape. If someone
# restructures it, fail loudly rather than silently reading the wrong constant.
GUARD_BLOCK = re.compile(
    r"#ifndef\s+__NEW__\b(?P<legacy>.*?)#else(?P<active>.*?)#endif",
    re.DOTALL,
)


def die(code: int, msg: str, file: str = "include/FCS152_KDU.h") -> None:
    """Emit a GitHub workflow annotation against the file that is actually wrong."""
    print(f"::error file={file}::{msg}" if code else msg)
    sys.exit(code)


def numeric(version: str) -> str:
    """'Rev-2.2.5507' and 'Rev 2.2.5507' both reduce to '2.2.5507'."""
    m = re.search(r"\d+(?:\.\d+)+", version)
    return m.group(0) if m else ""


def read_ini_version(text: str) -> str:
    m = re.search(r"^\s*custom_prog_version\s*=\s*(\S+)", text, re.M)
    if not m:
        die(2, "no 'custom_prog_version' key found", file="platformio.ini")
    return m.group(1)


def read_header_version(text: str) -> str:
    block = GUARD_BLOCK.search(text)
    if not block:
        die(2, "FCS152_KDU.h: expected an '#ifndef __NEW__ ... #else ... #endif' "
               "block around VERSION_152; the header has been restructured and "
               "this guard needs updating (see .github/scripts/check_version_sync.py)")

    active = re.findall(r'#define\s+VERSION_152\s+"([^"]+)"', block.group("active"))
    if len(active) != 1:
        die(2, f"FCS152_KDU.h: expected exactly one VERSION_152 in the active "
               f"'#else' branch, found {len(active)}")

    # Sanity check that the branch we skipped really is the legacy one.
    legacy = re.findall(r'#define\s+VERSION_152\s+"([^"]+)"', block.group("legacy"))
    if len(legacy) != 1:
        die(2, f"FCS152_KDU.h: expected exactly one VERSION_152 in the inactive "
               f"'#ifndef __NEW__' branch, found {len(legacy)}")

    return active[0]


def main() -> None:
    for path in (INI, HEADER):
        if not path.is_file():
            rel = path.relative_to(REPO).as_posix()
            die(2, f"missing expected file: {rel}", file=rel)

    ini_raw = read_ini_version(INI.read_text(encoding="utf-8", errors="replace"))
    hdr_raw = read_header_version(HEADER.read_text(encoding="utf-8", errors="replace"))
    ini_num, hdr_num = numeric(ini_raw), numeric(hdr_raw)

    if not ini_num or not hdr_num:
        die(2, f"could not extract a dotted version number from "
               f"custom_prog_version={ini_raw!r} / VERSION_152={hdr_raw!r}")

    print(f"platformio.ini  custom_prog_version = {ini_raw!r}  -> {ini_num}")
    print(f"FCS152_KDU.h    VERSION_152         = {hdr_raw!r}  -> {hdr_num}")

    if ini_num != hdr_num:
        die(1, f"firmware version mismatch: platformio.ini says {ini_num}, "
               f"include/FCS152_KDU.h says {hdr_num}. Update both, or the binary "
               f"filename will disagree with the version shown on the radio.")

    print(f"\nIn sync: {ini_num}")


if __name__ == "__main__":
    main()
