#!/usr/bin/env python3
"""Ensure production Shared gameplay V2 translation units are in the server build."""

from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
BUILD_SCRIPT = ROOT / "build_server_mingw_i686_j8.sh"
PRODUCTION_DIRS = [
    ROOT / "Shared" / "gameplay" / "covenants" / "v2",
    ROOT / "Shared" / "gameplay" / "story" / "v2",
    ROOT / "Shared" / "gameplay" / "progression" / "v2",
    ROOT / "Shared" / "gameplay" / "reputation" / "v2",
]


def fail(message: str) -> None:
    print(f"server Shared gameplay manifest validation: FAIL: {message}", file=sys.stderr)
    raise SystemExit(1)


def main() -> None:
    if not BUILD_SCRIPT.is_file():
        fail(f"missing {BUILD_SCRIPT}")
    script = BUILD_SCRIPT.read_text(encoding="utf-8")

    expected = set()
    for directory in PRODUCTION_DIRS:
        if not directory.is_dir():
            fail(f"missing production directory {directory}")
        for path in directory.glob("*.cpp"):
            expected.add(path.relative_to(ROOT).as_posix())

    listed = set(
        match.group(0)
        for match in re.finditer(
            r"Shared/gameplay/(?:covenants|story|progression|reputation)/v2/[A-Za-z0-9_./-]+\.cpp",
            script,
        )
    )
    missing = sorted(expected - listed)
    if missing:
        fail(f"production units absent from build_server_mingw_i686_j8.sh: {missing}")

    stale = sorted(listed - expected)
    if stale:
        fail(f"manifest references missing Shared production units: {stale}")

    print(f"server Shared gameplay manifest validation: PASS ({len(expected)} units)")


if __name__ == "__main__":
    main()
