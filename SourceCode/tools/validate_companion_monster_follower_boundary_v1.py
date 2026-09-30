#!/usr/bin/env python3
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
EXECUTOR = ROOT / "SrcServer/src/systems/gameplay/companions/CompanionSkillExecutor.cpp"
CONTRACT = ROOT / "Shared/gameplay/monsters/v2/MonsterCommands.h"
ADAPTER = ROOT / "SrcServer/src/systems/gameplay/monsters/v2/legacy/LegacyMonsterAdapter.cpp"


def fail(message: str) -> None:
    print(f"companion monster follower boundary v1: FAIL: {message}", file=sys.stderr)
    raise SystemExit(1)


def main() -> None:
    for path in (EXECUTOR, CONTRACT, ADAPTER):
        if not path.is_file():
            fail(f"missing {path.relative_to(ROOT)}")

    executor = EXECUTOR.read_text(encoding="utf-8", errors="replace")
    if executor.count("ServerLegacyMonsterAdapter().ConvertToFollower(") < 2:
        fail("Holy Incantation conversion paths are not routed through IMonsterCommands")

    forbidden = [
        "lpMasterPlayInfo = owner",
        "smMonsterInfo.dwAutoCharCode = 0",
        "PlayHolyIncantation[0] =",
        "PlayHolyIncantation[1] =",
        "smCharInfo.Next_Exp = owner->dwObjectSerial",
        "smCharInfo.Brood = smCHAR_MONSTER_USER",
    ]
    for token in forbidden:
        if token in executor:
            fail(f"direct follower mutation still present in CompanionSkillExecutor: {token}")

    contract = CONTRACT.read_text(encoding="utf-8", errors="replace")
    for token in ("class IMonsterCommands", "FollowerConversionRequest", "ConvertToFollower"):
        if token not in contract:
            fail(f"GREEN monster contract missing: {token}")
    for forbidden_symbol in ("rsPLAYINFO", "smCHAR", "windows.h", "onserver.h", "character.h"):
        if forbidden_symbol in contract:
            fail(f"GREEN monster contract contains legacy symbol/include: {forbidden_symbol}")

    adapter = ADAPTER.read_text(encoding="utf-8", errors="replace")
    required = [
        "target->lpMasterPlayInfo = owner",
        "target->smMonsterInfo.dwAutoCharCode = 0",
        "target->smCharInfo.bUpdateInfo[0]++",
        "target->PlayHolyIncantation[0]",
        "target->PlayHolyIncantation[1]",
        "target->smCharInfo.Life[0] = target->smCharInfo.Life[1]",
        "target->smCharInfo.Next_Exp = owner->dwObjectSerial",
        "target->smCharInfo.Brood = smCHAR_MONSTER_USER",
        "target->lpTargetChar = 0",
        "target->lpTargetPlayInfo = 0",
    ]
    for token in required:
        if token not in adapter:
            fail(f"legacy follower parity token missing from adapter: {token}")

    print("companion monster follower boundary v1: PASS")


if __name__ == "__main__":
    main()
