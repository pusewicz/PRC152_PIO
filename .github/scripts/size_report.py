#!/usr/bin/env python3
"""Turn two PlatformIO size outputs into a markdown delta table for a PR comment.

The ESP32-S2 has 320 KB of RAM and this firmware already links WiFi, WebServer
and ArduinoJson, so a change that quietly eats the remaining headroom is worth
seeing on the PR rather than discovering at flash time.

Usage:
    size_report.py --base base.txt --head head.txt [--out report.md]

Parses lines of the form PlatformIO prints:
    RAM:   [=         ]  15.0% (used 49000 bytes from 327680 bytes)
    Flash: [=======   ]  69.9% (used 916550 bytes from 1310720 bytes)
"""
from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path

LINE = re.compile(
    r"^(?P<name>RAM|Flash):\s*\[[^\]]*\]\s*(?P<pct>[\d.]+)%\s*"
    r"\(used (?P<used>\d+) bytes from (?P<total>\d+) bytes\)",
    re.M,
)


def parse(text: str) -> dict[str, tuple[int, int]]:
    """-> {'RAM': (used, total), 'Flash': (used, total)}"""
    return {
        m.group("name"): (int(m.group("used")), int(m.group("total")))
        for m in LINE.finditer(text)
    }


def human(n: int) -> str:
    return f"{n:,}"


def delta_cell(d: int) -> str:
    if d == 0:
        return "±0"
    return f"{'+' if d > 0 else '−'}{human(abs(d))}"


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--base", type=Path, required=True)
    ap.add_argument("--head", type=Path, required=True)
    ap.add_argument("--out", type=Path)
    a = ap.parse_args()

    base = parse(a.base.read_text(errors="replace"))
    head = parse(a.head.read_text(errors="replace"))

    if not head:
        print("could not parse any size lines from the head build", file=sys.stderr)
        return 2

    rows = []
    for name in ("Flash", "RAM"):
        if name not in head:
            continue
        h_used, h_total = head[name]
        pct = h_used / h_total * 100 if h_total else 0.0
        if name in base:
            b_used, _ = base[name]
            d = h_used - b_used
            rows.append(
                f"| {name} | {human(b_used)} | {human(h_used)} | "
                f"**{delta_cell(d)}** | {pct:.1f}% of {human(h_total)} |"
            )
        else:
            rows.append(
                f"| {name} | — | {human(h_used)} | — | {pct:.1f}% of {human(h_total)} |"
            )

    note = "" if base else (
        "\n> Base build produced no parseable size output, so no delta is shown.\n"
    )

    md = (
        "<!-- size-report -->\n"
        "### Firmware size\n\n"
        "| Section | Base | This PR | Δ | Usage |\n"
        "|---|---:|---:|---:|---|\n"
        + "\n".join(rows)
        + "\n"
        + note
    )

    if a.out:
        a.out.write_text(md)
    print(md)
    return 0


if __name__ == "__main__":
    sys.exit(main())
