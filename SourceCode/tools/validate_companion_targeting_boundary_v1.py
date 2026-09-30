#!/usr/bin/env python3
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
EXECUTOR = ROOT / "SrcServer/src/systems/gameplay/companions/CompanionSkillExecutor.cpp"
CONTRACT = ROOT / "Shared/gameplay/combat/v2/TargetingService.h"
ADAPTER = ROOT / "SrcServer/src/systems/gameplay/combat/v2/legacy/LegacyTargetingAdapter.cpp"


def fail(message: str) -> None:
    print(f"companion targeting boundary v1: FAIL: {message}", file=sys.stderr)
    raise SystemExit(1)


def main() -> None:
    for path in (EXECUTOR, CONTRACT, ADAPTER):
        if not path.is_file():
            fail(f"missing {path.relative_to(ROOT)}")

    executor = EXECUTOR.read_text(encoding="utf-8", errors="replace")
    required = [
        "ServerLegacyTargetingAdapter().ClearAcquiredTarget(",
        "ServerLegacyTargetingAdapter().ReleaseTarget(",
        "ServerLegacyTargetingAdapter().ForceTarget(",
        "TargetReleaseMode::AcquiredAndForced",
    ]
    for token in required:
        if token not in executor:
            fail(f"missing routed targeting operation: {token}")

    forbidden = [
        "caster->lpTargetChar = NULL",
        "caster->lpTargetPlayInfo = NULL",
        "monster->lpTargetChar = NULL",
        "attacker->lpTargetChar = NULL",
        "target->lpTargetChar = caster",
        "target->lpCompulsionChar = caster",
        "deadTarget->lpTargetChar = NULL",
        "deadTarget->lpTargetPlayInfo = NULL",
    ]
    for token in forbidden:
        if token in executor:
            fail(f"targeting mutation still direct in extracted path: {token}")

    contract = CONTRACT.read_text(encoding="utf-8", errors="replace")
    for token in ("class ITargetingService", "ClearAcquiredTarget", "ReleaseTarget", "ForceTarget"):
        if token not in contract:
            fail(f"GREEN targeting contract missing: {token}")
    for forbidden_symbol in ("rsPLAYINFO", "smCHAR", "windows.h", "onserver.h", "character.h"):
        if forbidden_symbol in contract:
            fail(f"GREEN targeting contract contains legacy symbol/include: {forbidden_symbol}")

    adapter = ADAPTER.read_text(encoding="utf-8", errors="replace")
    for token in (
        "resolved->lpTargetChar = 0",
        "resolved->lpTargetPlayInfo = 0",
        "resolvedActor->lpCompulsionChar = 0",
        "resolvedActor->dwCompulsionTime = 0",
        "resolvedActor->lpTargetChar = resolvedTarget",
        "resolvedActor->lpCompulsionChar = resolvedTarget",
        "resolvedActor->SetNextTarget",
    ):
        if token not in adapter:
            fail(f"targeting parity token missing from adapter: {token}")

    print("companion targeting boundary v1: PASS")


if __name__ == "__main__":
    main()
