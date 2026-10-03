#!/usr/bin/env python3
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
EXECUTOR = ROOT / "SrcServer/src/systems/gameplay/companions/CompanionSkillExecutor.cpp"
CONTRACT = ROOT / "Shared/gameplay/combat/v2/LifeCommands.h"
ADAPTER = ROOT / "SrcServer/src/systems/gameplay/combat/v2/legacy/LegacyLifeAdapter.cpp"


def fail(message: str) -> None:
    print(f"companion life boundary v1: FAIL: {message}", file=sys.stderr)
    raise SystemExit(1)


def main() -> None:
    for path in (EXECUTOR, CONTRACT, ADAPTER):
        if not path.is_file():
            fail(f"missing {path.relative_to(ROOT)}")

    executor = EXECUTOR.read_text(encoding="utf-8", errors="replace")
    required = [
        "ServerLegacyLifeAdapter().Restore(EntityId(static_cast<std::uint32_t>(caster->dwObjectSerial)), rawDamage)",
        "ServerLegacyLifeAdapter().ConsumeNonLethal",
        "ServerLegacyLifeAdapter().ConsumeLethal",
        "ServerLegacyLifeAdapter().Revive",
        "const combat::LifeCommandResult restored = combat::legacy::ServerLegacyLifeAdapter().Restore",
        "EntityId(static_cast<std::uint32_t>(target->dwObjectSerial)), heal",
        "EntityId(static_cast<std::uint32_t>(caster->dwObjectSerial)), totalLife",
    ]
    for token in required:
        if token not in executor:
            fail(f"missing routed life command: {token}")

    forbidden = [
        "caster->smCharInfo.Life[0] = (short)std::min((int)caster->smCharInfo.Life[1], (int)caster->smCharInfo.Life[0] + rawDamage)",
        "caster->smCharInfo.Life[0] = (short)((int)caster->smCharInfo.Life[0] - lifeCost)",
        "caster->smCharInfo.Life[0] = (short)std::min((int)caster->smCharInfo.Life[1], (int)caster->smCharInfo.Life[0] + totalLife)",
        "caster->smCharInfo.Life[0] = (short)std::min((int)caster->smCharInfo.Life[1], before + add)",
        "target->smCharInfo.Life[0] = (short)std::min((int)target->smCharInfo.Life[1]",
        "target->smCharInfo.Life[0] = (short)std::max(0, (int)target->smCharInfo.Life[0] - life)",
        "reviveTarget->smCharInfo.Life[0] = (short)std::max(1,",
        "deadTarget->smCharInfo.Life[0] = (short)std::max(1,",
        "reviveTarget->SetMotionFromCode(CHRMOTION_STATE_STAND)",
    ]
    for token in forbidden:
        if token in executor:
            fail(f"generic life mutation still direct in executor: {token}")

    contract = CONTRACT.read_text(encoding="utf-8", errors="replace")
    for forbidden_symbol in ("rsPLAYINFO", "smCHAR", "windows.h", "onserver.h", "character.h"):
        if forbidden_symbol in contract:
            fail(f"GREEN life contract contains legacy symbol/include: {forbidden_symbol}")

    adapter = ADAPTER.read_text(encoding="utf-8", errors="replace")
    for token in ("std::min(maximum, before + amount)", "before - amount <= 0",
                  "std::max(0, before - amount)", "CHRMOTION_STATE_STAND"):
        if token not in adapter:
            fail(f"life parity token missing from adapter: {token}")

    print("companion life boundary v1: PASS")


if __name__ == "__main__":
    main()
