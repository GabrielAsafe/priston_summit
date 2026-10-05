#!/usr/bin/env python3
from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
CONTRACT = ROOT / "Shared/gameplay/progression/v2/PlayerExperienceService.h"
ADAPTER = ROOT / "SrcServer/src/systems/gameplay/progression/v2/legacy/LegacyPlayerExperienceAdapter.cpp"
KERNEL = ROOT / "SrcServer/src/core/server/legacy_runtime/PartySever.cpp"

DIRECT_CALL_RE = re.compile(r"->AddExp\s*\(")


def fail(message: str) -> None:
    print(f"player experience ownership v1: FAIL: {message}", file=sys.stderr)
    raise SystemExit(1)


def main() -> None:
    for path in (CONTRACT, ADAPTER, KERNEL):
        if not path.is_file():
            fail(f"missing {path.relative_to(ROOT)}")

    contract = CONTRACT.read_text(encoding="utf-8", errors="replace")
    for token in (
        "class IPlayerExperienceMutationPort",
        "class PlayerExperienceService",
        "PlayerExperienceGrantRequest",
        "PlayerExperienceSource::ProgressionReward",
    ):
        if token not in contract:
            fail(f"GREEN experience contract missing: {token}")
    for forbidden in ("rsPLAYINFO", "AddExp(", "smCharInfo", "lpsmSock"):
        if forbidden in contract:
            fail(f"GREEN experience contract leaks legacy symbol: {forbidden}")

    adapter = ADAPTER.read_text(encoding="utf-8", errors="replace")
    if "player->AddExp(" not in adapter:
        fail("legacy experience adapter no longer owns rsPLAYINFO::AddExp invocation")
    if "players_.Resolve(request.player)" not in adapter:
        fail("legacy experience adapter does not resolve PlayerId through registry")

    kernel = KERNEL.read_text(encoding="utf-8", errors="replace")
    if "void rsPLAYINFO::AddExp(" not in kernel:
        fail("legacy AddExp kernel definition missing")

    unauthorized = []
    roots = [ROOT / "SrcServer/src", ROOT / "Shared/server/legacy_runtime"]
    for base in roots:
        for path in base.rglob("*"):
            if not path.is_file() or path.suffix not in {".cpp", ".h", ".hpp", ".inl"}:
                continue
            rel = path.relative_to(ROOT).as_posix()
            if "/legacy_oracle/" in f"/{rel}/" or ".before" in path.name or "_legacy_client" in path.name:
                continue
            if path == ADAPTER:
                continue
            text = path.read_text(encoding="utf-8", errors="replace")
            for lineno, line in enumerate(text.splitlines(), 1):
                if DIRECT_CALL_RE.search(line):
                    unauthorized.append(f"{rel}:{lineno}: {line.strip()}")

    if unauthorized:
        fail("direct rsPLAYINFO::AddExp callers remain outside adapter:\n" + "\n".join(unauthorized))

    print("player experience ownership v1: PASS (kernel private to LegacyPlayerExperienceAdapter; unauthorized=0)")


if __name__ == "__main__":
    main()
