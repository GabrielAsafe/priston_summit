#!/usr/bin/env python3
from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
EXECUTOR = ROOT / "SrcServer/src/systems/gameplay/companions/CompanionSkillExecutor.cpp"
CONTRACT = ROOT / "Shared/gameplay/inventory/v2/PotionService.h"
ADAPTER = ROOT / "SrcServer/src/systems/gameplay/inventory/v2/legacy/LegacyPotionAdapter.cpp"
RECORD = ROOT / "Shared/persistence/character_record/record_legacy_server.inl"


def fail(message: str) -> None:
    print(f"companion potion boundary v1: FAIL: {message}", file=sys.stderr)
    raise SystemExit(1)


def main() -> None:
    for path in (EXECUTOR, CONTRACT, ADAPTER, RECORD):
        if not path.is_file():
            fail(f"missing {path.relative_to(ROOT)}")

    executor = EXECUTOR.read_text(encoding="utf-8", errors="replace")
    contract = CONTRACT.read_text(encoding="utf-8", errors="replace")
    adapter = ADAPTER.read_text(encoding="utf-8", errors="replace")
    record = RECORD.read_text(encoding="utf-8", errors="replace")

    for token in (
        "ServerLegacyPotionAdapter().ConsumeAndApply(",
        "inventory::PotionResource::Life",
        "inventory::PotionResource::Mana",
        "inventory::PotionResource::Stamina",
        "CompanionPotionRetryDelay(result.legacyStatus)",
        "runtime.nextPotionUseTime = now + 1500U",
        "CompanionPotionDebugLog(",
        'PotionThreshold("PotionLifePercent", 45)',
        'PotionThreshold("PotionManaPercent", 25)',
        'PotionThreshold("PotionStaminaPercent", 25)',
    ):
        if token not in executor:
            fail(f"potion scheduling/telemetry token missing: {token}")

    for forbidden in (
        "rsConsumeOfflineCharacterPotion(",
        '#include "systems/persistence/character_record/record.h"',
    ):
        if forbidden in executor:
            fail(f"legacy potion persistence still crosses Companion boundary: {forbidden}")

    potion_section = executor[executor.find("static bool ConsumeAndApplyPotion("):executor.find("static bool CanCast(")]
    if re.search(r"caster->smCharInfo\.(?:Life|Mana|Stamina)\[0\]\s*=", potion_section):
        fail("potion restore still mutates Companion resources directly")

    for token in (
        "enum class PotionResource",
        "struct PotionUseRequest",
        "struct PotionUseResult",
        "class IPotionService",
        "PotionUseResult ConsumeAndApply(const PotionUseRequest& request)",
        "legacyStatus",
        "remainingCount",
        "restoreAmount",
        "itemCode",
        "beforeValue",
        "afterValue",
    ):
        if token not in contract:
            fail(f"GREEN potion contract missing: {token}")
    for forbidden in ("smCHAR", "rsPLAYINFO", "DWORD", "short ", "record.h", "onserver.h", "character.h"):
        if forbidden in contract:
            fail(f"GREEN potion contract contains legacy symbol/include: {forbidden}")

    for token in (
        "rsConsumeOfflineCharacterPotion(request.accountName.c_str(), request.characterName.c_str(), type",
        "result.legacyStatus = -11",
        "target->smCharInfo.Life[0] = static_cast<short>(std::min(",
        "target->smCharInfo.Mana[0] = static_cast<short>(std::min(",
        "target->smCharInfo.Stamina[0] = static_cast<short>(std::min(",
        "result.beforeValue",
        "result.afterValue",
    ):
        if token not in adapter:
            fail(f"legacy potion parity token missing from adapter: {token}")

    for token in (
        "recordItem.sItemInfo.PotionCount",
        "--recordItem.sItemInfo.PotionCount",
        "ReformItem(&recordItem.sItemInfo)",
        "MoveFileExA(",
    ):
        if token not in record:
            fail(f"authoritative DAT potion behavior missing: {token}")

    print("companion potion boundary v1: PASS (consume + restore routed; threshold/cooldown/telemetry preserved)")


if __name__ == "__main__":
    main()
