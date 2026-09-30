#!/usr/bin/env python3
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
EXECUTOR = ROOT / "SrcServer/src/systems/gameplay/companions/CompanionSkillExecutor.cpp"
CONTRACT = ROOT / "Shared/gameplay/combat/v2/HealingService.h"
ADAPTER = ROOT / "SrcServer/src/systems/gameplay/combat/v2/legacy/LegacyHealingAdapter.cpp"


def fail(message: str) -> None:
    print(f"companion healing boundary v1: FAIL: {message}", file=sys.stderr)
    raise SystemExit(1)


def main() -> None:
    for path in (EXECUTOR, CONTRACT, ADAPTER):
        if not path.is_file():
            fail(f"missing {path.relative_to(ROOT)}")

    executor = EXECUTOR.read_text(encoding="utf-8", errors="replace")
    if "rsPlayHealing(" in executor:
        fail("legacy rsPlayHealing call still present in CompanionSkillExecutor")
    if "OpCode::OPCODE_HEALING" in executor:
        fail("legacy healing packet construction still present in CompanionSkillExecutor")

    routed = "ServerLegacyHealingAdapter().Heal("
    if executor.count(routed) < 4:
        fail("Healing/Grand Healing consumers are not fully routed through healing service")
    if "PlayerId(static_cast<std::uint32_t>(owner->dwObjectSerial)), heal" not in executor:
        fail("Grand Healing owner path is not routed as PlayerId")
    if "PlayerId(static_cast<std::uint32_t>(targetSerial)), heal" not in executor:
        fail("single Healing owner path is not routed as PlayerId")
    if "EntityId(static_cast<std::uint32_t>(candidate->dwObjectSerial)), heal" not in executor:
        fail("Grand Healing companion path is not routed as EntityId")

    contract = CONTRACT.read_text(encoding="utf-8", errors="replace")
    for token in ("class IHealingService", "HealingResult Heal(EntityId source, EntityId target, int amount)",
                  "HealingResult Heal(EntityId source, PlayerId target, int amount)"):
        if token not in contract:
            fail(f"GREEN healing contract missing: {token}")
    for forbidden_symbol in ("rsPLAYINFO", "smCHAR", "smTRANS_COMMAND", "windows.h", "onserver.h", "character.h"):
        if forbidden_symbol in contract:
            fail(f"GREEN healing contract contains legacy symbol/include: {forbidden_symbol}")

    adapter = ADAPTER.read_text(encoding="utf-8", errors="replace")
    for token in ("OpCode::OPCODE_HEALING", "rsPlayHealing(&command)",
                  "player->smCharInfo.Life[0] = static_cast<short>(after)"):
        if token not in adapter:
            fail(f"legacy healing parity token missing from adapter: {token}")

    print("companion healing boundary v1: PASS")


if __name__ == "__main__":
    main()
