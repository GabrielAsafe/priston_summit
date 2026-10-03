#!/usr/bin/env python3
from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
EXECUTOR = ROOT / "SrcServer/src/systems/gameplay/companions/CompanionSkillExecutor.cpp"
CONTRACT = ROOT / "Shared/gameplay/combat/v2/TargetingService.h"
ADAPTER = ROOT / "SrcServer/src/systems/gameplay/combat/v2/legacy/LegacyTargetingAdapter.cpp"


def fail(msg: str) -> None:
    print(f"companion actor mutation boundary v2: FAIL: {msg}", file=sys.stderr)
    raise SystemExit(1)


def main() -> None:
    executor = EXECUTOR.read_text(encoding="utf-8", errors="replace")
    contract = CONTRACT.read_text(encoding="utf-8", errors="replace")
    adapter = ADAPTER.read_text(encoding="utf-8", errors="replace")

    if "ServerLegacyTargetingAdapter().ClearPlayerTarget(" not in executor:
        fail("push target release is not routed through targeting service")
    if re.search(r"target->lpTargetPlayInfo\s*=", executor):
        fail("direct lpTargetPlayInfo mutation remains in CompanionSkillExecutor")

    # The executor may read actor state to calculate formulas/eligibility, but it
    # must not own direct actor/character mutations anymore.
    direct = re.compile(
        r"(?:caster|target|candidate|monster|summon|deadTarget)->(?:smCharInfo\.[A-Za-z_0-9\[\]]+|"
        r"MotionLoop(?:Speed)?|lpTargetPlayInfo)\s*(?:\+=|-=|\*=|/=|(?<![<>!=])=(?!=))"
    )
    match = direct.search(executor)
    if match:
        fail(f"direct actor mutation remains: {match.group(0)}")

    if "ClearPlayerTarget(EntityId actor)" not in contract:
        fail("GREEN targeting contract missing ClearPlayerTarget")
    if "LegacyTargetingAdapter::ClearPlayerTarget(EntityId actor)" not in adapter:
        fail("legacy targeting adapter missing ClearPlayerTarget implementation")
    if "resolved->lpTargetPlayInfo = 0" not in adapter:
        fail("legacy adapter does not preserve player-target clear semantics")

    print("companion actor mutation boundary v2: PASS (no direct character/animation/player-target writes in executor)")


if __name__ == "__main__":
    main()
