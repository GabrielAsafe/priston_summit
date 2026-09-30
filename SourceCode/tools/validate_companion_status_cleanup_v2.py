#!/usr/bin/env python3
from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
EXECUTOR = ROOT / "SrcServer/src/systems/gameplay/companions/CompanionSkillExecutor.cpp"
CONTRACT = ROOT / "Shared/gameplay/combat/v2/StatusEffectService.h"
ADAPTER = ROOT / "SrcServer/src/systems/gameplay/combat/v2/legacy/LegacyStatusEffectAdapter.cpp"


def fail(message: str) -> None:
    print(f"companion status cleanup v2: FAIL: {message}", file=sys.stderr)
    raise SystemExit(1)


def main() -> None:
    for path in (EXECUTOR, CONTRACT, ADAPTER):
        if not path.is_file():
            fail(f"missing {path.relative_to(ROOT)}")

    executor = EXECUTOR.read_text(encoding="utf-8", errors="replace")
    contract = CONTRACT.read_text(encoding="utf-8", errors="replace")
    adapter = ADAPTER.read_text(encoding="utf-8", errors="replace")

    if executor.count("ServerLegacyStatusEffectAdapter().ApplyElementalSlow16Hz(") != 3:
        fail("expected exactly three elemental slow consumers")
    if executor.count("ServerLegacyStatusEffectAdapter().ClearStatus(") != 2:
        fail("expected exactly two explicit status-clear consumers")

    for token in (
        "3, 230 - 10 * runtime.enchantWeaponPoint, resistance",
        "5, 230 - 10 * runtime.persistentEffectPoint, resistance",
        "Frost_Javelin_IceTime[p]",
        "240 - (240 * Frost_Javelin_SpeedSubPercent[p]) / 100, resistance",
        "combat::StatusEffectKind::Stun",
    ):
        if token not in executor:
            fail(f"elemental slow/clear parity token missing from consumer: {token}")

    direct_patterns = (
        r"->PlaySlowCount\s*=",
        r"->PlaySlowSpeed\s*=",
        r"->PlayDistortion\s*=",
        r"->PlayStunCount\s*=",
        r"->PlayPoison\s*\[[^\]]+\]\s*=",
        r"->PlayHolyMind\s*\[[^\]]+\]\s*=",
        r"->dwAssassinEyeTime\s*=",
        r"->AssassinEyeParam\s*=",
        r"->PlayHeavenlyLight\s*\[[^\]]+\]\s*=",
    )
    for pattern in direct_patterns:
        if re.search(pattern, executor):
            fail(f"direct status mutation remains in Companion executor: {pattern}")

    for token in (
        "enum class StatusEffectKind",
        "Stun = 0",
        "ApplyElementalSlow16Hz(EntityId target, int durationSeconds",
        "int moveSpeed, int signedIceResistance",
        "ClearStatus(EntityId target, StatusEffectKind kind)",
    ):
        if token not in contract:
            fail(f"GREEN status contract missing cleanup semantic: {token}")

    for token in (
        "std::max(-100, std::min(100, signedIceResistance))",
        "int durationTicks = durationSeconds * 16",
        "durationTicks -= (durationTicks * resistance) / 100",
        "resolved->PlaySlowCount = std::max(0, durationTicks)",
        "resolved->PlaySlowSpeed = moveSpeed",
        "resolved->PlayDistortion = 0",
        "case StatusEffectKind::Stun:",
        "resolved->PlayStunCount = 0",
    ):
        if token not in adapter:
            fail(f"legacy elemental slow/clear parity token missing: {token}")

    print("companion status cleanup v2: PASS (3 elemental slows + 2 stun clears routed; executor status writes=0)")


if __name__ == "__main__":
    main()
