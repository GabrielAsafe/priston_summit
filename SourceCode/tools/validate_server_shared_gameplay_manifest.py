#!/usr/bin/env python3
"""Ensure every production Shared gameplay V2 translation unit is in the server build."""

from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
BUILD_SCRIPT = ROOT / "build_server_mingw_i686_j8.sh"
SHARED_GAMEPLAY = ROOT / "Shared" / "gameplay"


def fail(message: str) -> None:
    print(f"server Shared gameplay manifest validation: FAIL: {message}", file=sys.stderr)
    raise SystemExit(1)


def is_production_v2_cpp(path: Path) -> bool:
    relative = path.relative_to(SHARED_GAMEPLAY)
    return path.suffix == ".cpp" and "v2" in relative.parts and "tests" not in relative.parts


def main() -> None:
    if not BUILD_SCRIPT.is_file():
        fail(f"missing {BUILD_SCRIPT}")
    if not SHARED_GAMEPLAY.is_dir():
        fail(f"missing {SHARED_GAMEPLAY}")

    script = BUILD_SCRIPT.read_text(encoding="utf-8")
    expected = {
        path.relative_to(ROOT).as_posix()
        for path in SHARED_GAMEPLAY.rglob("*.cpp")
        if is_production_v2_cpp(path)
    }

    listed = set(
        match.group(0)
        for match in re.finditer(
            r"Shared/gameplay/[A-Za-z0-9_./-]+/v2/[A-Za-z0-9_./-]+\.cpp",
            script,
        )
    )

    missing = sorted(expected - listed)
    if missing:
        fail(f"production V2 units absent from build_server_mingw_i686_j8.sh: {missing}")

    stale = sorted(listed - expected)
    if stale:
        fail(f"manifest references missing/non-production Shared V2 units: {stale}")

    print(f"server Shared gameplay manifest validation: PASS ({len(expected)} production V2 units)")


if __name__ == "__main__":
    main()
