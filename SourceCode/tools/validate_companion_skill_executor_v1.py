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

errors = []

def need(path: Path, needle: str, label: str):
    data = path.read_text(encoding="latin1")
    if needle not in data:
        errors.append(f"{path.relative_to(ROOT)}: missing {label}: {needle}")

def forbid(path: Path, needle: str, label: str):
    data = path.read_text(encoding="latin1")
    if needle in data:
        errors.append(f"{path.relative_to(ROOT)}: forbidden {label}: {needle}")

for path in (executor, header, system, record, record_h):
    if not path.exists():
        errors.append(f"missing file: {path.relative_to(ROOT)}")

if not errors:
    # One real learned offensive skill per legacy class + Priestess healing.
    for token in (
        "JOBCODE_FIGHTER: return SKILL_IMPACT",
        "JOBCODE_MECHANICIAN: return SKILL_GREAT_SMASH",
        "JOBCODE_ARCHER: return SKILL_WIND_ARROW",
        "JOBCODE_PIKEMAN: return SKILL_JUMPING_CRASH",
        "JOBCODE_ATALANTA: return SKILL_TWIST_JAVELIN",
        "JOBCODE_KNIGHT: return SKILL_SWORD_BLAST",
        "JOBCODE_MAGICIAN: return SKILL_FIRE_BOLT",
        "JOBCODE_PRIESTESS: return SKILL_HOLY_BOLT",
        "SKILL_HEALING",
    ):
        need(executor, token, "skill mapping")

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
    need(executor, "caster->smCharInfo.Mana[0] < mana", "MP gate")
    need(executor, "caster->smCharInfo.Stamina[0] < stamina", "stamina gate")
    need(executor, "caster->smCharInfo.Mana[0] - mana", "MP consumption")
    need(executor, "caster->smCharInfo.Stamina[0] - stamina", "stamina consumption")
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
