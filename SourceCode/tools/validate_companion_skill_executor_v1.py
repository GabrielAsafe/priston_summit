#!/usr/bin/env python3
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
executor = ROOT / "SrcServer/src/systems/gameplay/companions/CompanionSkillExecutor.cpp"
header = ROOT / "SrcServer/src/systems/gameplay/companions/CompanionSkillExecutor.h"
system = ROOT / "SrcServer/src/systems/gameplay/companions/CompanionSystem.cpp"
record = ROOT / "Shared/persistence/character_record/record_legacy_server.inl"
record_h = ROOT / "SrcServer/src/systems/persistence/character_record/record.h"
config = ROOT / "Configs/Companion.ini"
profile = ROOT / "Shared/gameplay/companions/v2/CompanionSkillProfile.h"

errors = []

def need(path: Path, needle: str, label: str):
    data = path.read_text(encoding="latin1")
    if needle not in data:
        errors.append(f"{path.relative_to(ROOT)}: missing {label}: {needle}")

def forbid(path: Path, needle: str, label: str):
    data = path.read_text(encoding="latin1")
    if needle in data:
        errors.append(f"{path.relative_to(ROOT)}: forbidden {label}: {needle}")

for path in (executor, header, system, record, record_h, profile):
    if not path.exists():
        errors.append(f"missing file: {path.relative_to(ROOT)}")

if not errors:
    # Exact default offensive-skill parity moved out of the executor into
    # canonical Companion content. Keep the eight historical mappings stable.
    for token in (
        "{1, 0x02010004u, false}",  # Fighter -> Impact
        "{2, 0x01020001u, false}",  # Mechanician -> Great Smash
        "{3, 0x04010003u, false}",  # Archer -> Wind Arrow
        "{4, 0x03010004u, false}",  # Pikeman -> Jumping Crash
        "{5, 0x06020002u, false}",  # Atalanta -> Twist Javelin
        "{6, 0x05010001u, false}",  # Knight -> Sword Blast
        "{7, 0x08010002u, false}",  # Magician -> Fire Bolt
        "{8, 0x07010002u, true}",   # Priestess -> Holy Bolt + heal role
    ):
        need(profile, token, "canonical companion class skill mapping")
    need(executor, "DefaultDamageSkillForClass", "content-owned damage skill mapping")
    need(executor, "HasCompanionHealingRole", "content-owned healing role")
    need(executor, "SKILL_HEALING", "healing skill execution")
    forbid(executor, "JOBCODE_", "concrete class switch")
    forbid(system, "JOBCODE_", "concrete companion class branch")

    # Skill points are normalized into a logical catalog-owned slot space.
    # The legacy circular 16-byte DAT layout is translated by CompanionSystem;
    # the executor itself must never depend on (slot & 15).
    need(header, "COMPANION_MAX_SKILL_SLOTS = 64", "logical skill capacity")
    need(executor, "FindCatalogSkillForSlot", "catalog-owned slot resolver")
    need(executor, "points[requestedSlot - 1]", "logical skill-point lookup")
    forbid(executor, "points[useSlot & 15]", "legacy circular slot lookup")

    # Requirements/resources.
    need(executor, "info->RequireLevel", "level requirement")
    need(executor, "WeaponAllowed(info, rightCode, leftCode)", "weapon requirement")
    need(executor, "info->UseStamina[0] +", "legacy stamina formula")
    need(executor, "info->UseStamina[1] * (point - 1)", "legacy stamina scaling")
    need(executor, "InspectCompanionSkillResources(caster)", "resource boundary inspection")
    need(executor, "resources.mana < mana", "MP gate")
    need(executor, "resources.stamina < stamina", "stamina gate")
    need(executor, "SpendCompanionSkillResources(caster, mana, stamina)", "resource boundary consumption")
    need(executor, "runtime.nextDamageSkillTime", "damage cooldown")
    need(executor, "runtime.nextHealSkillTime", "heal cooldown")
    need(executor, "InRange(caster, target", "damage range")

    # Never tunnel companion skills through player-global active skill code.
    forbid(executor, "PlaySkillAttack(", "player-global skill executor")
    forbid(executor, "pUseSkill", "player-global active skill state")
    forbid(executor, "lpCurPlayer", "player-global character state")

    # Actual DAT potion stack is authoritative and rewritten atomically.
    need(record_h, "rsConsumeOfflineCharacterPotion", "potion persistence API")
    need(record, "DecodeCompress((BYTE *)cursor, (BYTE *)&recordItem, sizeof(recordItem))", "DAT item decode")
    need(record, "recordItem.sItemInfo.PotionCount", "real potion stack count")
    need(record, "lpRestoreAmount", "potion restore returned from DAT stack")
    need(record, "recordItem.sItemInfo.Life[0]", "HP potion restore from persisted stack")
    need(record, "recordItem.sItemInfo.Mana[0]", "MP potion restore from persisted stack")
    need(record, "recordItem.sItemInfo.Stamina[0]", "RES potion restore from persisted stack")
    need(executor, "ITEM_CHECK", "potion attempt telemetry")
    need(record, "--recordItem.sItemInfo.PotionCount", "real stack decrement")
    need(record, "ReformItem(&recordItem.sItemInfo)", "potion item checksum reform")
    need(record, "EecodeCompress(", "DAT item recompression")
    need(record, "recordData.ItemCount = newItemCount", "item count rebuild")
    need(record, "recordData.ItemSubStart = newItemSubStart", "sub inventory boundary rebuild")
    need(record, "MoveFileExA(", "atomic DAT replacement")
    need(record, "MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH", "durable DAT replacement")

    # Auto use remains configurable when Configs/Companion.ini is present, but
    # source-only snapshots may intentionally omit runtime config files. The
    # executor therefore owns safe defaults and the INI only overrides them.
    need(executor, 'SummitCompanionConfigInt("AutoUsePotions", 1)', "auto potion default")
    need(executor, 'TryGameplayConfigInt("Configs/Companion.ini", "Companion", key, value)', "compiled gameplay config bridge")
    need(executor, 'GetPrivateProfileIntA("Companion", key, fallback, ".//Configs//Companion.ini")', "legacy INI fallback")
    need(executor, 'PotionThreshold("PotionLifePercent", 45)', "life potion default")
    need(executor, 'PotionThreshold("PotionManaPercent", 25)', "mana potion default")
    need(executor, 'PotionThreshold("PotionStaminaPercent", 25)', "stamina potion trigger")
    # Companion.ini is an optional runtime override. Do not make a source
    # validator fail merely because an existing INI omits keys whose safe
    # defaults are owned by the executor.

    # Companion integration preserves basic attack fallback.
    need(system, "LoadCompanionLogicalSkillPoints(slot.skillRuntime", "saved logical skill-point load")
    need(system, "TryUseCompanionConsumable(", "item executor integration")
    need(system, "TryExecuteCompanionHeal(", "heal integration")
    system_text = system.read_text(encoding="latin1")
    if "TryExecuteCompanionDamageSkill(" not in system_text and "TryExecuteCompanionDamageLoadout(" not in system_text:
        errors.append(f"{system.relative_to(ROOT)}: missing damage skill integration")
    need(system, "EngageTarget(character, target, preferredRange", "basic attack fallback")
    need(system, "IsCompanionSkillLocked(", "basic-hit suppression during cast")

if errors:
    print("Companion items/skills V1: FAIL")
    for error in errors:
        print(" -", error)
    sys.exit(1)

print("Companion items/skills V1: PASS")
