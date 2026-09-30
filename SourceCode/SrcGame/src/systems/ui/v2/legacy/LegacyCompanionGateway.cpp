#include "systems/ui/v2/legacy/LegacyCompanionGateway.h"

#include <algorithm>
#include <cstring>
#include <windows.h>

// sinSkill.h is a legacy, non-self-contained header. Pull in the same minimal
// type providers used by the legacy client before exposing its skill-code
// constants to this UI adapter.
#include "systems/network/transport/legacy/smwsock.h"
#include "systems/rendering/legacy_3d/smd3d.h"
#include "core/data/legacy_loader/fileread.h"
#include "systems/inventory/legacy/sinItem.h"
#include "systems/combat/skills/legacy_sin/sinSkill.h"
#include "systems/network/client/netplay.h"
#include "systems/network/v2/legacy/LegacyClientCommandSender.h"

namespace ui::v2
{
namespace
{
using namespace gameplay::companions::protocol;


const char* StableCompanionSkillName(std::uint32_t skillCode)
{
    switch (skillCode)
    {
        case SKILL_EXTREME_SHIELD: return "Extreme Shield";
        case SKILL_MECHANIC_BOMB: return "Mechanic Bomb";
        case SKILL_PHYSICAL_ABSORB: return "Physical Absorption";
        case SKILL_POISON_ATTRIBUTE: return "Poison Resistance";
        case SKILL_GREAT_SMASH: return "Great Smash";
        case SKILL_MAXIMIZE: return "Maximize";
        case SKILL_AUTOMATION: return "Automation";
        case SKILL_SPARK: return "Spark";
        case SKILL_METAL_ARMOR: return "Metal Armor";
        case SKILL_GRAND_SMASH: return "Grand Smash";
        case SKILL_MECHANIC_WEAPON: return "Mechanic Weapon Mastery";
        case SKILL_SPARK_SHIELD: return "Spark Shield";
        case SKILL_IMPULSION: return "Impulsion";
        case SKILL_COMPULSION: return "Compulsion";
        case SKILL_MAGNETIC_SPHERE: return "Magnetic Sphere";
        case SKILL_METAL_GOLEM: return "Metal Golem";
        case SKILL_MELEE_MASTERY: return "Weapon Mastery";
        case SKILL_FIRE_ATTRIBUTE: return "Fire Resistance";
        case SKILL_RAVING: return "Raving";
        case SKILL_IMPACT: return "Impact";
        case SKILL_TRIPLE_IMPACT: return "Triple Impact";
        case SKILL_BRUTAL_SWING: return "Brutal Swing";
        case SKILL_ROAR: return "Roar";
        case SKILL_RAGE_OF_ZECRAM: return "Rage of Zecram";
        case SKILL_CONCENTRATION: return "Concentration";
        case SKILL_AVANGING_CRASH: return "Avenging Crash";
        case SKILL_SWIFT_AXE: return "Swift Axe";
        case SKILL_BONE_CRASH: return "Bone Crash";
        case SKILL_DETORYER: return "Destroyer";
        case SKILL_BERSERKER: return "Berserker";
        case SKILL_CYCLONE_STRIKE: return "Cyclone Strike";
        case SKILL_BOOST_HEALTH: return "Boost Health";
        case SKILL_PIKE_WIND: return "Pike Wind";
        case SKILL_ICE_ATTRIBUTE: return "Ice Resistance";
        case SKILL_CRITICAL_HIT: return "Critical Hit";
        case SKILL_JUMPING_CRASH: return "Jumping Crash";
        case SKILL_GROUND_PIKE: return "Ground Pike";
        case SKILL_TORNADO: return "Tornado";
        case SKILL_WEAPONE_DEFENCE_MASTERY: return "Weapon Defence Mastery";
        case SKILL_EXPANSION: return "Expansion";
        case SKILL_VENOM_SPEAR: return "Venom Spear";
        case SKILL_VANISH: return "Vanish";
        case SKILL_CRITICAL_MASTERY: return "Critical Mastery";
        case SKILL_CHAIN_LANCE: return "Chain Lance";
        case SKILL_ASSASSIN_EYE: return "Assassin Eye";
        case SKILL_CHARGING_STRIKE: return "Charging Strike";
        case SKILL_VAGUE: return "Vague";
        case SKILL_SHADOW_MASTER: return "Shadow Master";
        case SKILL_SCOUT_HAWK: return "Scout Hawk";
        case SKILL_SHOOTING_MASTERY: return "Shooting Mastery";
        case SKILL_WIND_ARROW: return "Wind Arrow";
        case SKILL_PERFECT_AIM: return "Perfect Aim";
        case SKILL_DIONS_EYE: return "Dions Eye";
        case SKILL_FALCON: return "Falcon";
        case SKILL_ARROW_OF_RAGE: return "Arrow of Rage";
        case SKILL_AVALANCHE: return "Avalanche";
        case SKILL_ELEMENTAL_SHOT: return "Elemental Shot";
        case SKILL_GOLDEN_FALCON: return "Golden Falcon";
        case SKILL_BOMB_SHOT: return "Bomb Shot";
        case SKILL_PERFORATION: return "Perforation";
        case SKILL_RECALL_WOLVERIN: return "Recall Wolverine";
        case SKILL_EVASION_MASTERY: return "Evasion Mastery";
        case SKILL_PHOENIX_SHOT: return "Phoenix Shot";
        case SKILL_FORCE_OF_NATURE: return "Force of Nature";
        case SKILL_SWORD_BLAST: return "Sword Blast";
        case SKILL_HOLY_BODY: return "Holy Body";
        case SKILL_PHYSICAL_TRANING: return "Physical Training";
        case SKILL_DOUBLE_CRASH: return "Double Crash";
        case SKILL_HOLY_VALOR: return "Holy Valor";
        case SKILL_BRANDISH: return "Brandish";
        case SKILL_PIERCING: return "Piercing";
        case SKILL_DRASTIC_SPIRIT: return "Drastic Spirit";
        case SKILL_SWORD_MASTERY: return "Sword Mastery";
        case SKILL_DIVINE_INHALATION: return "Divine Inhalation";
        case SKILL_HOLY_INCANTATION: return "Holy Incantation";
        case SKILL_GRAND_CROSS: return "Grand Cross";
        case SKILL_SWORD_OF_JUSTICE: return "Sword of Justice";
        case SKILL_GODLY_SHIELD: return "Godly Shield";
        case SKILL_GOD_BLESS: return "God Bless";
        case SKILL_DIVINE_PIERCING: return "Divine Piercing";
        case SKILL_SHIELD_STRIKE: return "Shield Strike";
        case SKILL_FARINA: return "Farina";
        case SKILL_THROWING_MASTERY: return "Throwing Mastery";
        case SKILL_VIGOR_SPEAR: return "Vigor Spear";
        case SKILL_WINDY: return "Windy";
        case SKILL_TWIST_JAVELIN: return "Twist Javelin";
        case SKILL_SOUL_SUCKER: return "Soul Sucker";
        case SKILL_FIRE_JAVELIN: return "Fire Javelin";
        case SKILL_SPLIT_JAVELIN: return "Split Javelin";
        case SKILL_TRIUMPH_OF_VALHALLA: return "Triumph of Valhalla";
        case SKILL_LIGHTNING_JAVELIN: return "Lightning Javelin";
        case SKILL_STORM_JAVELIN: return "Storm Javelin";
        case SKILL_HALL_OF_VALHALLA: return "Hall of Valhalla";
        case SKILL_X_RAGE: return "X Rage";
        case SKILL_FROST_JAVELIN: return "Frost Javelin";
        case SKILL_VENGEANCE: return "Vengeance";
        case SKILL_HEALING: return "Healing";
        case SKILL_HOLY_BOLT: return "Holy Bolt";
        case SKILL_MULTISPARK: return "Multi Spark";
        case SKILL_HOLY_MIND: return "Holy Mind";
        case SKILL_MEDITATION: return "Meditation";
        case SKILL_DIVINE_LIGHTNING: return "Divine Lightning";
        case SKILL_HOLY_REFLECTION: return "Holy Reflection";
        case SKILL_GRAND_HEALING: return "Grand Healing";
        case SKILL_VIGOR_BALL: return "Vigor Ball";
        case SKILL_RESURRECTION: return "Resurrection";
        case SKILL_EXTINCTION: return "Extinction";
        case SKILL_VIRTUAL_LIFE: return "Virtual Life";
        case SKILL_GLACIAL_SPIKE: return "Glacial Spike";
        case SKILL_REGENERATION_FIELD: return "Regeneration Field";
        case SKILL_CHAIN_LIGHTNING: return "Chain Lightning";
        case SKILL_SUMMON_MUSPELL: return "Summon Muspell";
        case SKILL_AGONY: return "Agony";
        case SKILL_FIRE_BOLT: return "Fire Bolt";
        case SKILL_ZENITH: return "Zenith";
        case SKILL_FIRE_BALL: return "Fire Ball";
        case SKILL_MENTAL_MASTERY: return "Mental Mastery";
        case SKILL_WATORNADO: return "Watornado";
        case SKILL_ENCHANT_WEAPON: return "Enchant Weapon";
        case SKILL_DEAD_RAY: return "Dead Ray";
        case SKILL_ENERGY_SHIELD: return "Energy Shield";
        case SKILL_DIASTROPHISM: return "Diastrophism";
        case SKILL_SPIRIT_ELEMENTAL: return "Spirit Elemental";
        case SKILL_DANCING_SWORD: return "Dancing Sword";
        case SKILL_FIRE_ELEMENTAL: return "Fire Elemental";
        case SKILL_FLAME_WAVE: return "Flame Wave";
        case SKILL_DISTORTION: return "Distortion";
        case SKILL_M_METEO: return "Meteor";
        default: return "";
    }
}

std::int32_t ToWireAction(CompanionUiAction action)
{
    switch (action)
    {
        case CompanionUiAction::Refresh: return COMPANION_UI_REFRESH;
        case CompanionUiAction::Select: return COMPANION_UI_SELECT;
        case CompanionUiAction::Summon: return COMPANION_UI_SUMMON;
        case CompanionUiAction::Dismiss: return COMPANION_UI_DISMISS;
        case CompanionUiAction::DismissAll: return COMPANION_UI_DISMISS_ALL;
        case CompanionUiAction::SetMode: return COMPANION_UI_SET_MODE;
        case CompanionUiAction::EquipRightNext: return COMPANION_UI_EQUIP_RIGHT_NEXT;
        case CompanionUiAction::EquipRightPrev: return COMPANION_UI_EQUIP_RIGHT_PREV;
        case CompanionUiAction::EquipLeftNext: return COMPANION_UI_EQUIP_LEFT_NEXT;
        case CompanionUiAction::EquipLeftPrev: return COMPANION_UI_EQUIP_LEFT_PREV;
        case CompanionUiAction::ResetEquipment: return COMPANION_UI_RESET_EQUIPMENT;
        case CompanionUiAction::ResyncData: return COMPANION_UI_RESYNC_DAT;
        case CompanionUiAction::SetTactic: return COMPANION_UI_SET_TACTIC;
        case CompanionUiAction::SetRetreatHp: return COMPANION_UI_SET_RETREAT_HP;
        case CompanionUiAction::SetPreferredRange: return COMPANION_UI_SET_PREFERRED_RANGE;
        case CompanionUiAction::CycleActiveSkill: return COMPANION_UI_CYCLE_ACTIVE_SKILL;
        case CompanionUiAction::ResetActiveSkills: return COMPANION_UI_RESET_ACTIVE_SKILLS;
        case CompanionUiAction::CycleBuffSkill: return COMPANION_UI_CYCLE_BUFF_SKILL;
        case CompanionUiAction::AdjustBuffInterval: return COMPANION_UI_ADJUST_BUFF_INTERVAL;
        case CompanionUiAction::ResetBuffs: return COMPANION_UI_RESET_BUFFS;
        case CompanionUiAction::CycleActiveMinResource: return COMPANION_UI_CYCLE_ACTIVE_MIN_RESOURCE;
        case CompanionUiAction::CycleActiveMinTargets: return COMPANION_UI_CYCLE_ACTIVE_MIN_TARGETS;
        case CompanionUiAction::CycleActiveTargetMode: return COMPANION_UI_CYCLE_ACTIVE_TARGET_MODE;
        case CompanionUiAction::ToggleCustomTactics: return COMPANION_UI_TOGGLE_CUSTOM_TACTICS;
        case CompanionUiAction::ToggleTacticRule: return COMPANION_UI_TOGGLE_TACTIC_RULE;
        case CompanionUiAction::CycleTacticCondition: return COMPANION_UI_CYCLE_TACTIC_CONDITION;
        case CompanionUiAction::CycleTacticAction: return COMPANION_UI_CYCLE_TACTIC_ACTION;
        case CompanionUiAction::CycleTacticThreshold: return COMPANION_UI_CYCLE_TACTIC_THRESHOLD;
        case CompanionUiAction::MoveTacticRule: return COMPANION_UI_MOVE_TACTIC_RULE;
    }
    return COMPANION_UI_REFRESH;
}
}

LegacyCompanionGateway& GetLegacyCompanionGateway()
{
    static LegacyCompanionGateway gateway;
    return gateway;
}

void LegacyCompanionGateway::RequestRefresh()
{
    Execute(CompanionUiAction::Refresh);
}

void LegacyCompanionGateway::Execute(CompanionUiAction action, const std::string& characterName, int value)
{
    using namespace gameplay::companions::protocol;
    auto& sender = gameplay::network::legacy::ClientLegacyCommandSender();
    if (!sender.Available())
        return;

    CompanionUIActionPacket packet{};
    packet.size = sizeof(packet);
    packet.code = OPCODE_COMPANION_UI_ACTION;
    packet.version = COMPANION_UI_PROTOCOL_VERSION;
    packet.action = ToWireAction(action);
    packet.value = value;
    if (!characterName.empty())
        std::strncpy(packet.characterName, characterName.c_str(), sizeof(packet.characterName) - 1);

    sender.Send(gameplay::network::PacketView(&packet, packet.size, packet.code));
}

void LegacyCompanionGateway::ApplyState(const gameplay::companions::protocol::CompanionUIStatePacket& packet)
{
    using namespace gameplay::companions::protocol;
    if (packet.version != COMPANION_UI_PROTOCOL_VERSION)
        return;

    snapshot_ = CompanionUiSnapshot{};
    snapshot_.valid = true;
    snapshot_.mode = static_cast<CompanionUiMode>(std::clamp(packet.mode, 0, 2));
    snapshot_.activeCount = packet.activeCount;
    snapshot_.maxActive = packet.maxActive;
    snapshot_.lastResult = packet.lastResult;
    snapshot_.skillLoadoutActive = packet.skillLoadoutActive != 0;
    snapshot_.buffLoadoutActive = packet.buffLoadoutActive != 0;
    snapshot_.customTacticsEnabled = packet.customTacticsEnabled != 0;
    for (int rule = 0; rule < COMPANION_UI_TACTIC_RULES; ++rule)
    {
        const auto& sourceRule = packet.tacticRules[rule];
        auto& targetRule = snapshot_.tacticRules[static_cast<std::size_t>(rule)];
        targetRule.enabled = sourceRule.enabled != 0;
        targetRule.condition = std::clamp(sourceRule.condition, 0, (int)COMPANION_TACTIC_CONDITION_PARTY_THREATENED);
        targetRule.action = std::clamp(sourceRule.action, 0, (int)COMPANION_TACTIC_ACTION_HOLD_POSITION);
        targetRule.threshold = std::clamp(sourceRule.threshold, 0, 95);
    }
    for (int slot = 0; slot < 4; ++slot)
    {
        const auto& sourceActive = packet.activeSkills[slot];
        auto& targetActive = snapshot_.activeSkills[static_cast<std::size_t>(slot)];
        targetActive.useSlot = sourceActive.useSlot;
        targetActive.skillCode = sourceActive.skillCode;
        targetActive.point = sourceActive.point;
        targetActive.intervalSeconds = sourceActive.intervalSeconds;
        targetActive.minResourcePercent = std::clamp(sourceActive.minResourcePercent, 0, 90);
        targetActive.minimumTargets = std::clamp(sourceActive.minimumTargets, 1, 16);
        targetActive.targetMode = std::clamp(sourceActive.targetMode, 0, 2);
        targetActive.name = sourceActive.skillName;

        const auto& sourceBuff = packet.buffSkills[slot];
        auto& targetBuff = snapshot_.buffSkills[static_cast<std::size_t>(slot)];
        targetBuff.useSlot = sourceBuff.useSlot;
        targetBuff.skillCode = sourceBuff.skillCode;
        targetBuff.point = sourceBuff.point;
        targetBuff.intervalSeconds = sourceBuff.intervalSeconds;
        targetBuff.name = sourceBuff.skillName;
    }
    snapshot_.entryCount = std::clamp(packet.entryCount, 0, static_cast<int>(CompanionUiMaxCharacters));

    for (int i = 0; i < snapshot_.entryCount; ++i)
    {
        const auto& source = packet.entries[i];
        auto& target = snapshot_.entries[static_cast<std::size_t>(i)];
        target.name = source.characterName;
        target.level = source.level;
        target.jobCode = source.jobCode;
        target.active = source.active != 0;
        target.selected = source.selected != 0;
        target.currentCharacter = source.currentCharacter != 0;
        target.rightItemCode = source.rightItemCode;
        target.leftItemCode = source.leftItemCode;
        target.rightItemName = source.rightItemName;
        target.leftItemName = source.leftItemName;
        target.life = source.life;
        target.maxLife = source.maxLife;
        target.mana = source.mana;
        target.maxMana = source.maxMana;
        target.dead = source.dead != 0;
        target.targetName = source.targetName;
        target.mode = static_cast<CompanionUiMode>(std::clamp(source.mode, 0, 2));
        target.tactic = static_cast<CompanionUiTactic>(std::clamp(source.tactic, 0, 3));
        target.retreatHpPercent = std::clamp(source.retreatHpPercent, 0, 90);
        target.preferredRange = std::clamp(source.preferredRange, 0, 600);
    }
}

void LegacyCompanionGateway::ApplyRuntimeState(const gameplay::companions::protocol::CompanionRuntimeStatePacket& packet)
{
    using namespace gameplay::companions::protocol;
    if (packet.version != COMPANION_RUNTIME_PROTOCOL_VERSION)
        return;

    runtimeSnapshot_ = CompanionRuntimeSnapshot{};
    runtimeSnapshot_.valid = true;
    runtimeSnapshot_.entryCount = std::clamp(packet.entryCount, 0, COMPANION_RUNTIME_MAX_ACTIVE);
    for (int i = 0; i < runtimeSnapshot_.entryCount; ++i)
    {
        const auto& source = packet.entries[i];
        auto& target = runtimeSnapshot_.entries[static_cast<std::size_t>(i)];
        target.name = source.characterName;
        target.objectSerial = source.objectSerial;
        target.level = source.level;
        target.jobCode = source.jobCode;
        target.life = source.life;
        target.maxLife = source.maxLife;
        target.mana = source.mana;
        target.maxMana = source.maxMana;
        target.dead = source.dead != 0;
        target.mode = static_cast<CompanionUiMode>(std::clamp(source.mode, 0, 2));
        target.tactic = static_cast<CompanionUiTactic>(std::clamp(source.tactic, 0, 3));
        target.casting = source.casting != 0;
        target.castingSkillCode = source.castingSkillCode;
        target.castingSkillName = source.castingSkillName;
        target.castProgressPermille = std::clamp(source.castProgressPermille, 0, 1000);
        target.cooldownMs = std::max(0, source.cooldownMs);
        target.statusFlags = source.statusFlags;
        target.effectCount = std::clamp(source.effectCount, 0, COMPANION_RUNTIME_MAX_EFFECTS);
        for(int effect = 0; effect < target.effectCount; ++effect)
        {
            target.effects[static_cast<std::size_t>(effect)].effectCode = source.effects[effect].effectCode;
            target.effects[static_cast<std::size_t>(effect)].name = StableCompanionSkillName(source.effects[effect].effectCode);
            target.effects[static_cast<std::size_t>(effect)].remainingMs = std::max(0, source.effects[effect].remainingMs);
            target.effects[static_cast<std::size_t>(effect)].positive = source.effects[effect].positive != 0;
        }
        target.targetName = source.targetName;
    }
}

void LegacyCompanionGateway::Reset()
{
    snapshot_ = CompanionUiSnapshot{};
    runtimeSnapshot_ = CompanionRuntimeSnapshot{};
}

void ApplyLegacyCompanionStatePacket(const gameplay::companions::protocol::CompanionUIStatePacket& packet)
{
    GetLegacyCompanionGateway().ApplyState(packet);
}

void ApplyLegacyCompanionRuntimePacket(const gameplay::companions::protocol::CompanionRuntimeStatePacket& packet)
{
    GetLegacyCompanionGateway().ApplyRuntimeState(packet);
}
}
