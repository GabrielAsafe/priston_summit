#!/usr/bin/env python3
from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
EXECUTOR = ROOT / "SrcServer/src/systems/gameplay/companions/CompanionSkillExecutor.cpp"
CONTRACT = ROOT / "Shared/gameplay/combat/v2/CombatBuffService.h"
ADAPTER = ROOT / "SrcServer/src/systems/gameplay/combat/v2/legacy/LegacyCombatBuffAdapter.cpp"


def fail(message: str) -> None:
    print(f"companion passive stat boundary v2: FAIL: {message}", file=sys.stderr)
    raise SystemExit(1)


def section(text: str, start: str, end: str) -> str:
    a = text.find(start)
    if a < 0:
        fail(f"missing section start: {start}")
    b = text.find(end, a)
    if b < 0:
        fail(f"missing section end: {end}")
    return text[a:b]


def main() -> None:
    for path in (EXECUTOR, CONTRACT, ADAPTER):
        if not path.is_file():
            fail(f"missing {path.relative_to(ROOT)}")

    executor = EXECUTOR.read_text(encoding="utf-8", errors="replace")
    contract = CONTRACT.read_text(encoding="utf-8", errors="replace")
    adapter = ADAPTER.read_text(encoding="utf-8", errors="replace")
    passive = section(executor, "void ApplyCompanionPassiveSkills", "bool IsCompanionSkillLocked")

    for token in (
        "ApplyCompanionPassiveDelta(caster, delta)",
        "delta.resistance[sITEMINFO_POISON]",
        "delta.attackMin =",
        "delta.chanceBlock = W_D_Mastery_Block[p]",
        "delta.attackRating =",
        "delta.criticalHit = Critical_Mastery_Critical[p]",
        "delta.maxStamina = newMax - oldMax",
        "delta.manaRegen = Meditation_Regen[p]",
        "delta.maxMana = newMax - oldMax",
        "delta.maxLife = newMax - oldMax",
        "delta.lifeRegen = (float)Fighter_T5_Survival_Regen[p]",
        "delta.moveSpeed =",
        "runtime.passiveEvadePercent",
    ):
        if token not in passive:
            fail(f"missing passive routing/parity token: {token}")

    # Reads are allowed for percentage formulas and full-resource preservation.
    # Direct writes to authoritative character-sheet passive stats are not.
    write_patterns = (
        r"caster->smCharInfo\.Resistance\[[^\]]+\]\s*=",
        r"caster->smCharInfo\.Attack_Damage\[[01]\]\s*\+=",
        r"caster->smCharInfo\.Chance_Block\s*\+=",
        r"caster->smCharInfo\.Attack_Rating\s*\+=",
        r"caster->smCharInfo\.Critical_Hit\s*\+=",
        r"caster->smCharInfo\.Stamina\[1\]\s*=",
        r"caster->smCharInfo\.Mana_Regen\s*\+=",
        r"caster->smCharInfo\.Mana\[1\]\s*=",
        r"caster->smCharInfo\.Life\[1\]\s*=",
        r"caster->smCharInfo\.Life_Regen\s*\+=",
        r"caster->smCharInfo\.Move_Speed\s*\+=",
    )
    for pattern in write_patterns:
        match = re.search(pattern, passive)
        if match:
            fail(f"direct passive character-sheet mutation remains: {match.group(0)}")

    for token in (
        "int criticalHit;",
        "int moveSpeed;",
        "int maxMana;",
        "int maxStamina;",
    ):
        if token not in contract:
            fail(f"combat buff contract missing passive stat field: {token}")

    for token in (
        "resolved->smCharInfo.Critical_Hit += delta.criticalHit",
        "resolved->smCharInfo.Move_Speed += delta.moveSpeed",
        "resolved->smCharInfo.Mana[1] = static_cast<short>",
        "resolved->smCharInfo.Stamina[1] = static_cast<short>",
        "resolved->smCharInfo.Critical_Hit -= delta.criticalHit",
        "resolved->smCharInfo.Move_Speed -= delta.moveSpeed",
        "resolved->smCharInfo.Mana[0] > resolved->smCharInfo.Mana[1]",
        "resolved->smCharInfo.Stamina[0] > resolved->smCharInfo.Stamina[1]",
    ):
        if token not in adapter:
            fail(f"legacy combat-buff adapter missing passive parity token: {token}")

    print("companion passive stat boundary v2: PASS (character-sheet passive writes routed through CombatBuffService)")


if __name__ == "__main__":
    main()
