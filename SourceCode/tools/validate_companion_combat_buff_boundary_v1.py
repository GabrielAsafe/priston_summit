#!/usr/bin/env python3
from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
EXECUTOR = ROOT / "SrcServer/src/systems/gameplay/companions/CompanionSkillExecutor.cpp"
CONTRACT = ROOT / "Shared/gameplay/combat/v2/CombatBuffService.h"
ADAPTER = ROOT / "SrcServer/src/systems/gameplay/combat/v2/legacy/LegacyCombatBuffAdapter.cpp"


def fail(message: str) -> None:
    print(f"companion combat buff boundary v1: FAIL: {message}", file=sys.stderr)
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

    buff_code = section(executor, "static combat::CombatBuffDelta ToCombatBuffDelta", "static bool TryCompanionSummonSkill")
    for token in (
        "ServerLegacyCombatBuffAdapter().Apply(",
        "ServerLegacyCombatBuffAdapter().Remove(",
        "ToCombatBuffDelta(modifier)",
    ):
        if token not in buff_code:
            fail(f"missing routed combat-buff operation: {token}")

    # The formula layer may read smCharInfo to calculate percentage-based deltas,
    # but the timed-buff path may no longer write the sheet directly.
    direct_write = re.compile(
        r"caster->smCharInfo\.(?:Attack_Damage\[[01]\]|Attack_Rating|Defence|Attack_Speed|"
        r"Shooting_Range|Absorption|Chance_Block|Life_Regen|Mana_Regen|Resistance\[[^\]]+\]|Life\[[01]\])\s*(?:\+=|-=|=)"
    )
    match = direct_write.search(buff_code)
    if match:
        fail(f"timed combat buff sheet mutation still direct: {match.group(0)}")

    # Representative legacy formulas stay in the executor; only authority to
    # mutate the sheet moved behind the boundary.
    for token in (
        "Maximize_Damage[p]",
        "Automation_Damage[p]",
        "Berserker_SubAbsorb[p]",
        "Virtual_Life_Percent[p]",
        "Metal_Armor_Defense[p]",
        "Force_Of_Nature_AddDamage[p]",
        "Zenith_Element[p]",
        "Hardening_Shield_AttackRateReduction[p]",
        "Pike_T5_Death_AttackRateBoost[p]",
        "Magician_T5_WizardTrance_AttackRateReduction[p]",
    ):
        if token not in buff_code:
            fail(f"legacy buff formula lost during extraction: {token}")

    for token in (
        "struct CombatBuffDelta",
        "class ICombatBuffService",
        "CombatBuffResult Apply(EntityId target, const CombatBuffDelta& delta)",
        "CombatBuffResult Remove(EntityId target, const CombatBuffDelta& delta)",
        "int resistance[8]",
    ):
        if token not in contract:
            fail(f"GREEN combat-buff contract missing: {token}")
    for forbidden in ("smCHAR", "rsPLAYINFO", "DWORD", "short ", "windows.h", "character.h", "onserver.h"):
        if forbidden in contract:
            fail(f"GREEN combat-buff contract contains legacy symbol/include: {forbidden}")

    for token in (
        "resolved->smCharInfo.Attack_Damage[0] += delta.attackMin",
        "resolved->smCharInfo.Attack_Rating += delta.attackRating",
        "resolved->smCharInfo.Resistance[r] = static_cast<short>(",
        "resolved->smCharInfo.Life[1] = static_cast<short>(",
        "resolved->smCharInfo.Attack_Damage[0] -= delta.attackMin",
        "resolved->smCharInfo.Life[0] > resolved->smCharInfo.Life[1]",
        "resolved->smCharInfo.Attack_Speed < 1",
        "resolved->smCharInfo.Life_Regen < 0.0f",
    ):
        if token not in adapter:
            fail(f"legacy combat-buff parity token missing from adapter: {token}")

    print("companion combat buff boundary v1: PASS (timed sheet apply/remove routed; formulas remain legacy)")


if __name__ == "__main__":
    main()
