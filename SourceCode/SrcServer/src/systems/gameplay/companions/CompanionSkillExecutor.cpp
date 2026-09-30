#include "systems/gameplay/companions/CompanionSkillExecutor.h"

#include <algorithm>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "content/v1/gameplay_config/content/GameplayConfigRuntime.h"
#include "systems/network/transport/legacy/smwsock.h"
#include "systems/rendering/legacy_3d/smd3d.h"
#include "systems/gameplay/character/core/character.h"
#include "systems/audio/effects/legacy/effectsnd.h"
#include "core/server/gameplay_runtime/playmain.h"
#include "core/gameplay/legacy_support/playsub.h"
#include "core/server/legacy_runtime/onserver.h"
#include "systems/combat/skills/legacy/sinSkill.h"
#include "inventory/legacy/sinItem_legacy_server.h"
#include "gameplay/skills/Tempskron_legacy_server.h"
#include "gameplay/skills/Morayion_legacy_server.h"
#include "gameplay/skills/v2/legacy/LegacySkillAdapter.h"
#include "gameplay/companions/v2/CompanionSkillProfile.h"
#include "gameplay/skills/v2/runtime/SkillRuntime.h"
#include "gameplay/combat/v2/DamageContracts.h"
#include "gameplay/combat/v2/LifeCommands.h"
#include "gameplay/combat/v2/HealingService.h"
#include "gameplay/combat/v2/TargetingService.h"
#include "gameplay/combat/v2/StatusEffectService.h"
#include "gameplay/combat/v2/CombatBuffService.h"
#include "gameplay/inventory/v2/PotionService.h"
#include "gameplay/monsters/v2/MonsterCommands.h"
#include "gameplay/monsters/v2/MonsterQuery.h"
#include "gameplay/world/v2/MovementCommands.h"
#include "systems/gameplay/combat/v2/legacy/LegacyDamageAdapter.h"
#include "systems/gameplay/combat/v2/legacy/LegacyLifeAdapter.h"
#include "systems/gameplay/combat/v2/legacy/LegacyHealingAdapter.h"
#include "systems/gameplay/combat/v2/legacy/LegacyTargetingAdapter.h"
#include "systems/gameplay/combat/v2/legacy/LegacyStatusEffectAdapter.h"
#include "systems/gameplay/combat/v2/legacy/LegacyCombatBuffAdapter.h"
#include "systems/gameplay/inventory/v2/legacy/LegacyPotionAdapter.h"
#include "systems/gameplay/monsters/v2/legacy/LegacyMonsterAdapter.h"
#include "systems/gameplay/world/v2/legacy/LegacyMovementAdapter.h"
#include "systems/gameplay/skills/v2/legacy/LegacySkillRuntimeAdapter.h"
#include "systems/gameplay/player/v2/legacy/LegacyOnlinePlayerRegistry.h"
#include "opcodes.h"

extern DWORD dwPlayServTime;

namespace gameplay
{
namespace companions
{
namespace
{

static int SummitCompanionConfigInt(const char *key, int fallback)
{
    if(summit::content::gameplay_config::CompiledGameplayConfigEnabled())
    {
        int value = 0;
        if(summit::content::gameplay_config::TryGameplayConfigInt("Configs/Companion.ini", "Companion", key, value))
            return value;
        if(summit::content::gameplay_config::GameplayConfigStrictMode())
            return fallback;
    }
    return GetPrivateProfileIntA("Companion", key, fallback, ".//Configs//Companion.ini");
}

#include "gameplay/skills/v2/legacy/LegacyExtendedSkillMetadata.inl"

static int RandomBetween(int low, int high)
{
    if(high < low)
        std::swap(low, high);
    if(high <= low)
        return low;
    return low + (rand() % (high - low + 1));
}

// Legacy potion restore uses an upper-exclusive range when min/max differ.
static int PotionRandomBetween(int low, int high)
{
    if(high < low)
        std::swap(low, high);
    if(high <= low)
        return low;
    return low + (rand() % (high - low));
}

static int ClampPercent(int value)
{
    if(value < 0) return 0;
    if(value > 100) return 100;
    return value;
}

static bool AutoUsePotionsEnabled()
{
    return SummitCompanionConfigInt("AutoUsePotions", 1) != 0;
}

static int PotionThreshold(const char *key, int fallback)
{
    return ClampPercent(SummitCompanionConfigInt(key, fallback));
}

static int SkillResourceReservePercent(const char *key, int fallback)
{
    return ClampPercent(GetPrivateProfileIntA(
        "Companion", key, fallback, ".//Configs//Companion.ini"));
}

static int SkillResourceReserveAmount(int maximum, const char *key, int fallbackPercent)
{
    if(maximum <= 0)
        return 0;
    const int percent = SkillResourceReservePercent(key, fallbackPercent);
    return (maximum * percent + 99) / 100;
}

static DWORD DamageSkillForJob(DWORD jobCode)
{
    return static_cast<DWORD>(DefaultDamageSkillForClass(
        static_cast<classes::PlayableClassId>(jobCode)));
}

static const gameplay::skills::SkillDefinition *FindCatalogSkillForSlot(DWORD jobCode, int useSlot)
{
    return gameplay::skills::DefaultLegacySkillAdapter().ResolveSlot((int)jobCode, useSlot);
}

static int LearnedPointForCode(DWORD jobCode, const BYTE *points, DWORD skillCode)
{
    if(!points || !skillCode)
        return 0;

    const gameplay::skills::SkillDefinition *definition =
        gameplay::skills::DefaultLegacySkillAdapter().Find((gameplay::skills::SkillId)skillCode);
    if(!definition || definition->ownerClassId != (int)jobCode || definition->useSlot < 1)
        return 0;

    const int index = definition->useSlot - 1;
    return index >= 0 && index < COMPANION_MAX_SKILL_SLOTS
        ? std::min(10, (int)points[index]) : 0;
}

static bool ResolveUseSlot(DWORD jobCode, const BYTE *points, int requestedSlot,
    DWORD *skillCode, int *point, const char **skillName)
{
    if(skillCode) *skillCode = 0;
    if(point) *point = 0;
    if(skillName) *skillName = "";
    if(!points || requestedSlot < 1 || requestedSlot > COMPANION_MAX_SKILL_SLOTS)
        return false;

    const gameplay::skills::SkillDefinition *definition =
        FindCatalogSkillForSlot(jobCode, requestedSlot);
    if(!definition)
        return false;

    if(skillCode) *skillCode = (DWORD)definition->id;
    if(point) *point = std::min(10, (int)points[requestedSlot - 1]);
    if(skillName) *skillName = definition->name.c_str();
    return true;
}

static const char *CanonicalCompanionSkillName(DWORD skillCode)
{
    const gameplay::skills::SkillDefinition *definition =
        gameplay::skills::DefaultLegacySkillAdapter().Find((gameplay::skills::SkillId)skillCode);
    return definition ? definition->name.c_str() : "";
}

static bool IsDamageSkillCode(DWORD skillCode)
{
    return gameplay::skills::DefaultLegacySkillAdapter().IsDamage((gameplay::skills::SkillId)skillCode);
}

static bool IsControlSkillCode(DWORD skillCode)
{
    return gameplay::skills::DefaultLegacySkillAdapter().IsControl((gameplay::skills::SkillId)skillCode);
}

static bool IsOffensiveSkillCode(DWORD skillCode)
{
    return gameplay::skills::DefaultLegacySkillAdapter().IsOffensive((gameplay::skills::SkillId)skillCode);
}

static bool IsBuffSkillCode(DWORD skillCode)
{
    return gameplay::skills::DefaultLegacySkillAdapter().IsBuff((gameplay::skills::SkillId)skillCode);
}

static bool IsSupportSkillCode(DWORD skillCode)
{
    return gameplay::skills::DefaultLegacySkillAdapter().IsSupport((gameplay::skills::SkillId)skillCode);
}

static bool IsMagicDamageSkillCode(DWORD skillCode)
{
    switch(skillCode)
    {
        case SKILL_FIRE_BOLT:
        case SKILL_FIRE_BALL:
        case SKILL_WATORNADO:
        case SKILL_DEAD_RAY:
        case SKILL_DIASTROPHISM:
        case SKILL_M_METEO:
        case SKILL_T5_LIGHTNING_SURGE:
        case SKILL_T5_RED_RAY:
        case SKILL_T5_CATACLYSM:
            return true;
        default:
            return false;
    }
}

static int ActiveMagicDamagePercent(const CompanionSkillRuntime &runtime)
{
    int percent = 0;
    for(int i = 0; i < 4; ++i)
        if(runtime.buffModifiers[i].skillCode == SKILL_SPIRIT_ELEMENTAL)
            percent += runtime.buffModifiers[i].magicSkillDamagePercent;
    return percent;
}

static int ActiveGenericDamagePercent(const CompanionSkillRuntime &runtime)
{
    int percent = 0;
    for(int i = 0; i < 4; ++i)
        percent += runtime.buffModifiers[i].genericDamagePercent;
    return percent;
}

static int ActiveUndeadDamagePercent(const CompanionSkillRuntime &runtime)
{
    int percent = 0;
    for(int i = 0; i < 4; ++i)
        percent += runtime.buffModifiers[i].undeadDamagePercent;
    return percent;
}

static int ActiveCriticalChancePercent(const CompanionSkillRuntime &runtime)
{
    int percent = 0;
    for(int i = 0; i < 4; ++i)
        percent += runtime.buffModifiers[i].criticalChancePercent;
    return std::max(0, std::min(100, percent));
}

// V43 runtime combat modifiers retained alongside the canonical/Tier-5 modifiers.
static int ActiveOutgoingFlatDamage(const CompanionSkillRuntime &runtime)
{
    int flat = 0;
    for(int i = 0; i < 4; ++i)
        flat += runtime.buffModifiers[i].outgoingFlatDamage;
    return flat;
}

static int ActiveForceNatureDamage(const CompanionSkillRuntime &runtime, DWORD now)
{
    return runtime.forceNatureExpiresAt && (int)(runtime.forceNatureExpiresAt - now) > 0
        ? runtime.forceNatureDamage : 0;
}

static int ActiveHolyValorPercent(const CompanionSkillRuntime &runtime, DWORD now)
{
    return runtime.holyValorExpiresAt && (int)(runtime.holyValorExpiresAt - now) > 0
        ? ClampPercent(runtime.holyValorPercent) : 0;
}

static int ActiveValhallaMaxDamageBonus(const CompanionSkillRuntime &runtime, DWORD now)
{
    if(runtime.hallValhallaExpiresAt && (int)(runtime.hallValhallaExpiresAt - now) > 0)
        return std::max(0, runtime.hallValhallaMaxDamageBonus);
    if(runtime.triumphValhallaExpiresAt && (int)(runtime.triumphValhallaExpiresAt - now) > 0)
        return std::max(0, runtime.triumphValhallaMaxDamageBonus);
    return 0;
}

static int ActiveHallValhallaEvasion(const CompanionSkillRuntime &runtime, DWORD now)
{
    return runtime.hallValhallaExpiresAt && (int)(runtime.hallValhallaExpiresAt - now) > 0
        ? ClampPercent(runtime.hallValhallaEvasionPercent) : 0;
}

static int ApplyEnchantWeaponBonus(const CompanionSkillRuntime &runtime, smCHAR *target, int damage, DWORD now)
{
    if(!target || damage <= 0 || !runtime.enchantWeaponExpiresAt ||
       (int)(runtime.enchantWeaponExpiresAt - now) <= 0 ||
       runtime.enchantWeaponPoint < 1 || runtime.enchantWeaponPoint > 10)
        return damage;
    const int p = runtime.enchantWeaponPoint - 1;
    int low = 0, high = 0, resistanceIndex = sITEMINFO_NONE;
    switch(runtime.enchantWeaponElement)
    {
        case 0: low = Enchant_Weapon_Damage_Ice[p][0]; high = Enchant_Weapon_Damage_Ice[p][1]; resistanceIndex = sITEMINFO_ICE; break;
        case 1: low = Enchant_Weapon_Damage_Lightning[p][0]; high = Enchant_Weapon_Damage_Lightning[p][1]; resistanceIndex = sITEMINFO_LIGHTING; break;
        case 2: low = Enchant_Weapon_Damage_Fire[p][0]; high = Enchant_Weapon_Damage_Fire[p][1]; resistanceIndex = sITEMINFO_FIRE; break;
        default: return damage;
    }
    damage += RandomBetween(low, high);
    int resistance = (int)target->smCharInfo.Resistance[resistanceIndex];
    resistance = std::max(-100, std::min(100, resistance));
    damage -= (high * resistance) / 100;
    if(damage < 0) damage = 0;
    if(runtime.enchantWeaponElement == 0)
        combat::legacy::ServerLegacyStatusEffectAdapter().ApplyElementalSlow16Hz(
            EntityId(static_cast<std::uint32_t>(target->dwObjectSerial)),
            3, 230 - 10 * runtime.enchantWeaponPoint, resistance);
    return damage;
}

static const sSKILL_INFO *FindSkillInfo(DWORD skillCode)
{
    for(int i = 0; i < SIN_MAX_SKILL; ++i)
        if(sSkill_Info[i].CODE == skillCode)
            return &sSkill_Info[i];
    return NULL;
}

static bool WeaponAllowed(const sSKILL_INFO *info, DWORD rightCode, DWORD leftCode)
{
    if(!info)
        return false;

    bool restricted = false;
    for(int i = 0; i < 8; ++i)
    {
        const DWORD allowed = info->UseWeaponCode[i];
        if(!allowed)
            continue;
        restricted = true;
        if(allowed == SKILL_ALL_WEAPON_USE)
            return true;
        if(rightCode && (rightCode & sinITEM_MASK2) == (allowed & sinITEM_MASK2))
            return true;
        if(leftCode && (leftCode & sinITEM_MASK2) == (allowed & sinITEM_MASK2))
            return true;
    }
    return !restricted;
}

static bool ExtendedWeaponAllowed(const LegacyExtendedSkillMeta *meta, DWORD rightCode, DWORD leftCode)
{
    if(!meta) return false;
    bool restricted = false;
    for(int i = 0; i < 8; ++i)
    {
        const DWORD allowed = meta->weaponCode[i];
        if(!allowed) continue;
        restricted = true;
        if(allowed == SKILL_ALL_WEAPON_USE) return true;
        if(rightCode && (rightCode & sinITEM_MASK2) == (allowed & sinITEM_MASK2)) return true;
        if(leftCode && (leftCode & sinITEM_MASK2) == (allowed & sinITEM_MASK2)) return true;
    }
    return !restricted;
}

static int ExtendedManaCost(const LegacyExtendedSkillMeta *meta, int point)
{
    if(!meta || !meta->manaTable || point < 1 || point > 10) return 0;
    return std::max(0, meta->manaTable[point - 1]);
}

static int ExtendedStaminaCost(const LegacyExtendedSkillMeta *meta, int point)
{
    if(!meta || point < 1 || point > 10) return 0;
    return std::max(0, meta->staminaBase + meta->staminaPerPoint * (point - 1));
}

static int ManaCost(const sSKILL_INFO *info, int point)
{
    if(!info || !info->UseMana || point < 1 || point > 10)
        return 0;
    const int value = info->UseMana[point - 1];
    return value > 0 ? value : 0;
}

static int StaminaCost(const sSKILL_INFO *info, int point)
{
    if(!info || point < 1 || point > 10)
        return 0;
    const int value = info->UseStamina[0] +
        (info->UseStamina[1] * (point - 1));
    return value > 0 ? value : 0;
}

static int SkillRange(DWORD skillCode, const smCHAR *caster, int point)
{
    const int p = std::max(0, std::min(9, point - 1));
    if(skillCode == SKILL_PIKE_WIND)
        return std::max(1, Pike_Wind_Push_Lenght[p]);
    if(skillCode == SKILL_SHIELD_STRIKE)
        return 160;
    if(skillCode == SKILL_SOUL_SUCKER)
        return 200;
    if(skillCode == SKILL_GROUND_PIKE)
        return std::max(1, G_Pike_Range[p]);
    if(skillCode == SKILL_ROAR)
        return std::max(1, Roar_Range[p]);
    if(skillCode == SKILL_DISTORTION)
        return std::max(1, Distortion_Area[p]);
    if(skillCode == SKILL_VENOM_SPEAR)
        return std::max(1, VenomSpear_Range[p]);
    if(skillCode == SKILL_EXTINCTION)
        return 160;
    if(skillCode == SKILL_GLACIAL_SPIKE)
        return 340;
    if(skillCode == SKILL_FLAME_WAVE)
        return 340;
    if(skillCode == SKILL_PIERCING)
        return 140;
    if(skillCode == SKILL_PERFORATION)
    {
        return (caster ? caster->smCharInfo.Shooting_Range : 300) + Perforation_Attack_Range[p] + 40;
    }
    if(skillCode == SKILL_SWORD_BLAST)
    {
        int pointRange = caster ? caster->smCharInfo.Shooting_Range + 120 : 260;
        if(pointRange < 260) pointRange = 260;
        return pointRange;
    }
    if(skillCode == SKILL_WIND_ARROW || skillCode == SKILL_PERFECT_AIM ||
       skillCode == SKILL_BOMB_SHOT || skillCode == SKILL_PERFORATION || skillCode == SKILL_ARROW_OF_RAGE || skillCode == SKILL_AVALANCHE ||
       skillCode == SKILL_TWIST_JAVELIN || skillCode == SKILL_FIRE_JAVELIN || skillCode == SKILL_SPLIT_JAVELIN ||
       skillCode == SKILL_VENOM_SPEAR || skillCode == SKILL_SOUL_SUCKER ||
       skillCode == SKILL_HOLY_BOLT || skillCode == SKILL_MULTISPARK || skillCode == SKILL_HOLY_MIND ||
       skillCode == SKILL_GLACIAL_SPIKE || skillCode == SKILL_EXTINCTION ||
       skillCode == SKILL_FIRE_BOLT || skillCode == SKILL_FIRE_BALL ||
       skillCode == SKILL_WATORNADO || skillCode == SKILL_DEAD_RAY || skillCode == SKILL_DISTORTION ||
       skillCode == SKILL_MECHANIC_BOMB || skillCode == SKILL_SPARK ||
       skillCode == SKILL_IMPULSION || skillCode == SKILL_ELEMENTAL_SHOT ||
       skillCode == SKILL_PHOENIX_SHOT || skillCode == SKILL_VIGOR_SPEAR ||
       skillCode == SKILL_LIGHTNING_JAVELIN || skillCode == SKILL_STORM_JAVELIN ||
       skillCode == SKILL_VENGEANCE || skillCode == SKILL_DIVINE_LIGHTNING ||
       skillCode == SKILL_VIGOR_BALL || skillCode == SKILL_CHAIN_LIGHTNING ||
       skillCode == SKILL_DIASTROPHISM || skillCode == SKILL_M_METEO ||
       skillCode == SKILL_ASSASSIN_EYE)
    {
        int range = caster ? caster->smCharInfo.Shooting_Range + 96 : 300;
        if(range < 260) range = 260;
        if(range > 520) range = 520;
        return range;
    }
    switch(skillCode)
    {
        case SKILL_T5_PARASIT_SHOT:
        case SKILL_T5_FIERCE_WIND:
        case SKILL_T5_BOMBARDMENT:
        case SKILL_T5_PLAGUE_JAVELIN:
        case SKILL_T5_COMBO_JAVELIN:
        case SKILL_T5_LIGHTNING_SURGE:
        case SKILL_T5_ENTALING_ROOTS:
        case SKILL_T5_HEAVENLY_LIGHT:
        case SKILL_T5_CATACLYSM:
            return std::max(260, std::min(560, (caster ? caster->smCharInfo.Shooting_Range : 300) + 96));
        case SKILL_T5_RED_RAY:
            return std::max(260, std::min(600, (caster ? caster->smCharInfo.Shooting_Range : 300) + Magician_T5_RedRay_AddRange[std::max(0, std::min(9, point - 1))]));
        case SKILL_T5_DIVINE_INQUISITON:
        case SKILL_T5_SEISMIC_IMPACT:
        case SKILL_T5_MAGNETIC_DISCHARGE:
        case SKILL_T5_TWISTER:
        case SKILL_T5_WHIRLWIND:
            return 220;
        case SKILL_T5_RING_OF_SPEARS:
        case SKILL_T5_RUPTURE:
        case SKILL_T5_DIVINE_CROSS:
            return 150;
        default:
            break;
    }
    return 125;
}

static bool InRange(const smCHAR *a, const smCHAR *b, int range)
{
    if(!a || !b || range <= 0)
        return false;
    const int dx = (b->pX - a->pX) >> FLOATNS;
    const int dz = (b->pZ - a->pZ) >> FLOATNS;
    if(abs((b->pY - a->pY) >> FLOATNS) > 128)
        return false;
    return dx * dx + dz * dz <= range * range;
}

static int DamageForSkill(const smCHAR *caster, const smCHAR *target, DWORD skillCode, int point, const CompanionSkillRuntime *runtime = NULL, bool includePostRuntimeModifiers = true)
{
    if(!caster || point < 1 || point > 10)
        return 0;

    const int p = point - 1;
    int base = RandomBetween(caster->smCharInfo.Attack_Damage[0], caster->smCharInfo.Attack_Damage[1]);
    if(base < 1) base = 1;

    int damage = base;
    switch(skillCode)
    {
        case SKILL_RAVING:
            damage += (damage * Raving_Damage[p]) / 100;
            break;
        case SKILL_IMPACT:
            damage += (damage * Impact_Damage[p]) / 100;
            break;
        case SKILL_TRIPLE_IMPACT:
        {
            const int perHit = damage + (damage * T_Impact_Damage[p]) / 100;
            damage = perHit * std::max(1, T_Impact_Hit[p]);
            break;
        }
        case SKILL_BRUTAL_SWING:
            damage += (damage * B_Swing_Damage[p]) / 100;
            if(target && target->smCharInfo.Brood == smCHAR_MONSTER_DEMON)
                damage += (damage * 40) / 100;
            break;
        case SKILL_GREAT_SMASH:
        {
            const int percent = G_Smash_Attack_RagePercent[p] > 0 ?
                G_Smash_Attack_RagePercent[p] : G_Smash_Attack_Rage[p];
            damage += (damage * percent) / 100;
            break;
        }
        case SKILL_MECHANIC_BOMB:
            damage += (damage * Mechanic_Bomb_DamagePercent[p]) / 100;
            break;
        case SKILL_SPARK:
            damage += (damage * Spark_DamagePercent[p]) / 100;
            break;
        case SKILL_PIKE_WIND:
            damage += (damage * Pike_Wind_DamagePercent[p]) / 100;
            break;
        case SKILL_CRITICAL_HIT:
        {
            int critical = caster->smCharInfo.Critical_Hit + Critical_Hit_Critical[p];
            critical += ((caster->smCharInfo.Level - (target ? target->smCharInfo.Level : caster->smCharInfo.Level)) * 25) / 100;
            if(target && target->dwAssassinEyeTime > dwPlayServTime && critical > 0)
                critical -= target->AssassinEyeParam;
            if(critical > 70) critical = 70;
            if(critical > 0 && (rand() % 100) < critical)
                damage = (damage * 170) / 100;
            break;
        }
        case SKILL_GROUND_PIKE:
            damage += (damage * G_Pike_DamagePercent[p]) / 100;
            break;
        case SKILL_JUMPING_CRASH:
            damage += (damage * Jumping_Crash_Damage[p]) / 100;
            if(target && target->smCharInfo.Brood == smCHAR_MONSTER_DEMON)
                damage += (damage * 30) / 100;
            break;
        case SKILL_WIND_ARROW:
            damage += (damage * Wind_Arrow_Damage[p]) / 100;
            if(target && target->smCharInfo.Brood == smCHAR_MONSTER_NORMAL)
                damage += (damage * 30) / 100;
            break;
        case SKILL_BOMB_SHOT:
            damage += (damage * BombShot_Damage[p]) / 100;
            if(target && target->smCharInfo.Brood == smCHAR_MONSTER_DEMON)
                damage += (damage * BombShot_DemonDamage[p]) / 100;
            break;
        case SKILL_PERFORATION:
            damage += (damage * Perforation_Damage[p]) / 100;
            break;
        case SKILL_ARROW_OF_RAGE:
            // Legacy server range handler adds level * A_Rage_Damage rather
            // than treating the table as a percentage.
            damage += caster->smCharInfo.Level * A_Rage_Damage[p];
            break;
        case SKILL_AVALANCHE:
            // One arrow per event. Remaining arrows are scheduled by V9.
            damage += (damage * Avalanche_Damage[p]) / 100;
            break;
        case SKILL_TWIST_JAVELIN:
            damage += (damage * Twist_Javelin_Damage[p]) / 100;
            if(target && target->smCharInfo.Brood == smCHAR_MONSTER_DEMON)
                damage += (damage * 30) / 100;
            break;
        case SKILL_SWORD_BLAST:
            if(Sword_Blast_DamagePercent[p] > 0)
                damage += (damage * Sword_Blast_DamagePercent[p]) / 100;
            else
                damage += RandomBetween(Sword_Blast_Damage[p][0], Sword_Blast_Damage[p][1]);
            break;
        case SKILL_MULTISPARK:
            // The legacy server sends one damage event scaled by the random
            // spark count encoded in the high AttackSkil bits. The caller
            // pre-scales the base roll through runtime.lastSkillPlayCode param.
        {
            int sparkCount = (int)((runtime ? runtime->lastSkillPlayCode : 0) >> 12) & 0xF;
            if(sparkCount <= 0)
                sparkCount = std::max(1, (M_Spark_Num[p] / 2) + 1);
            damage += (damage * M_Spark_Damage_Percent[p] * sparkCount) / 100;
            break;
        }
        case SKILL_HOLY_BOLT:
            if(HolyBolt_DamagePercent[p] > 0)
                damage += (damage * HolyBolt_DamagePercent[p]) / 100;
            else
                damage += RandomBetween(HolyBolt_Damage[p][0], HolyBolt_Damage[p][1]);
            break;
        case SKILL_FIRE_BOLT:
            if(FireBolt_DamagePercent[p] > 0)
                damage += (damage * FireBolt_DamagePercent[p]) / 100;
            else
                damage += RandomBetween(FireBolt_Damage[p][0], FireBolt_Damage[p][1]);
            damage += caster->smCharInfo.Level / 3;
            break;
        case SKILL_FIRE_BALL:
            damage += (damage * FireBall_DamagePercent[p]) / 100;
            break;
        case SKILL_WATORNADO:
            damage += (damage * Watornado_DamagePercent[p]) / 100;
            break;
        case SKILL_AVANGING_CRASH:
            damage += (damage * A_Crash_Damage[p]) / 100;
            break;
        case SKILL_GRAND_SMASH:
            damage += (damage * Grand_Smash_Damage[p]) / 100;
            break;
        case SKILL_PERFECT_AIM:
            damage += (damage * Perfect_Aim_DamagePercent[p]) / 100;
            damage += caster->smCharInfo.Level / std::max(1, Perfect_Aim_Damage_LV[p]);
            if(target && target->smCharInfo.Brood == smCHAR_MONSTER_DEMON)
                damage += (damage * 30) / 100;
            break;
        case SKILL_CHAIN_LANCE:
            damage += (damage * Chain_Lance_Damage[p]) / 100;
            break;
        case SKILL_CHARGING_STRIKE:
            // Companion AI deterministically completes the charge; legacy
            // Power2[0] == 0 selects the charged damage table.
            damage += (damage * Charging_Strike_CharingDamagePercent[p]) / 100;
            break;
        case SKILL_FIRE_JAVELIN:
            damage += (damage * Fire_Javelin_DamagePercent[p]) / 100;
            if(target && target->smCharInfo.Brood == smCHAR_MONSTER_MUTANT)
                damage += (damage * 50) / 100;
            break;
        case SKILL_SPLIT_JAVELIN:
        {
            // Legacy sinGetAccuracy() currently returns 100 unconditionally;
            // therefore every configured Split Javelin projectile connects.
            // Preserve the server policy: each successful projectile rolls base
            // damage independently and applies Split_Javelin_Damage percent.
            const int hits = std::max(1, Split_Javelin_AttackNum[p]);
            damage = 0;
            for(int hit = 0; hit < hits; ++hit)
            {
                int roll = RandomBetween(caster->smCharInfo.Attack_Damage[0], caster->smCharInfo.Attack_Damage[1]);
                if(roll < 1) roll = 1;
                roll += (roll * Split_Javelin_Damage[p]) / 100;
                damage += roll;
            }
            break;
        }
        case SKILL_DOUBLE_CRASH:
            damage += (damage * D_Crash_Damage[p]) / 100;
            break;
        case SKILL_FLAME_WAVE:
            damage += (damage * Flame_Wave_FireDamage_Percent[p]) / 100;
            damage += caster->smCharInfo.Level / 3;
            break;
        case SKILL_DEAD_RAY:
            damage += (damage * Dead_Ray_Damage_Percent[p]) / 100;
            damage += caster->smCharInfo.Level / 3;
            break;
        case SKILL_IMPULSION:
            damage += (damage * Impulsion_LightingDamagePercent[p]) / 100;
            if(target && target->smCharInfo.Brood == smCHAR_MONSTER_MECHANIC)
                damage += (damage * 30) / 100;
            break;
        case SKILL_RAGE_OF_ZECRAM:
            damage += (damage * R_Zecram_Damage[p]) / 100;
            break;
        case SKILL_BONE_CRASH:
            damage += (damage * B_Crash_Damage[p]) / 100;
            if(target && target->smCharInfo.Brood == smCHAR_MONSTER_DEMON)
                damage += (damage * B_Crash_DemonDamage[p]) / 100;
            break;
        case SKILL_DETORYER:
            damage += (damage * Destoryer_DamagePercent[p]) / 100;
            if(target && target->smCharInfo.Brood == smCHAR_MONSTER_DEMON)
                damage += (damage * 30) / 100;
            break;
        case SKILL_CYCLONE_STRIKE:
            damage += (damage * Cyclone_Strike_DamagePercent[p]) / 100;
            break;
        case SKILL_TORNADO:
            damage += (damage * Tornado_Damage[p]) / 100;
            break;
        case SKILL_EXPANSION:
            damage += (damage * Expansion_Damage[p]) / 100;
            damage += (damage * (5 * point)) / 100;
            break;
        case SKILL_VENOM_SPEAR:
            // Legacy range packet uses half of the base power for the initial hit.
            damage = std::max(1, damage >> 1);
            break;
        case SKILL_SHADOW_MASTER:
            damage += (damage * Shadow_Master_DamagePercent[p]) / 100;
            break;
        case SKILL_ELEMENTAL_SHOT:
            damage += (damage * Elemental_ShotPercent[p]) / 100;
            break;
        case SKILL_PHOENIX_SHOT:
            damage += (damage * Phoenix_Shot_DamagePercent[p]) / 100;
            break;
        case SKILL_BRANDISH:
            damage += (damage * Brandish_Damage[p]) / 100;
            break;
        case SKILL_PIERCING:
            damage += (damage * Piercing_Damage[p]) / 100;
            break;
        case SKILL_GRAND_CROSS:
            damage += (damage * Grand_Cross_Damage[p]) / 100;
            if(target && target->smCharInfo.Brood == smCHAR_MONSTER_UNDEAD)
                damage += (damage * Grand_Cross_UndeadDamage[p]) / 100;
            break;
        case SKILL_SWORD_OF_JUSTICE:
            damage += (damage * Sword_Of_Justice_DamagePercent[p]) / 100;
            break;
        case SKILL_DIVINE_PIERCING:
            damage += (damage * Divine_Piercing_DamagePercent[p]) / 100;
            break;
        case SKILL_SHIELD_STRIKE:
            damage += (damage * S_Strike_DamagePercent[p]) / 100;
            break;
        case SKILL_FARINA:
            // Legacy Farina keeps the normal weapon damage roll; its special
            // behavior is temporary Attack_Rating and a faster attack loop.
            break;
        case SKILL_VIGOR_SPEAR:
            damage += (damage * V_Spear_DamagePercent[p]) / 100;
            break;
        case SKILL_LIGHTNING_JAVELIN:
            damage += (damage * Lightning_Javelin_DamagePercent[p]) / 100;
            if(target && target->smCharInfo.Brood == smCHAR_MONSTER_UNDEAD)
                damage += damage / 2;
            break;
        case SKILL_STORM_JAVELIN:
            damage += (damage * Storm_Javelin_Damage[p]) / 100;
            break;
        case SKILL_X_RAGE:
            damage += (damage * X_Rage_DamagePercent[p]) / 100;
            break;
        case SKILL_VENGEANCE:
            damage += (damage * Vengeance_DamagePercent[p]) / 100;
            break;

        case SKILL_GLACIAL_SPIKE:
            damage += (damage * Glacial_Spike_Damage_Percent[p]) / 100;
            break;
        case SKILL_DIVINE_LIGHTNING:
            damage += (damage * Divine_Lightning_Damage_Percent[p]) / 100;
            break;
        case SKILL_VIGOR_BALL:
            damage += (damage * Vigor_Ball_Damage_Percent[p]) / 100;
            break;
        case SKILL_CHAIN_LIGHTNING:
            damage += (damage * Chain_Lightning_Damage_Percent[p]) / 100;
            break;
        case SKILL_DIASTROPHISM:
            damage += (damage * Diastrophism_Damage_Percent[p]) / 100;
            damage += caster->smCharInfo.Level / 3;
            break;
        case SKILL_M_METEO:
            damage += (damage * Meteo_Damage_Percent[p]) / 100;
            damage += caster->smCharInfo.Level / 3;
            break;
        case SKILL_T5_RUPTURE:
            damage = (damage + (damage * Rupture_DamageBoost[p]) / 100) * 2;
            break;
        case SKILL_T5_PARASIT_SHOT:
            damage += (damage * Parasit_Shot_DamageBoost[p]) / 100;
            break;
        case SKILL_T5_MAGNETIC_DISCHARGE:
            damage += (damage * Magnetic_Discharge_DamageBoost[p]) / 100;
            break;
        case SKILL_T5_SEISMIC_IMPACT:
            damage += (damage * Fighter_T5_Seismic_Damage[p]) / 100;
            break;
        case SKILL_T5_WHIRLWIND:
            damage += (damage * Fighter_T5_Whirlwind_DamageBoost[p]) / 100;
            break;
        case SKILL_T5_TWISTER:
            damage += (damage * Pike_T5_Twister_DamageBoost[p]) / 100;
            break;
        case SKILL_T5_FIERCE_WIND:
            damage += (damage * Archer_T5_Fierce_DamageBoost[p]) / 100;
            break;
        case SKILL_T5_BOMBARDMENT:
            damage += (damage * Archer_T5_Bombard_DamageBoost[p]) / 100;
            break;
        case SKILL_T5_DIVINE_INQUISITON:
            damage += (damage * Knight_T5_DivineInquisition_DamageBoost[p]) / 100;
            if(target && target->smCharInfo.Brood == smCHAR_MONSTER_UNDEAD)
                damage += (damage * Knight_T5_DivineInquisition_UndeadDamage[p]) / 100;
            break;
        case SKILL_T5_DIVINE_CROSS:
            damage += (damage * Knight_T5_DivineCross_DamageBoost[p]) / 100;
            break;
        case SKILL_T5_PLAGUE_JAVELIN:
            damage += (damage * Atalanta_T5_Plague_DamageBoost[p]) / 100;
            break;
        case SKILL_T5_COMBO_JAVELIN:
            damage = (damage + (damage * Atalanta_T5_Combo_DamageBoost[p]) / 100) * 3;
            break;
        case SKILL_T5_LIGHTNING_SURGE:
            damage += (damage * Priestess_T5_LightningSurge_DamageBoost[p]) / 100;
            break;
        case SKILL_T5_RED_RAY:
            damage += (damage * Magician_T5_RedRay_DamageBoost[p]) / 100;
            break;
        case SKILL_T5_CATACLYSM:
            damage += (damage * Magician_T5_Cataclysm_DamageBoost[p]) / 100;
            break;
        default:
            return 0;
    }
    if(runtime)
    {
        const int genericPercent = ActiveGenericDamagePercent(*runtime);
        if(genericPercent > 0)
            damage += (damage * genericPercent) / 100;
        if(IsMagicDamageSkillCode(skillCode))
        {
            const int magicPercent = ActiveMagicDamagePercent(*runtime);
            if(magicPercent > 0)
                damage += (damage * magicPercent) / 100;
        }
        if(target && target->smCharInfo.Brood == smCHAR_MONSTER_UNDEAD)
        {
            const int undeadPercent = ActiveUndeadDamagePercent(*runtime);
            if(undeadPercent > 0)
                damage += (damage * undeadPercent) / 100;
        }
        const int criticalPercent = ActiveCriticalChancePercent(*runtime);
        if(criticalPercent > 0 && (rand() % 100) < criticalPercent)
            damage += damage / 2;
    }
    // God Bless is a post-formula flat bonus in the legacy server policy, so
    // percentage skills must not multiply it by baking it into Attack_Damage.
    if(runtime && includePostRuntimeModifiers)
    {
        damage += ActiveOutgoingFlatDamage(*runtime);
        damage += ActiveForceNatureDamage(*runtime, dwPlayServTime);
        if(!IsMagicDamageSkillCode(skillCode))
            damage = ApplyEnchantWeaponBonus(*runtime, target, damage, dwPlayServTime);
    }
    return damage;
}

static bool ApplySkillDamage(smCHAR *caster, rsPLAYINFO *owner, smCHAR *target, int rawDamage)
{
    if(!caster || !owner || !target)
        return false;

    const PlayerId ownerId = player::legacy::ServerLegacyOnlinePlayerRegistry().IdOf(owner);
    const combat::DamageRequest request(EntityId(static_cast<std::uint32_t>(caster->dwObjectSerial)),
                                        ownerId,
                                        EntityId(static_cast<std::uint32_t>(target->dwObjectSerial)),
                                        rawDamage);
    const combat::DamageResult result = combat::legacy::ServerLegacyDamageAdapter().Apply(request);
    return result.Accepted();
}

static void ApplyCompanionTargetSideEffects(smCHAR *target, DWORD skillCode, int point, int rawDamage)
{
    if(!target || point < 1 || point > 10)
        return;
    const int p = point - 1;
    switch(skillCode)
    {
        case SKILL_VENOM_SPEAR:
            combat::legacy::ServerLegacyStatusEffectAdapter().ApplyPoison(
                EntityId(static_cast<std::uint32_t>(target->dwObjectSerial)),
                VenomSpear_Time[p] * 17, (rawDamage * VenomSpear_DamagePercent[p]) / 100);
            break;
        case SKILL_ROAR:
        {
            int resistance = std::max(0, std::min(100, (int)target->smCharInfo.Resistance[sITEMINFO_BIONIC]));
            combat::legacy::ServerLegacyStatusEffectAdapter().ApplyStun(
                EntityId(static_cast<std::uint32_t>(target->dwObjectSerial)),
                ((Roar_Time[p] << 4) * (100 - resistance)) / 100);
            break;
        }
        case SKILL_SHIELD_STRIKE:
        {
            int resistance = std::max(0, std::min(100, (int)target->smCharInfo.Resistance[sITEMINFO_BIONIC]));
            combat::legacy::ServerLegacyStatusEffectAdapter().ApplyStun(
                EntityId(static_cast<std::uint32_t>(target->dwObjectSerial)),
                ((10 << 4) * (100 - resistance)) / 100);
            break;
        }
        case SKILL_ASSASSIN_EYE:
            combat::legacy::ServerLegacyStatusEffectAdapter().ApplyAssassinEye(
                EntityId(static_cast<std::uint32_t>(target->dwObjectSerial)),
                static_cast<std::uint32_t>(dwPlayServTime + (DWORD)Assassin_Eye_Time[p] * 1000U),
                Assassin_Eye_M_SubCritical[p]);
            break;
        case SKILL_HOLY_MIND:
        {
            int resistance = std::max(0, std::min(100, (int)target->smCharInfo.Resistance[sITEMINFO_BIONIC]));
            combat::legacy::ServerLegacyStatusEffectAdapter().ApplyHolyMind(
                EntityId(static_cast<std::uint32_t>(target->dwObjectSerial)), HolyMind_DecDamage[p],
                ((HolyMind_Time[p] << 4) * (100 - resistance)) / 100);
            break;
        }
        case SKILL_DISTORTION:
        {
            const int time = Distortion_Time[p] * 17;
            combat::legacy::ServerLegacyStatusEffectAdapter().ApplySlow(
                EntityId(static_cast<std::uint32_t>(target->dwObjectSerial)),
                combat::SlowEffect(time, 240 - (240 * Distortion_SpeedSubPercent[p]) / 100,
                                   time, false, false, true));
            break;
        }
        case SKILL_EXTINCTION:
            if(target->smCharInfo.Brood == smCHAR_MONSTER_UNDEAD)
            {
                int resistance = std::max(-100, std::min(100, (int)target->smCharInfo.Resistance[sITEMINFO_BIONIC]));
                int chance = resistance == 100 ? 0 : Extinction_Percent[p] + target->smCharInfo.Level / 4;
                if((rand() % 100) < std::max(0, std::min(100, chance)))
                {
                    const int life = std::max(1, ((int)target->smCharInfo.Life[0] * Extinction_Amount[p]) / 100);
                    target->smCharInfo.Life[0] = (short)std::max(0, (int)target->smCharInfo.Life[0] - life);
                    if(target->smCharInfo.Life[0] <= 0)
                        target->SetMotionFromCode(CHRMOTION_STATE_DEAD);
                }
            }
            break;
        case SKILL_T5_PARASIT_SHOT:
            combat::legacy::ServerLegacyStatusEffectAdapter().ApplyPoison(
                EntityId(static_cast<std::uint32_t>(target->dwObjectSerial)),
                Parasit_Shot_PoisonDuration[p] * 17,
                (rawDamage * Parasit_Shot_PoisonDamagePercent[p]) / 100);
            break;
        case SKILL_T5_PLAGUE_JAVELIN:
            combat::legacy::ServerLegacyStatusEffectAdapter().ApplyPoison(
                EntityId(static_cast<std::uint32_t>(target->dwObjectSerial)),
                Atalanta_T5_Plague_PoisonDuration[p] * 17,
                (rawDamage * Atalanta_T5_Plague_PoisonDamagePercent[p]) / 100);
            break;
        case SKILL_T5_RING_OF_SPEARS:
        {
            int resistance = std::max(0, std::min(100, (int)target->smCharInfo.Resistance[sITEMINFO_BIONIC]));
            const int stun = Pike_T5_Ring_Duration[p] << 4;
            combat::legacy::ServerLegacyStatusEffectAdapter().ApplyStun(
                EntityId(static_cast<std::uint32_t>(target->dwObjectSerial)),
                (stun * (100 - resistance)) / 100);
            break;
        }
        case SKILL_T5_ENTALING_ROOTS:
            combat::legacy::ServerLegacyStatusEffectAdapter().ApplyStun(
                EntityId(static_cast<std::uint32_t>(target->dwObjectSerial)),
                Archer_T5_Roots_Duration[p] << 4);
            break;
        case SKILL_T5_HEAVENLY_LIGHT:
            combat::legacy::ServerLegacyStatusEffectAdapter().ApplyHeavenlyLight(
                EntityId(static_cast<std::uint32_t>(target->dwObjectSerial)),
                Priestess_T5_HeavenlyLight_AttackRateReduction[p],
                Priestess_T5_HeavenlyLight_Duration[p] << 4);
            break;
        default:
            break;
    }
}

static bool ApplyCompanionSkillImpact(smCHAR *caster, rsPLAYINFO *owner, smCHAR *target,
    DWORD skillCode, int point, int rawDamage)
{
    if(!ApplySkillDamage(caster, owner, target, rawDamage))
        return false;
    if(skillCode == SKILL_SOUL_SUCKER && caster->smCharInfo.Life[0] > 0 && caster->smCharInfo.Life[1] > 0)
        combat::legacy::ServerLegacyLifeAdapter().Restore(EntityId(static_cast<std::uint32_t>(caster->dwObjectSerial)), rawDamage);
    ApplyCompanionTargetSideEffects(target, skillCode, point, rawDamage);
    return true;
}

static int CompanionSkillAreaRadius(DWORD skillCode, int point)
{
    if(point < 1 || point > 10)
        return 0;
    const int p = point - 1;
    switch(skillCode)
    {
        case SKILL_ROAR: return std::max(1, Roar_Range[p]);
        case SKILL_GROUND_PIKE: return std::max(1, G_Pike_Range[p]);
        case SKILL_VENOM_SPEAR: return std::max(1, VenomSpear_Range[p]);
        case SKILL_SOUL_SUCKER: return 180;
        case SKILL_EXTINCTION: return 240;
        case SKILL_GLACIAL_SPIKE: return std::max(1, Glacial_Spike_Area[p]);
        case SKILL_DISTORTION: return std::max(1, Distortion_Area[p]);
        case SKILL_BOMB_SHOT: return std::max(1, BombShot_Area[p]);
        case SKILL_MECHANIC_BOMB: return std::max(1, Mechanic_Bomb_Attack_Range[p]);
        case SKILL_SPARK: return std::max(1, Spark_Area[p]);
        case SKILL_ARROW_OF_RAGE: return std::max(1, A_Rage_Area[p]);
        case SKILL_FIRE_BALL: return std::max(1, FireBall_Area[p]);
        case SKILL_WATORNADO: return std::max(1, Watornado_Area[p]);
        case SKILL_IMPULSION: return std::max(1, Impulsion_Range[p]);
        case SKILL_RAGE_OF_ZECRAM: return 70;
        case SKILL_CYCLONE_STRIKE: return std::max(1, Cyclone_Strike_Area[p]);
        case SKILL_TORNADO: return std::max(1, Tornado_Range[p]);
        case SKILL_EXPANSION: return 180;
        case SKILL_BRANDISH: return std::max(1, Brandish_Range[p]);
        case SKILL_LIGHTNING_JAVELIN: return 100;
        case SKILL_X_RAGE: return std::max(1, X_Rage_Area[p]);
        case SKILL_DIVINE_LIGHTNING: return 320;
        case SKILL_CHAIN_LIGHTNING: return std::max(1, Chain_Lightning_Range[p] * Chain_Lightning_Num[p]);
        case SKILL_DIASTROPHISM: return std::max(1, Diastrophism_Area[p]);
        case SKILL_M_METEO: return std::max(1, Meteo_Area[p]);
        case SKILL_T5_MAGNETIC_DISCHARGE: return std::max(1, Impulsion_Range[p]);
        case SKILL_T5_SEISMIC_IMPACT: return std::max(1, Fighter_T5_Seismic_Area[p]);
        case SKILL_T5_WHIRLWIND: return std::max(1, Cyclone_Strike_Area[p]);
        case SKILL_T5_TWISTER: return std::max(1, Tornado_Range[p]);
        case SKILL_T5_FIERCE_WIND: return 240;
        case SKILL_T5_DIVINE_INQUISITON: return std::max(1, Knight_T5_DivineInquisition_Area[p]);
        case SKILL_T5_PLAGUE_JAVELIN: return 180;
        case SKILL_T5_LIGHTNING_SURGE: return std::max(1, Priestess_T5_LightningSurge_Area[p]);
        default: return 0;
    }
}

static int CompanionSkillBoxHalfWidth(DWORD skillCode, int point)
{
    if(point < 1 || point > 10) return 0;
    switch(skillCode)
    {
        case SKILL_SHIELD_STRIKE: return 50;
        case SKILL_SOUL_SUCKER: return 30;
        case SKILL_GLACIAL_SPIKE: return 50;
        case SKILL_FLAME_WAVE: return 60;
        case SKILL_PERFORATION: return 40;
        case SKILL_PIERCING: return 40;
        case SKILL_STORM_JAVELIN: return 30;
        case SKILL_SWORD_OF_JUSTICE: return 70;
        default: return 0;
    }
}

static int CompanionSkillBoxForwardLength(const smCHAR *caster, DWORD skillCode, int point)
{
    if(point < 1 || point > 10) return 0;
    const int p = point - 1;
    switch(skillCode)
    {
        case SKILL_SHIELD_STRIKE: return 160;
        case SKILL_SOUL_SUCKER: return 200;
        case SKILL_GLACIAL_SPIKE: return 340;
        case SKILL_FLAME_WAVE: return 340;
        case SKILL_PERFORATION:
            return std::max(1, (caster ? caster->smCharInfo.Shooting_Range : 300) + Perforation_Attack_Range[p] + 40);
        case SKILL_PIERCING: return 140;
        case SKILL_STORM_JAVELIN: return 340;
        case SKILL_SWORD_OF_JUSTICE: return std::max(1, Sword_Of_Justice_Area[p] + 40);
        default: return 0;
    }
}

static bool IsInsideCompanionForwardBox(const smCHAR *caster, const smCHAR *aimTarget,
    const smCHAR *candidate, int halfWidth, int forwardLength)
{
    if(!caster || !aimTarget || !candidate || halfWidth <= 0 || forwardLength <= 0)
        return false;
    const long long fx = (aimTarget->pX - caster->pX) >> FLOATNS;
    const long long fz = (aimTarget->pZ - caster->pZ) >> FLOATNS;
    const long long len2 = fx * fx + fz * fz;
    if(len2 <= 0)
        return candidate == aimTarget;
    const long long vx = (candidate->pX - caster->pX) >> FLOATNS;
    const long long vz = (candidate->pZ - caster->pZ) >> FLOATNS;
    if(abs((candidate->pY - caster->pY) >> FLOATNS) > 128)
        return false;
    const long long dot = vx * fx + vz * fz;
    if(dot < 0)
        return false;
    if(dot * dot > (long long)forwardLength * forwardLength * len2)
        return false;
    const long long cross = vx * fz - vz * fx;
    return cross * cross <= (long long)halfWidth * halfWidth * len2;
}

static int CompanionSkillMaxTargets(DWORD skillCode, int point)
{
    if(point < 1 || point > 10) return 0;
    const int p = point - 1;
    switch(skillCode)
    {
        case SKILL_VENOM_SPEAR: return std::max(1, VenomSpear_Num[p]);
        case SKILL_MULTISPARK: return std::max(1, M_Spark_Num[p]);
        case SKILL_IMPULSION: return std::max(1, Impulsion_LightingNum[p] + 1);
        case SKILL_CYCLONE_STRIKE: return std::max(1, Cyclone_Strike_AttackNum[p] + 1);
        case SKILL_DIVINE_LIGHTNING: return std::max(1, Divine_Lightning_Num[p]);
        case SKILL_CHAIN_LIGHTNING: return std::max(1, Chain_Lightning_Num[p]);
        case SKILL_T5_MAGNETIC_DISCHARGE: return std::max(1, Impulsion_LightingNum[p] + 1);
        case SKILL_T5_WHIRLWIND: return std::max(1, Cyclone_Strike_AttackNum[p] + 1);
        case SKILL_T5_PLAGUE_JAVELIN: return std::max(1, Atalanta_T5_Plague_Targets[p]);
        default: return 0;
    }
}

static int ClampLegacyResistance(int resistance)
{
    if(resistance > 100) return 100;
    if(resistance < -100) return -100;
    return resistance;
}

static int ApplyLegacyResistance(int rawDamage, const smCHAR *target, int resistanceIndex)
{
    if(!target || rawDamage <= 0)
        return rawDamage;
    const int resistance = ClampLegacyResistance(target->smCharInfo.Resistance[resistanceIndex]);
    return rawDamage - (rawDamage * resistance) / 100;
}

static bool ApplyDirectLifeLoss(smCHAR *caster, rsPLAYINFO *owner, smCHAR *target, int damage)
{
    if(!caster || !owner || !target || !target->Flag || target->smCharInfo.Life[0] <= 0 || damage <= 0)
        return false;
    if(target->smCharInfo.State != smCHAR_STATE_ENEMY || target->smCharInfo.Brood == smCHAR_MONSTER_USER || target->lpMasterPlayInfo)
        return false;
    target->smCharInfo.Life[0] -= damage;
    if(target->smCharInfo.Life[0] <= 0)
    {
        target->smCharInfo.Life[0] = 0;
        target->SetMotionFromCode(CHRMOTION_STATE_DEAD);
        target->lpExt2 = owner;
        target->lpExt3 = caster;
    }
    else if(target->MotionInfo && target->MotionInfo->State < 0x100)
    {
        target->SetMotionFromCode(CHRMOTION_STATE_DAMAGE);
    }
    return true;
}

static bool IsLegacyPushImmuneBoss(const smCHAR *target)
{
    if(!target) return true;
    const DWORD code = target->smCharInfo.dwCharSoundCode;
    return code == snCHAR_SOUND_DEATHKNIGHT || code == snCHAR_SOUND_BABEL ||
        code == snCHAR_SOUND_KELVEZU || code == snCHAR_SOUND_MOKOVA;
}

static void PushTargetAwayFromCaster(smCHAR *caster, smCHAR *target, int distance)
{
    if(!caster || !target || distance <= 0 || IsLegacyPushImmuneBoss(target) || target->smCharInfo.Life[0] <= 0)
        return;
    const world::MovementCommandResult moved = world::legacy::ServerLegacyMovementAdapter().PushAway(
        EntityId(static_cast<std::uint32_t>(target->dwObjectSerial)),
        EntityId(static_cast<std::uint32_t>(caster->dwObjectSerial)), distance);
    if(moved.Ok())
        target->lpTargetPlayInfo = NULL;
}

static bool ApplyShieldStrikeToTarget(smCHAR *caster, rsPLAYINFO *owner, smCHAR *target, int point,
    const CompanionSkillRuntime &runtime)
{
    if(!caster || !target || point < 1 || point > 10) return false;
    const int raw = DamageForSkill(caster, target, SKILL_SHIELD_STRIKE, point, &runtime);
    if(!ApplySkillDamage(caster, owner, target, raw)) return false;
    int resistance = target->smCharInfo.Resistance[sITEMINFO_BIONIC];
    if(resistance < 0) resistance = 0;
    if(resistance > 100) resistance = 100;
    combat::legacy::ServerLegacyStatusEffectAdapter().ApplyStun(
        EntityId(static_cast<std::uint32_t>(target->dwObjectSerial)),
        (16 * 10 * (100 - resistance)) / 100);
    PushTargetAwayFromCaster(caster, target, 10);
    return true;
}

static bool ApplyStatusEffectToTarget(smCHAR *caster, rsPLAYINFO *owner, smCHAR *target,
    DWORD skillCode, int point, const CompanionSkillRuntime &runtime)
{
    if(!caster || !owner || !target || point < 1 || point > 10 || !target->Flag || target->smCharInfo.Life[0] <= 0)
        return false;
    if(target->smCharInfo.State != smCHAR_STATE_ENEMY || target->smCharInfo.Brood == smCHAR_MONSTER_USER || target->lpMasterPlayInfo)
        return false;
    const int p = point - 1;

    if(skillCode == SKILL_HOLY_MIND)
    {
        int resistance = target->smCharInfo.Resistance[sITEMINFO_BIONIC];
        if(resistance < 0) resistance = 0;
        if(resistance > 100) resistance = 100;
        const int time = (15 << 4) * (100 - resistance) / 100;
        if(time > 0)
            combat::legacy::ServerLegacyStatusEffectAdapter().ApplyHolyMind(
                EntityId(static_cast<std::uint32_t>(target->dwObjectSerial)),
                HolyMind_DecDamage[p], time);
        // Full Bionic resistance cancels the debuff, but the legacy cast still
        // completes and consumes its resources.
        return true;
    }

    if(skillCode == SKILL_ASSASSIN_EYE)
    {
        combat::legacy::ServerLegacyStatusEffectAdapter().ApplyAssassinEye(
            EntityId(static_cast<std::uint32_t>(target->dwObjectSerial)),
            static_cast<std::uint32_t>(dwPlayServTime + (DWORD)Assassin_Eye_Time[p] * 1000U),
            Assassin_Eye_M_SubCritical[p]);
        return true;
    }

    if(skillCode == SKILL_HOLY_INCANTATION)
    {
        monsters::MonsterSnapshot monster;
        if(!monsters::legacy::ServerLegacyMonsterAdapter().TryGetMonster(
               EntityId(static_cast<std::uint32_t>(target->dwObjectSerial)), monster) ||
           monster.family != monsters::MonsterFamily::Undead)
            return false;
        int chance = H_Incantation_Success[p];
        int resistance = ClampLegacyResistance(monster.bionicResistance);
        if(resistance != 0)
        {
            if(resistance == 100)
                chance = 0;
            else
                chance += caster->smCharInfo.Level / 4;
        }
        // A resisted/failed roll is still a valid legacy cast: resources and
        // cooldown are consumed even though the conversion does not land.
        if(chance <= 0 || (rand() % 100) >= chance)
            return true;

        const monsters::FollowerConversionResult conversion = monsters::legacy::ServerLegacyMonsterAdapter().ConvertToFollower(
            monsters::FollowerConversionRequest(
                EntityId(static_cast<std::uint32_t>(target->dwObjectSerial)),
                PlayerId(static_cast<std::uint32_t>(owner->dwObjectSerial)),
                H_Incantation_Time[p] * 17, H_Incantation_AddLife[p]));
        return conversion.Ok();
    }

    if(skillCode == SKILL_ROAR)
    {
        int resistance = target->smCharInfo.Resistance[sITEMINFO_BIONIC];
        if(resistance < 0) resistance = 0;
        if(resistance > 100) resistance = 100;
        const int stun = (Roar_Time[p] << 4) * (100 - resistance) / 100;
        combat::legacy::ServerLegacyStatusEffectAdapter().ApplyStun(
            EntityId(static_cast<std::uint32_t>(target->dwObjectSerial)), stun);
        return stun > 0;
    }

    if(skillCode == SKILL_DISTORTION)
    {
        const int time = Distortion_Time[p] * 17;
        combat::legacy::ServerLegacyStatusEffectAdapter().ApplySlow(
            EntityId(static_cast<std::uint32_t>(target->dwObjectSerial)),
            combat::SlowEffect(time, 240 - (240 * Distortion_SpeedSubPercent[p]) / 100,
                               time, true, false, true));
        return time > 0;
    }

    if(skillCode == SKILL_EXTINCTION)
    {
        monsters::MonsterSnapshot monster;
        if(!monsters::legacy::ServerLegacyMonsterAdapter().TryGetMonster(
               EntityId(static_cast<std::uint32_t>(target->dwObjectSerial)), monster) ||
           monster.family != monsters::MonsterFamily::Undead)
            return false;
        int chance = Extinction_Percent[p];
        int resistance = ClampLegacyResistance(monster.bionicResistance);
        if(resistance != 0)
        {
            if(resistance == 100)
                chance = 0;
            else
                chance += caster->smCharInfo.Level / 4;
        }
        combat::legacy::ServerLegacyStatusEffectAdapter().ClearStatus(
            EntityId(static_cast<std::uint32_t>(target->dwObjectSerial)), combat::StatusEffectKind::Stun);
        if(chance <= 0 || (rand() % 100) >= chance)
            return false;
        const int damage = (monster.life * Extinction_Amount[p]) / 100;
        return ApplyDirectLifeLoss(caster, owner, target, damage);
    }

    int damage = DamageForSkill(caster, target, skillCode, point, &runtime);
    if(skillCode == SKILL_VENOM_SPEAR)
    {
        const int poisonBase = damage;
        damage = ApplyLegacyResistance(damage, target, sITEMINFO_POISON);
        combat::legacy::ServerLegacyStatusEffectAdapter().ApplyPoison(
            EntityId(static_cast<std::uint32_t>(target->dwObjectSerial)),
            VenomSpear_Time[p] * 17,
            std::max(0, (poisonBase * VenomSpear_DamagePercent[p]) / 100));
        return ApplySkillDamage(caster, owner, target, damage);
    }
    if(skillCode == SKILL_GROUND_PIKE || skillCode == SKILL_GLACIAL_SPIKE)
    {
        damage = ApplyLegacyResistance(damage, target, sITEMINFO_ICE);
        const int resistance = std::max(0, ClampLegacyResistance(target->smCharInfo.Resistance[sITEMINFO_ICE]));
        int time = skillCode == SKILL_GLACIAL_SPIKE ? (8 * 17) : (G_Pike_Time[p] * 17);
        const int slowTime = std::max(0, time - (time * resistance) / 100);
        combat::legacy::ServerLegacyStatusEffectAdapter().ApplySlow(
            EntityId(static_cast<std::uint32_t>(target->dwObjectSerial)),
            combat::SlowEffect(slowTime, skillCode == SKILL_GLACIAL_SPIKE ? 200 : 0,
                               0, true, true, true));
        return ApplySkillDamage(caster, owner, target, damage);
    }
    return false;
}

static bool ApplyPikeWind(smCHAR *caster, rsPLAYINFO *owner, smCHAR *primary, int point,
    const CompanionSkillRuntime &runtime)
{
    STG_AREA *area = caster ? static_cast<STG_AREA *>(caster->lpExt4) : NULL;
    if(!area || !primary || point < 1 || point > 10) return false;
    const int radius = std::max(1, Pike_Wind_Push_Lenght[point - 1]);
    const int radius2 = radius * radius;
    bool applied = false;
    for(int i = -1; i < area->MonsterLastPoint; ++i)
    {
        smCHAR *candidate = i < 0 ? primary : area->lpCharMonster[i];
        if(!candidate || (i >= 0 && candidate == primary) || !candidate->Flag || candidate->smCharInfo.Life[0] <= 0) continue;
        if(candidate->smCharInfo.State != smCHAR_STATE_ENEMY || candidate->smCharInfo.Brood == smCHAR_MONSTER_USER || candidate->lpMasterPlayInfo) continue;
        const int dx = (candidate->pX - caster->pX) >> FLOATNS;
        const int dz = (candidate->pZ - caster->pZ) >> FLOATNS;
        const int dist2 = dx * dx + dz * dz;
        if(abs((candidate->pY - caster->pY) >> FLOATNS) > 100 || dist2 > radius2) continue;
        if(ApplySkillDamage(caster, owner, candidate, DamageForSkill(caster, candidate, SKILL_PIKE_WIND, point, &runtime)))
        {
            const int dist = (int)sqrt((double)dist2);
            PushTargetAwayFromCaster(caster, candidate, std::max(0, radius - dist));
            combat::legacy::ServerLegacyStatusEffectAdapter().ClearStatus(
                EntityId(static_cast<std::uint32_t>(candidate->dwObjectSerial)), combat::StatusEffectKind::Stun);
            applied = true;
        }
    }
    return applied;
}

static bool ApplyShieldStrike(smCHAR *caster, rsPLAYINFO *owner, smCHAR *primary, int point,
    const CompanionSkillRuntime &runtime)
{
    STG_AREA *area = caster ? static_cast<STG_AREA *>(caster->lpExt4) : NULL;
    if(!area || !primary) return false;
    bool applied = false;
    if(IsInsideCompanionForwardBox(caster, primary, primary, 50, 160))
        applied = ApplyShieldStrikeToTarget(caster, owner, primary, point, runtime);
    for(int i = 0; i < area->MonsterLastPoint; ++i)
    {
        smCHAR *candidate = area->lpCharMonster[i];
        if(!candidate || candidate == primary || !candidate->Flag || candidate->smCharInfo.Life[0] <= 0) continue;
        if(!IsInsideCompanionForwardBox(caster, primary, candidate, 50, 160)) continue;
        if(ApplyShieldStrikeToTarget(caster, owner, candidate, point, runtime)) applied = true;
    }
    return applied;
}

static bool ApplySoulSucker(smCHAR *caster, rsPLAYINFO *owner, smCHAR *primary, int point)
{
    STG_AREA *area = caster ? static_cast<STG_AREA *>(caster->lpExt4) : NULL;
    if(!area || !primary || point < 1 || point > 10) return false;
    int totalLife = 0;
    bool applied = false;
    for(int i = -1; i < area->MonsterLastPoint; ++i)
    {
        smCHAR *candidate = i < 0 ? primary : area->lpCharMonster[i];
        if(!candidate || (i >= 0 && candidate == primary) || !candidate->Flag || candidate->smCharInfo.Life[0] <= 0) continue;
        if(candidate->smCharInfo.State != smCHAR_STATE_ENEMY || candidate->smCharInfo.Brood == smCHAR_MONSTER_USER || candidate->lpMasterPlayInfo) continue;
        if(!IsInsideCompanionForwardBox(caster, primary, candidate, 30, 200)) continue;
        if(((int)candidate->smCharInfo.Life[1] * 50) / 100 >= candidate->smCharInfo.Life[0]) continue;
        int absorb = Soul_Sucker_Absorb[point - 1];
        int bionic = ClampLegacyResistance(candidate->smCharInfo.Resistance[sITEMINFO_BIONIC]);
        if(bionic == 100) absorb = 0;
        if(absorb <= 0) continue;
        int life = ((int)candidate->smCharInfo.Life[1] * absorb) / 100;
        life = std::min(life, (int)candidate->smCharInfo.Life[0]);
        if(life <= 0) continue;
        if(ApplyDirectLifeLoss(caster, owner, candidate, life))
        {
            totalLife += life;
            applied = true;
        }
    }
    if(totalLife > 0)
        caster->smCharInfo.Life[0] = (short)std::min((int)caster->smCharInfo.Life[1], (int)caster->smCharInfo.Life[0] + totalLife);
    return applied;
}

static bool ApplyCompanionStatusSkill(smCHAR *caster, rsPLAYINFO *owner, smCHAR *primary,
    DWORD skillCode, int point, const CompanionSkillRuntime &runtime)
{
    if(!caster || !primary)
        return false;

    // These legacy effects are explicitly single-target. Do not let them fall
    // through the radial status iterator used by Ground Pike/Venom/etc.
    if(skillCode == SKILL_HOLY_MIND || skillCode == SKILL_ASSASSIN_EYE ||
       skillCode == SKILL_HOLY_INCANTATION)
        return ApplyStatusEffectToTarget(caster, owner, primary, skillCode, point, runtime);

    STG_AREA *area = static_cast<STG_AREA *>(caster->lpExt4);
    if(!area)
        return false;

    const int boxHalfWidth = CompanionSkillBoxHalfWidth(skillCode, point);
    const int boxForwardLength = CompanionSkillBoxForwardLength(caster, skillCode, point);
    bool applied = false;
    if(boxHalfWidth > 0 && boxForwardLength > 0)
    {
        if(IsInsideCompanionForwardBox(caster, primary, primary, boxHalfWidth, boxForwardLength))
            applied = ApplyStatusEffectToTarget(caster, owner, primary, skillCode, point, runtime);
        for(int i = 0; i < area->MonsterLastPoint; ++i)
        {
            smCHAR *candidate = area->lpCharMonster[i];
            if(candidate == primary || !candidate || !candidate->Flag || candidate->smCharInfo.Life[0] <= 0)
                continue;
            if(!IsInsideCompanionForwardBox(caster, primary, candidate, boxHalfWidth, boxForwardLength))
                continue;
            if(ApplyStatusEffectToTarget(caster, owner, candidate, skillCode, point, runtime))
                applied = true;
        }
        return applied;
    }

    const int radius = SkillRange(skillCode, caster, point);
    const int radiusSquared = radius * radius;
    const int maxTargets = skillCode == SKILL_VENOM_SPEAR ? std::max(1, VenomSpear_Num[point - 1]) : 0;
    int appliedTargets = 0;
    for(int i = -1; i < area->MonsterLastPoint; ++i)
    {
        smCHAR *candidate = i < 0 ? primary : area->lpCharMonster[i];
        if(!candidate || (i >= 0 && candidate == primary) || !candidate->Flag || candidate->smCharInfo.Life[0] <= 0)
            continue;
        const int dx = (candidate->pX - caster->pX) >> FLOATNS;
        const int dz = (candidate->pZ - caster->pZ) >> FLOATNS;
        if(abs((candidate->pY - caster->pY) >> FLOATNS) > 128 || dx * dx + dz * dz > radiusSquared)
            continue;
        if(maxTargets > 0 && appliedTargets >= maxTargets)
            break;
        if(ApplyStatusEffectToTarget(caster, owner, candidate, skillCode, point, runtime))
        {
            applied = true;
            ++appliedTargets;
        }
    }
    return applied;
}

static bool ApplyCompanionSkillDamage(smCHAR *caster, rsPLAYINFO *owner, smCHAR *primary,
    DWORD skillCode, int point, const CompanionSkillRuntime &runtime)
{
    const int boxHalfWidth = CompanionSkillBoxHalfWidth(skillCode, point);
    const int boxForwardLength = CompanionSkillBoxForwardLength(caster, skillCode, point);
    if(boxHalfWidth > 0 && boxForwardLength > 0)
    {
        STG_AREA *area = caster ? static_cast<STG_AREA *>(caster->lpExt4) : NULL;
        if(!area || !primary)
            return false;
        bool applied = false;
        if(IsInsideCompanionForwardBox(caster, primary, primary, boxHalfWidth, boxForwardLength))
            applied = ApplyCompanionSkillImpact(caster, owner, primary, skillCode, point,
                DamageForSkill(caster, primary, skillCode, point, &runtime));
        for(int i = 0; i < area->MonsterLastPoint; ++i)
        {
            smCHAR *candidate = area->lpCharMonster[i];
            if(candidate == primary || !candidate || !candidate->Flag || candidate->smCharInfo.Life[0] <= 0)
                continue;
            if(candidate->smCharInfo.State != smCHAR_STATE_ENEMY ||
               candidate->smCharInfo.Brood == smCHAR_MONSTER_USER || candidate->lpMasterPlayInfo)
                continue;
            if(!IsInsideCompanionForwardBox(caster, primary, candidate, boxHalfWidth, boxForwardLength))
                continue;
            if(ApplyCompanionSkillImpact(caster, owner, candidate, skillCode, point,
                DamageForSkill(caster, candidate, skillCode, point, &runtime)))
                applied = true;
        }
        return applied;
    }

    const int areaRadius = CompanionSkillAreaRadius(skillCode, point);
    if(areaRadius <= 0)
        return ApplyCompanionSkillImpact(caster, owner, primary, skillCode, point, DamageForSkill(caster, primary, skillCode, point, &runtime));

    STG_AREA *area = caster ? static_cast<STG_AREA *>(caster->lpExt4) : NULL;
    if(!area || !primary)
        return false;

    const int radiusSquared = areaRadius * areaRadius;

    // The selected target is authoritative for the cast. Apply it first so an
    // area-list refresh race cannot turn a valid cast into a visual-only cast.
    bool applied = ApplyCompanionSkillImpact(caster, owner, primary, skillCode, point,
        DamageForSkill(caster, primary, skillCode, point, &runtime));
    int appliedTargets = applied ? 1 : 0;
    const int maxTargets = CompanionSkillMaxTargets(skillCode, point);

    for(int i = 0; i < area->MonsterLastPoint; ++i)
    {
        smCHAR *candidate = area->lpCharMonster[i];
        if(candidate == primary)
            continue;
        if(!candidate || !candidate->Flag || candidate->smCharInfo.Life[0] <= 0)
            continue;
        if(candidate->smCharInfo.State != smCHAR_STATE_ENEMY ||
           candidate->smCharInfo.Brood == smCHAR_MONSTER_USER || candidate->lpMasterPlayInfo)
            continue;
        const int dx = (candidate->pX - primary->pX) >> FLOATNS;
        const int dz = (candidate->pZ - primary->pZ) >> FLOATNS;
        if(dx * dx + dz * dz > radiusSquared)
            continue;
        if(maxTargets > 0 && appliedTargets >= maxTargets)
            break;
        const int raw = DamageForSkill(caster, candidate, skillCode, point, &runtime);
        if(ApplyCompanionSkillImpact(caster, owner, candidate, skillCode, point, raw))
        {
            applied = true;
            ++appliedTargets;
        }
    }
    return applied;
}

static bool ApplyCompanionControlSkill(smCHAR *caster, rsPLAYINFO *owner, smCHAR *primary,
    DWORD skillCode, int point)
{
    if(!caster || !primary || !IsControlSkillCode(skillCode) || point < 1 || point > 10)
        return false;
    const int p = point - 1;

    if(skillCode == SKILL_HOLY_INCANTATION)
    {
        if(!owner)
            return false;
        monsters::MonsterSnapshot monster;
        if(!monsters::legacy::ServerLegacyMonsterAdapter().TryGetMonster(
               EntityId(static_cast<std::uint32_t>(primary->dwObjectSerial)), monster) ||
           monster.family != monsters::MonsterFamily::Undead)
            return false;
        int resistance = std::max(-100, std::min(100, monster.bionicResistance));
        int chance = resistance == 100 ? 0 : H_Incantation_Success[p] + owner->smCharInfo.Level / 4;
        if((rand() % 100) < std::max(0, std::min(100, chance)))
        {
            monsters::legacy::ServerLegacyMonsterAdapter().ConvertToFollower(
                monsters::FollowerConversionRequest(
                    EntityId(static_cast<std::uint32_t>(primary->dwObjectSerial)),
                    PlayerId(static_cast<std::uint32_t>(owner->dwObjectSerial)),
                    H_Incantation_Time[p] * 17, H_Incantation_AddLife[p]));
        }
        // A resisted charm is still a valid cast and consumes its resources.
        return true;
    }

    int radius = 0;
    if(skillCode == SKILL_T5_HEAVENLY_LIGHT)
        radius = std::max(1, Priestess_T5_HeavenlyLight_Area[p]);
    else
        radius = CompanionSkillAreaRadius(skillCode, point);

    if(radius > 0)
    {
        STG_AREA *area = static_cast<STG_AREA *>(caster->lpExt4);
        if(!area) return false;
        const int radius2 = radius * radius;
        bool applied = false;
        for(int i = 0; i < area->MonsterLastPoint; ++i)
        {
            smCHAR *candidate = area->lpCharMonster[i];
            if(!candidate || !candidate->Flag || candidate->smCharInfo.Life[0] <= 0) continue;
            if(candidate->smCharInfo.State != smCHAR_STATE_ENEMY || candidate->smCharInfo.Brood == smCHAR_MONSTER_USER || candidate->lpMasterPlayInfo) continue;
            const int dx = (candidate->pX - primary->pX) >> FLOATNS;
            const int dz = (candidate->pZ - primary->pZ) >> FLOATNS;
            if(dx * dx + dz * dz > radius2) continue;
            ApplyCompanionTargetSideEffects(candidate, skillCode, point, 0);
            applied = true;
        }
        // The primary can be absent from MonsterLastPoint during an area refresh.
        if(!applied && primary->Flag && primary->smCharInfo.Life[0] > 0)
        {
            ApplyCompanionTargetSideEffects(primary, skillCode, point, 0);
            applied = true;
        }
        return applied;
    }

    ApplyCompanionTargetSideEffects(primary, skillCode, point, 0);
    return true;
}

static bool CompanionPotionDebugEnabled()
{
    char value[8] = { 0 };
    return GetEnvironmentVariableA("SUMMIT_COMPANION_DEBUG", value, sizeof(value)) > 0 && value[0] != '0';
}

static const char *CompanionPotionStatusName(int status)
{
    switch(status)
    {
        case 2: return "MONITOR_CONSUMED";
        case 1: return "STACK_CONSUMED";
        case -1: return "INVALID_ARGUMENT";
        case -2: return "ACCOUNT_MISMATCH";
        case -3: return "RECORD_OPEN_FAILED";
        case -4: return "RECORD_INVALID";
        case -5: return "EMPTY";
        case -6: return "ITEM_STREAM_INVALID";
        case -7: return "STACK_RESTORE_INVALID";
        case -8: return "ITEM_RECOMPRESS_FAILED";
        case -9: return "TEMP_WRITE_FAILED";
        case -10: return "RECORD_REPLACE_FAILED";
        case -11: return "RESTORE_ZERO";
        case -12: return "MONITOR_RESTORE_METADATA_MISSING";
        case -13: return "MONITOR_UNSYNCHRONIZED";
        default: return "UNKNOWN";
    }
}

static const char *CompanionPotionSourceName(int status)
{
    if(status == 1) return "STACK";
    if(status == 2) return "MONITOR";
    return "NONE";
}

static DWORD CompanionPotionRetryDelay(int status)
{
    if(status == -5 || status == -12 || status == -13)
        return 5000U;
    if(status == -3 || status == -4 || status == -6 ||
       status == -8 || status == -9 || status == -10)
        return 3000U;
    return 1500U;
}

static void CompanionPotionDebugLog(const char *characterName, CompanionSkillRuntime &runtime,
    int type, int currentValue, int maxValue, int threshold, int status, int tier,
    DWORD itemCode, int restore, int remaining, DWORD now)
{
    if(!CompanionPotionDebugEnabled() || type < 0 || type >= 3)
        return;

    const bool changed = runtime.lastPotionStatus[type] != status ||
        runtime.lastPotionTier[type] != tier || runtime.lastPotionCode[type] != itemCode;
    if(!changed && runtime.nextPotionDebugTime[type] &&
       (int)(runtime.nextPotionDebugTime[type] - now) > 0)
        return;

    runtime.lastPotionStatus[type] = status;
    runtime.lastPotionTier[type] = tier;
    runtime.lastPotionCode[type] = itemCode;
    runtime.nextPotionDebugTime[type] = now + 5000U;

    FILE *file = fopen("LOGS\\companion_debug_server.log", "a");
    if(!file)
        file = fopen("companion_debug_server.log", "a");
    if(!file)
        return;
    const char *kind = type == 0 ? "HP" : (type == 1 ? "MP" : "RES");
    fprintf(file,
        "[%lu] ITEM_CHECK name='%s' type=%s value=%d/%d threshold=%d status=%d statusName=%s source=%s tier=%d code=0x%08lX restore=%d remaining=%d\n",
        (unsigned long)GetTickCount(), characterName ? characterName : "", kind,
        currentValue, maxValue, threshold, status, CompanionPotionStatusName(status),
        CompanionPotionSourceName(status), tier, (unsigned long)itemCode, restore, remaining);
    fflush(file);
    fclose(file);
}

static bool ConsumeAndApplyPotion(const char *accountName, const char *characterName,
    smCHAR *caster, CompanionSkillRuntime &runtime, int type, int threshold, DWORD now)
{
    if(!caster || type < 0 || type >= 3)
        return false;

    const inventory::PotionResource resource = type == 0 ? inventory::PotionResource::Life :
        (type == 1 ? inventory::PotionResource::Mana : inventory::PotionResource::Stamina);
    const inventory::PotionUseResult result = inventory::legacy::ServerLegacyPotionAdapter().ConsumeAndApply(
        inventory::PotionUseRequest(accountName ? accountName : "", characterName ? characterName : "",
            EntityId(static_cast<std::uint32_t>(caster->dwObjectSerial)), resource));

    if(!result.Ok())
    {
        runtime.nextPotionUseTime = now + CompanionPotionRetryDelay(result.legacyStatus);
        CompanionPotionDebugLog(characterName, runtime, type, result.beforeValue, result.maxValue,
            threshold, result.legacyStatus, result.tier, static_cast<DWORD>(result.itemCode),
            result.restoreAmount, result.remainingCount, now);
        return false;
    }

    runtime.nextPotionUseTime = now + 1500U;
    CompanionPotionDebugLog(characterName, runtime, type, result.beforeValue, result.maxValue,
        threshold, result.legacyStatus > 0 ? result.legacyStatus : 1, result.tier,
        static_cast<DWORD>(result.itemCode), result.restoreAmount, result.remainingCount, now);
    return true;
}

static bool CanCast(smCHAR *caster, DWORD rightCode, DWORD leftCode,
    DWORD skillCode, int point, const sSKILL_INFO **outInfo, int *outMana, int *outStamina,
    CompanionSkillAttempt *attempt, bool preserveResourceReserve = false)
{
    if(outInfo) *outInfo = NULL;
    if(outMana) *outMana = 0;
    if(outStamina) *outStamina = 0;
    if(attempt)
    {
        attempt->skillCode = skillCode;
        attempt->point = point;
    }
    if(!caster || point <= 0 || point > 10)
    {
        if(attempt) attempt->reason = COMPANION_SKILL_NO_POINT;
        return false;
    }

    const LegacyExtendedSkillMeta *extended = FindLegacyExtendedSkillMeta(skillCode);
    const sSKILL_INFO *info = extended ? NULL : FindSkillInfo(skillCode);
    if(!extended && !info)
    {
        if(attempt) attempt->reason = COMPANION_SKILL_NO_METADATA;
        return false;
    }

    const int requiredLevel = extended ? extended->requireLevel : info->RequireLevel;
    if(attempt) attempt->requiredLevel = requiredLevel;
    if(caster->smCharInfo.Level < requiredLevel)
    {
        if(attempt) attempt->reason = COMPANION_SKILL_LEVEL;
        return false;
    }

    const bool weaponAllowed = extended ?
        ExtendedWeaponAllowed(extended, rightCode, leftCode) : WeaponAllowed(info, rightCode, leftCode);
    if(!weaponAllowed)
    {
        if(attempt) attempt->reason = COMPANION_SKILL_WEAPON;
        return false;
    }

    const int mana = extended ? ExtendedManaCost(extended, point) : ManaCost(info, point);
    const int stamina = extended ? ExtendedStaminaCost(extended, point) : StaminaCost(info, point);
    if(attempt)
    {
        attempt->manaCost = mana;
        attempt->staminaCost = stamina;
    }
    if(caster->smCharInfo.Mana[0] < mana)
    {
        if(attempt) attempt->reason = COMPANION_SKILL_MP;
        return false;
    }
    if(caster->smCharInfo.Stamina[0] < stamina)
    {
        if(attempt) attempt->reason = COMPANION_SKILL_STAMINA;
        return false;
    }

    int manaReserve = 0;
    int staminaReserve = 0;
    if(preserveResourceReserve)
    {
        manaReserve = SkillResourceReserveAmount(
            caster->smCharInfo.Mana[1], "SkillManaReservePercent", 10);
        staminaReserve = SkillResourceReserveAmount(
            caster->smCharInfo.Stamina[1], "SkillStaminaReservePercent", 10);
        if(attempt)
        {
            attempt->manaReserve = manaReserve;
            attempt->staminaReserve = staminaReserve;
        }

        // Offensive casts must leave a configurable reserve after paying their
        // real legacy cost. This prevents the regen loop from enabling a single
        // cast at exactly the cost boundary and immediately starving again.
        // Support/heal/resource skills call CanCast with this policy disabled.
        if(mana > 0 && (int)caster->smCharInfo.Mana[0] - mana < manaReserve)
        {
            if(attempt) attempt->reason = COMPANION_SKILL_MP_RESERVE;
            return false;
        }
        if(stamina > 0 && (int)caster->smCharInfo.Stamina[0] - stamina < staminaReserve)
        {
            if(attempt) attempt->reason = COMPANION_SKILL_STAMINA_RESERVE;
            return false;
        }
    }

    if(outInfo) *outInfo = info; // NULL is valid for registry-backed extended skills.
    if(outMana) *outMana = mana;
    if(outStamina) *outStamina = stamina;
    if(attempt) attempt->reason = COMPANION_SKILL_OK;
    return true;
}

static DWORD SkillPlayCode(DWORD skillCode)
{
    return (DWORD)gameplay::skills::DefaultLegacySkillAdapter().PlayCode(
        (gameplay::skills::SkillId)skillCode);
}

static void BeginCast(smCHAR *caster, CompanionSkillRuntime &runtime, DWORD skillCode,
    int point, DWORD targetSerial, DWORD now, DWORD lockMs,
    int targetX = 0, int targetY = 0, int targetZ = 0, int visualParam = 0)
{
    runtime.lastSkillCode = skillCode;
    runtime.lastSkillPoint = (DWORD)std::max(0, std::min(point, 10));
    runtime.lastSkillTargetSerial = targetSerial;
    runtime.lastSkillTargetX = targetX;
    runtime.lastSkillTargetY = targetY;
    runtime.lastSkillTargetZ = targetZ;
    ++runtime.skillVisualSequence;
    if(runtime.skillVisualSequence == 0)
        runtime.skillVisualSequence = 1;
    runtime.skillLockStartedAt = now;
    runtime.skillLockUntil = now + lockMs;

    combat::legacy::ServerLegacyTargetingAdapter().ClearAcquiredTarget(
        EntityId(static_cast<std::uint32_t>(caster->dwObjectSerial)));

    const skills::SkillRuntimeResult skillRuntime = skills::legacy::ServerLegacySkillRuntimeAdapter().Begin(
        skills::SkillRuntimeRequest(
            EntityId(static_cast<std::uint32_t>(caster->dwObjectSerial)),
            static_cast<skills::SkillId>(skillCode), point, visualParam));
    runtime.lastSkillPlayCode = skillRuntime.playCode;
    runtime.lastSkillMotionState = skillRuntime.motionState;
}

static combat::CombatBuffDelta ToCombatBuffDelta(const CompanionBuffModifier &modifier)
{
    combat::CombatBuffDelta delta;
    delta.attackMin = modifier.attackMinDelta;
    delta.attackMax = modifier.attackMaxDelta;
    delta.attackRating = modifier.attackRatingDelta;
    delta.defence = modifier.defenceDelta;
    delta.attackSpeed = modifier.attackSpeedDelta;
    delta.shootingRange = modifier.shootingRangeDelta;
    delta.absorption = modifier.absorptionDelta;
    delta.chanceBlock = modifier.chanceBlockDelta;
    delta.maxLife = modifier.maxLifeDelta;
    delta.lifeRegen = modifier.lifeRegenDelta;
    delta.manaRegen = modifier.manaRegenDelta;
    for(int r = 0; r < 8; ++r)
        delta.resistance[r] = modifier.resistanceDelta[r];
    return delta;
}

static void RemoveBuffModifier(smCHAR *caster, CompanionBuffModifier &modifier)
{
    if(caster && modifier.skillCode)
    {
        combat::legacy::ServerLegacyCombatBuffAdapter().Remove(
            EntityId(static_cast<std::uint32_t>(caster->dwObjectSerial)), ToCombatBuffDelta(modifier));
    }
    modifier.Reset();
}

static int BuffDurationSeconds(DWORD skillCode, int point)
{
    if(point < 1 || point > 10)
        return 0;
    const int p = point - 1;
    switch(skillCode)
    {
        case SKILL_PHYSICAL_ABSORB: return P_Absorb_UseTime[p];
        case SKILL_MAXIMIZE: return Maximize_Time[p];
        case SKILL_AUTOMATION: return Automation_Time[p];
        case SKILL_CONCENTRATION: return Concentration_Time[p];
        case SKILL_SWIFT_AXE: return Swift_Axe_Time[p];
        case SKILL_BERSERKER: return Berserker_Time[p];
        case SKILL_DRASTIC_SPIRIT: return Drastic_Spirit_Time[p];
        case SKILL_SCOUT_HAWK: return Scout_Hawk_UseTime[p];
        case SKILL_WINDY: return Windy_Time[p];
        case SKILL_VIRTUAL_LIFE: return Virtual_Life_Time[p];
        case SKILL_EXTREME_SHIELD: return E_Shield_UseTime[p];
        case SKILL_METAL_ARMOR: return Metal_Armor_Time[p];
        case SKILL_SPARK_SHIELD: return Spark_Shield_Time[p];
        case SKILL_COMPULSION: return Compulsion_Time[p];
        case SKILL_MAGNETIC_SPHERE: return Magnetic_Sphere_Time[p];
        case SKILL_VANISH: return Vanish_Time[p];
        case SKILL_VAGUE: return Vague_Time[p];
        case SKILL_FORCE_OF_NATURE: return Force_Of_Nature_Time[p];
        case SKILL_HOLY_BODY: return HolyBody_Time[p];
        case SKILL_HOLY_VALOR: return Holy_Valor_Time[p];
        case SKILL_DIVINE_INHALATION: return D_Inhalation_Time[p];
        case SKILL_GODLY_SHIELD: return Godly_Shield_Time[p];
        case SKILL_GOD_BLESS: return God_Bless_Time[p];
        case SKILL_TRIUMPH_OF_VALHALLA: return T_Of_Valhalla_Time[p];
        case SKILL_HALL_OF_VALHALLA: return Hall_Of_Valhalla_Time[p];
        case SKILL_FROST_JAVELIN: return Frost_Javelin_Time[p];
        case SKILL_HOLY_REFLECTION: return Holy_Reflection_Time[p];
        case SKILL_REGENERATION_FIELD: return Regeneration_Field_Time[p];
        case SKILL_ENCHANT_WEAPON: return Enchant_Weapon_Time[p];
        case SKILL_ENERGY_SHIELD: return Energy_Shield_Time[p];
        case SKILL_FALCON: return Falcon_Time[p];
        case SKILL_GOLDEN_FALCON: return Golden_Falcon_Time[p];
        case SKILL_DANCING_SWORD: return Dancing_Sword_Time[p];
        case SKILL_SUMMON_MUSPELL: return Summon_Muspell_Time[p];
        case SKILL_ZENITH: return Zenith_Time[p];
        case SKILL_SPIRIT_ELEMENTAL: return Spirit_Elemental_Time[p];
        case SKILL_T5_HARDENING_SHIELD: return Hardening_Shield_Time[p];
        case SKILL_T5_RUTHLESS_OF_ZECRAM: return 300;
        case SKILL_T5_FOCUS: return 300;
        case SKILL_T5_DEATH_MASTER: return 300;
        case SKILL_T5_LETHAL_SIGHT: return 300;
        case SKILL_T5_HOLY_CONVICTION: return Knight_T5_HolyConviction_Duration[p];
        case SKILL_T5_GLORIOUS_SHIELD: return Knight_T5_GloriousShield_Duration[p];
        case SKILL_T5_CONSECRATION: return 300;
        case SKILL_T5_WIZARD_TRANCE: return Magician_T5_WizardTrance_Duration[p];
        case SKILL_T5_STONE_SKIN: return Magician_T5_StoneSkin_Duration[p];
        default: return 0;
    }
}

static bool IsVanishDetectionBoss(const smCHAR *monster)
{
    if(!monster)
        return false;
    const DWORD sound = monster->smCharInfo.dwCharSoundCode;
    return sound == snCHAR_SOUND_BABEL || sound == snCHAR_SOUND_DEATHKNIGHT ||
        sound == snCHAR_SOUND_KELVEZU || sound == snCHAR_SOUND_MOKOVA;
}

static int ActiveVanishPoint(const CompanionSkillRuntime &runtime, DWORD now)
{
    for(int i = 0; i < 4; ++i)
    {
        const CompanionBuffModifier &modifier = runtime.buffModifiers[i];
        if(modifier.skillCode == SKILL_VANISH && modifier.vanishPoint >= 1 &&
           modifier.vanishPoint <= 10 && modifier.expiresAt &&
           (int)(modifier.expiresAt - now) > 0)
            return modifier.vanishPoint;
    }
    return 0;
}

static void ConsumeCompanionVanish(smCHAR *caster, CompanionSkillRuntime &runtime)
{
    for(int i = 0; i < 4; ++i)
    {
        CompanionBuffModifier &modifier = runtime.buffModifiers[i];
        if(modifier.skillCode == SKILL_VANISH)
        {
            RemoveBuffModifier(caster, modifier);
            return;
        }
    }
}

static int VanishSightSquared(const smCHAR *monster, int point)
{
    if(!monster || point < 1 || point > 10)
        return 0;
    int sight = monster->smMonsterInfo.Real_Sight;
    sight += (sight * Vanish_Range[point - 1]) / 100;
    if(sight <= 0)
        return 0;
    return sight * sight;
}

static bool IsWithinVanishSight(const smCHAR *caster, const smCHAR *monster, int point)
{
    if(!caster || !monster || IsVanishDetectionBoss(monster))
        return true;
    const int dx = (caster->pX - monster->pX) >> FLOATNS;
    const int dy = (caster->pY - monster->pY) >> FLOATNS;
    const int dz = (caster->pZ - monster->pZ) >> FLOATNS;
    if(abs(dy) >= 140)
        return false;
    const int sightSquared = VanishSightSquared(monster, point);
    return sightSquared > 0 && (dx * dx + dy * dy + dz * dz) < sightSquared;
}

static void DropDistantTargetsForVanish(smCHAR *caster, int point)
{
    if(!caster || point < 1 || point > 10)
        return;
    for(int i = 0; i < OTHER_PLAYER_MAX; ++i)
    {
        smCHAR *monster = &chrOtherPlayer[i];
        if(!monster->Flag || monster == caster || IsVanishDetectionBoss(monster))
            continue;
        if(monster->lpTargetChar == caster && !IsWithinVanishSight(caster, monster, point))
        {
            combat::legacy::ServerLegacyTargetingAdapter().ReleaseTarget(
                EntityId(static_cast<std::uint32_t>(monster->dwObjectSerial)),
                EntityId(static_cast<std::uint32_t>(caster->dwObjectSerial)),
                combat::TargetReleaseMode::AcquiredAndForced);
        }
    }
}

static bool ApplyBuffModifier(smCHAR *caster, CompanionBuffModifier &modifier,
    DWORD skillCode, int point, DWORD now, const CompanionSkillRuntime *runtime)
{
    if(!caster || point < 1 || point > 10 || !IsBuffSkillCode(skillCode))
        return false;

    RemoveBuffModifier(caster, modifier);
    const int p = point - 1;
    modifier.skillCode = skillCode;
    switch(skillCode)
    {
        case SKILL_PHYSICAL_ABSORB:
            modifier.absorptionDelta = RandomBetween(P_Absorb[p][0], P_Absorb[p][1]);
            break;
        case SKILL_MAXIMIZE:
            modifier.attackMaxDelta = (caster->smCharInfo.Attack_Damage[1] * Maximize_Damage[p]) / 100;
            break;
        case SKILL_AUTOMATION:
            modifier.attackMinDelta = (caster->smCharInfo.Attack_Damage[0] * Automation_Damage[p]) / 100;
            modifier.attackMaxDelta = (caster->smCharInfo.Attack_Damage[1] * Automation_Damage[p]) / 100;
            modifier.attackSpeedDelta = Automation_Speed[p];
            break;
        case SKILL_CONCENTRATION:
            modifier.attackRatingDelta = Concentration_AttackRate[p];
            break;
        case SKILL_SWIFT_AXE:
            modifier.attackSpeedDelta = (caster->smCharInfo.Attack_Speed * Swift_Axe_Speed[p]) / 100;
            break;
        case SKILL_BERSERKER:
            // Legacy inventory code applies Berserker_AddAttack as a flat
            // min/max bonus and Berserker_SubAbsorb as an already-negative
            // absorption delta. Preserve those semantics; do not reinterpret
            // either table as a percentage.
            modifier.attackMinDelta = Berserker_AddAttack[p];
            modifier.attackMaxDelta = Berserker_AddAttack[p];
            modifier.absorptionDelta = std::max(-(int)caster->smCharInfo.Absorption, Berserker_SubAbsorb[p]);
            break;
        case SKILL_DRASTIC_SPIRIT:
            modifier.defenceDelta = (caster->smCharInfo.Defence * Drastic_Spirit_Defense[p]) / 100;
            break;
        case SKILL_SCOUT_HAWK:
            modifier.attackRatingDelta = (caster->smCharInfo.Attack_Rating * Scout_Hawk_Attack_Rate[p]) / 100;
            break;
        case SKILL_WINDY:
            modifier.attackRatingDelta = (caster->smCharInfo.Attack_Rating * Windy_Attack_Rating[p]) / 100;
            modifier.shootingRangeDelta = 30;
            break;
        case SKILL_VIRTUAL_LIFE:
        {
            const int oldMax = caster->smCharInfo.Life[1];
            const int requested = oldMax + (oldMax * Virtual_Life_Percent[p]) / 100;
            const int newMax = std::min(32767, std::max(oldMax, requested));
            modifier.maxLifeDelta = newMax - oldMax;
            // The legacy incoming-damage path also uses the Virtual Life
            // parameter as a percentage reduction while the buff is active.
            modifier.incomingDamageReductionPercent = ClampPercent(Virtual_Life_Percent[p]);
            break;
        }
        case SKILL_METAL_ARMOR:
        {
            const int baseDefence = caster->smCharInfo.Defence;
            modifier.defenceDelta = (baseDefence * Metal_Armor_Defense[p]) / 100;
            // Server legacy Metal Armor additionally subtracts the Physical
            // Absorption roll average as a flat amount on incoming attacks.
            if(runtime)
            {
                const int absorbPoint = LearnedPointForCode(caster->smCharInfo.JOB_CODE,
                    runtime->skillPoints, SKILL_PHYSICAL_ABSORB);
                if(absorbPoint >= 1 && absorbPoint <= 10)
                {
                    const int ap = absorbPoint - 1;
                    modifier.incomingFlatReduction = (P_Absorb[ap][0] + P_Absorb[ap][1]) / 2;
                }
            }
            break;
        }
        case SKILL_SPARK_SHIELD:
        {
            // Legacy Spark Shield derives retaliation damage from the learned
            // Spark point passed as SParam when the buff is cast.
            if(runtime)
            {
                const int sparkPoint = LearnedPointForCode(caster->smCharInfo.JOB_CODE,
                    runtime->skillPoints, SKILL_SPARK);
                if(sparkPoint >= 1 && sparkPoint <= 10)
                    modifier.sparkShieldDamage =
                        (Spark_Shield_Damage[p] * Spark_DamagePercent[sparkPoint - 1]) / 10;
            }
            break;
        }
        case SKILL_COMPULSION:
            modifier.incomingFlatReduction = std::max(0, Compulsion_AddAbsorb[p]);
            break;
        case SKILL_MAGNETIC_SPHERE:
            modifier.attackMinDelta = Magnetic_Sphere_AddDamage[p][0];
            modifier.attackMaxDelta = Magnetic_Sphere_AddDamage[p][1];
            modifier.genericDamagePercent = Magnetic_Sphere_AddDamagePercent[p];
            break;
        case SKILL_VANISH:
            // Vanish keeps no sheet delta. Its point is consumed by target
            // visibility and the next offensive action.
            modifier.vanishPoint = point;
            break;
        case SKILL_VAGUE:
            modifier.evasionPercent = ClampPercent(Vague_EvasionPercent[p]);
            break;
        case SKILL_FORCE_OF_NATURE:
            modifier.attackMinDelta = Force_Of_Nature_AddDamage[p];
            modifier.attackMaxDelta = Force_Of_Nature_AddDamage[p];
            modifier.attackRatingDelta = Force_Of_Nature_AddHit[p];
            break;
        case SKILL_HOLY_BODY:
            modifier.undeadDamageReductionPercent = ClampPercent(HolyBody_Absorb[p]);
            break;
        case SKILL_HOLY_VALOR:
            modifier.undeadDamagePercent = Holy_Valor_Damage[p];
            break;
        case SKILL_DIVINE_INHALATION:
            modifier.divineInhalationPoint = point;
            break;
        case SKILL_GODLY_SHIELD:
            modifier.incomingDamageReductionPercent = ClampPercent(Godly_Shield_AbsorbPercent[p]);
            break;
        case SKILL_GOD_BLESS:
            modifier.outgoingFlatDamage = God_Bless_AddDamage[p];
            break;
        case SKILL_TRIUMPH_OF_VALHALLA:
            modifier.attackMaxDelta = T_Of_Valhalla_Damage[p];
            break;
        case SKILL_HALL_OF_VALHALLA:
            modifier.evadePercent = Hall_Of_Valhalla_AddEvation[p];
            modifier.attackMaxDelta = T_Of_Valhalla_Damage[p];
            break;
        case SKILL_FROST_JAVELIN:
            // The elemental damage/slow is evaluated on each normal javelin
            // attack because the legacy server checks the weapon at hit time.
            modifier.frostJavelinPoint = point;
            break;
        case SKILL_HOLY_REFLECTION:
            modifier.holyReflectionPercent = Holy_Reflection_Return_Damage[p];
            break;
        case SKILL_REGENERATION_FIELD:
            modifier.lifeRegenDelta = Regeneration_Field_LifeRegen[p];
            modifier.manaRegenDelta = Regeneration_Field_ManaRegen[p];
            break;
        case SKILL_ENCHANT_WEAPON:
            modifier.attackMinDelta = Enchant_Weapon_Damage[p];
            modifier.attackMaxDelta = Enchant_Weapon_Damage[p];
            break;
        case SKILL_ENERGY_SHIELD:
            modifier.energyShieldPercent = ClampPercent(Energy_Shield_DecDamage[p]);
            break;
        case SKILL_FALCON:
            modifier.attackMinDelta = Falcon_Damage[p][0];
            modifier.attackMaxDelta = Falcon_Damage[p][1];
            break;
        case SKILL_GOLDEN_FALCON:
            modifier.attackMinDelta = Golden_Falcon_Damage[p];
            modifier.attackMaxDelta = Golden_Falcon_Damage[p];
            modifier.lifeRegenDelta = Golden_Falcon_LifeRegen[p];
            break;
        case SKILL_DANCING_SWORD:
            modifier.attackMinDelta = Dancing_Sword_Damage[p][0];
            modifier.attackMaxDelta = Dancing_Sword_Damage[p][1];
            break;
        case SKILL_SUMMON_MUSPELL:
            modifier.attackMinDelta = Summon_Muspell_Damage[p][0];
            modifier.attackMaxDelta = Summon_Muspell_Damage[p][1];
            modifier.chanceBlockDelta = Summon_Muspell_BlockPercent[p];
            modifier.absorptionDelta = Summon_Muspell_UndeadAbsorbPercent[p];
            break;
        case SKILL_EXTREME_SHIELD:
            modifier.chanceBlockDelta = E_Shield_BlockRate[p];
            break;
        case SKILL_ZENITH:
        {
            const int add = Zenith_Element[p];
            const int elements[] = { sITEMINFO_BIONIC, sITEMINFO_FIRE, sITEMINFO_ICE, sITEMINFO_LIGHTING, sITEMINFO_POISON };
            for(unsigned i = 0; i < sizeof(elements) / sizeof(elements[0]); ++i)
            {
                const int r = elements[i];
                modifier.resistanceDelta[r] = add;
            }
            break;
        }
        case SKILL_SPIRIT_ELEMENTAL:
            modifier.manaRegenDelta = Spirit_Elemental_RegenMana[p];
            modifier.magicSkillDamagePercent = Spirit_Elemental_Damage[p];
            break;
        case SKILL_T5_HARDENING_SHIELD:
            modifier.absorptionDelta = Hardening_Shield_DamageReduction[p];
            modifier.attackRatingDelta = -(caster->smCharInfo.Attack_Rating * Hardening_Shield_AttackRateReduction[p]) / 100;
            break;
        case SKILL_T5_RUTHLESS_OF_ZECRAM:
            // Ruthless is not a continuous damage multiplier: every attack
            // drains HP and only every Nth hit receives the damage bonus.
            modifier.triggerThreshold = std::max(1, Fighter_T5_Ruthless_HitsBoost[p]);
            modifier.lifeDrainPercent = Fighter_T5_Ruthless_HPDecrease[p];
            break;
        case SKILL_T5_FOCUS:
            // Death Mantra/Focus uses flat attack deltas in the legacy client
            // stat pipeline (sinInvenTory.cpp), not percentages. Keep the
            // server-authoritative companion modifier byte-for-byte semantic
            // compatible with that contract.
            modifier.attackMinDelta = -Pike_T5_Focus_MinAttackReduction[p];
            modifier.attackMaxDelta = Pike_T5_Focus_MaxAttackIncrease[p];
            break;
        case SKILL_T5_DEATH_MASTER:
            modifier.genericDamagePercent = Pike_T5_Death_DamageBoost[p];
            modifier.attackRatingDelta = (caster->smCharInfo.Attack_Rating * Pike_T5_Death_AttackRateBoost[p]) / 100;
            break;
        case SKILL_T5_LETHAL_SIGHT:
            modifier.shootingRangeDelta = Archer_T5_Lethal_RangeAdd[p];
            modifier.genericDamagePercent = Archer_T5_Lethal_DamageBoost[p];
            modifier.criticalChancePercent = Archer_T5_Lethal_CriticalRate[p];
            modifier.consumeOnAttack = true;
            break;
        case SKILL_T5_HOLY_CONVICTION:
            modifier.undeadDamagePercent = Knight_T5_HolyConviction_UndeadBoost[p];
            break;
        case SKILL_T5_GLORIOUS_SHIELD:
            modifier.defenceDelta = (caster->smCharInfo.Defence * Knight_T5_GloriousShield_DefenseBoost[p]) / 100;
            break;
        case SKILL_T5_CONSECRATION:
            modifier.defenceDelta = (caster->smCharInfo.Defence * Priestess_T5_Consecration_DefenseBoost[p]) / 100;
            modifier.absorptionDelta = Priestess_T5_Consecration_Absorption[p];
            break;
        case SKILL_T5_WIZARD_TRANCE:
            modifier.chanceBlockDelta = Magician_T5_WizardTrance_BlockBoost[p];
            modifier.attackRatingDelta = -(caster->smCharInfo.Attack_Rating * Magician_T5_WizardTrance_AttackRateReduction[p]) / 100;
            break;
        case SKILL_T5_STONE_SKIN:
            // Companion monsters do not have rsPLAYINFO's no-death timer; use the
            // same combat stat path to make the window authoritative without global state.
            modifier.absorptionDelta = std::max(0, 100 - (int)caster->smCharInfo.Absorption);
            break;
        default:
            modifier.Reset();
            return false;
    }

    const combat::CombatBuffResult buffResult = combat::legacy::ServerLegacyCombatBuffAdapter().Apply(
        EntityId(static_cast<std::uint32_t>(caster->dwObjectSerial)), ToCombatBuffDelta(modifier));
    if(!buffResult.Ok())
    {
        modifier.Reset();
        return false;
    }

    int duration = BuffDurationSeconds(skillCode, point);
    if(duration < 1) duration = 1;
    modifier.expiresAt = now + (DWORD)duration * 1000U;
    if(skillCode == SKILL_VANISH)
        DropDistantTargetsForVanish(caster, point);
    return true;
}

static bool TryCompanionSummonSkill(smCHAR *caster, rsPLAYINFO *owner, DWORD skillCode, int point,
    DWORD rightItemCode, DWORD leftItemCode, CompanionSkillRuntime &runtime, DWORD now,
    CompanionSkillAttempt *attempt)
{
    if(!caster || !owner || point < 1 || point > 10)
        return false;

    const sSKILL_INFO *info = NULL;
    int mana = 0, stamina = 0;
    if(!CanCast(caster, rightItemCode, leftItemCode, skillCode, point, &info, &mana, &stamina, attempt))
        return false;

    const int p = point - 1;
    const char *canonicalName = NULL;
    int attackPower = 0, attackRating = 0, life = 0, duration = 0;
    DWORD legacyPlayCode = 0;
    bool tier5Summon = false;
    if(skillCode == SKILL_T5_SUMMON_TIGER)
    {
        canonicalName = "Summon Tiger";
        attackPower = Atalanta_T5_SummonTiger_AttackPower[p];
        attackRating = Atalanta_T5_SummonTiger_AttackRating[p];
        life = Atalanta_T5_SummonTiger_Life[p];
        duration = Atalanta_T5_SummonTiger_Duration[p];
        tier5Summon = true;
    }
    else if(skillCode == SKILL_T5_ICE_ELEMENTAL)
    {
        canonicalName = "Ice Elemental";
        attackPower = Priestess_T5_IceElemental_AttackPower[p];
        attackRating = Priestess_T5_IceElemental_AttackRating[p];
        life = Priestess_T5_IceElemental_Life[p];
        duration = Priestess_T5_IceElemental_Duration[p];
        tier5Summon = true;
    }
    else if(skillCode == SKILL_METAL_GOLEM)
    {
        legacyPlayCode = SKILL_PLAY_METAL_GOLEM;
        attackPower = RandomBetween(Metal_Golem_Damage[p][0], Metal_Golem_Damage[p][1]);
        attackRating = Metal_Golem_Hit[p];
        life = Metal_Golem_Life[p] + owner->smCharInfo.Strength * 2;
        duration = 355;
    }
    else if(skillCode == SKILL_RECALL_WOLVERIN)
    {
        legacyPlayCode = SKILL_PLAY_RECALL_WOLVERIN;
        attackPower = RandomBetween(Recall_Wolverin_Damage[p][0], Recall_Wolverin_Damage[p][1]);
        attackRating = Recall_Wolverin_Hit[p];
        life = Recall_Wolverin_Life[p];
        duration = 355;
    }
    else if(skillCode == SKILL_FIRE_ELEMENTAL)
    {
        legacyPlayCode = SKILL_PLAY_FIRE_ELEMENTAL;
        attackPower = RandomBetween(Fire_Elemental_Damage[p][0], Fire_Elemental_Damage[p][1]);
        attackRating = Fire_Elemental_Hit[p];
        life = Fire_Elemental_Life[p];
        duration = 355;
    }
    else
        return false;

    // Spawn at the companion, but keep the real account owner as master. Tier-5
    // uses canonical-name helpers; legacy entity summons keep their established
    // OpenMonsterFromSkill templates and are then normalized below.
    smCHAR *summon = tier5Summon ?
        rsOpenTier5SummonFromCanonicalName(canonicalName, caster->pX, caster->pY, caster->pZ, owner) :
        OpenMonsterFromSkill(legacyPlayCode, caster->pX, caster->pY, caster->pZ, owner, caster);
    if(!summon)
    {
        if(attempt) attempt->reason = COMPANION_SKILL_EFFECT_FAILED;
        return false;
    }

    summon->dwUpdateCharInfoTime = now + (DWORD)std::max(1, duration) * 1000U;
    if(skillCode == SKILL_METAL_GOLEM)
    {
        summon->smCharInfo.Attack_Damage[0] = Metal_Golem_Damage[p][0];
        summon->smCharInfo.Attack_Damage[1] = Metal_Golem_Damage[p][1];
        summon->smCharInfo.Defence = Metal_Golem_Defense[p];
    }
    else if(skillCode == SKILL_RECALL_WOLVERIN)
    {
        summon->smCharInfo.Attack_Damage[0] = Recall_Wolverin_Damage[p][0];
        summon->smCharInfo.Attack_Damage[1] = Recall_Wolverin_Damage[p][1];
        summon->smCharInfo.Defence = Recall_Wolverin_Defense[p] + owner->smCharInfo.Dexterity * 2;
    }
    else if(skillCode == SKILL_FIRE_ELEMENTAL)
    {
        summon->smCharInfo.Attack_Damage[0] = Fire_Elemental_Damage[p][0];
        summon->smCharInfo.Attack_Damage[1] = Fire_Elemental_Damage[p][1];
    }
    else
    {
        summon->smCharInfo.Attack_Damage[0] = attackPower;
        summon->smCharInfo.Attack_Damage[1] = attackPower;
    }
    summon->smCharInfo.Attack_Rating = attackRating;
    summon->smCharInfo.Life[0] = (short)std::min(32767, std::max(1, life));
    summon->smCharInfo.Life[1] = summon->smCharInfo.Life[0];
    summon->sMoveStepCount[0] = 1;
    summon->sMoveStepCount[1] = 2;
    if(skillCode == SKILL_T5_SUMMON_TIGER)
        summon->smCharInfo.wPlayClass[1] = (WORD)(fONE + p * 15);
    else if(skillCode == SKILL_T5_ICE_ELEMENTAL)
        summon->smCharInfo.wPlayClass[1] = (WORD)(fONE + (p + 1) * 15);
    else if(skillCode == SKILL_METAL_GOLEM)
        summon->smCharInfo.wPlayClass[1] = (WORD)(fONE + p * 15);
    else if(skillCode == SKILL_RECALL_WOLVERIN)
        summon->smCharInfo.wPlayClass[1] = (WORD)((fONE / 2) + p * 12);
    summon->HoSkillCode = (int)(tier5Summon ? SKILL_PLAY_T51 : legacyPlayCode);
    summon->HoSkillMode = 1;

    caster->smCharInfo.Mana[0] = (short)std::max(0, (int)caster->smCharInfo.Mana[0] - mana);
    caster->smCharInfo.Stamina[0] = (short)std::max(0, (int)caster->smCharInfo.Stamina[0] - stamina);
    BeginCast(caster, runtime, skillCode, point, summon->dwObjectSerial, now, 850U,
        summon->pX, summon->pY, summon->pZ);
    if(attempt) attempt->reason = COMPANION_SKILL_OK;
    return true;
}

static bool IsChannelSkillCode(DWORD skillCode)
{
    return skillCode == SKILL_T5_WHIRLWIND ||
        skillCode == SKILL_T5_BOMBARDMENT ||
        skillCode == SKILL_T5_CATACLYSM;
}

static int ChannelDurationSeconds(DWORD skillCode, int point)
{
    if(point < 1 || point > 10) return 0;
    const int p = point - 1;
    if(skillCode == SKILL_T5_WHIRLWIND) return Fighter_T5_Whirlwind_Duration[p];
    if(skillCode == SKILL_T5_BOMBARDMENT) return Archer_T5_Bombard_Duration[p];
    if(skillCode == SKILL_T5_CATACLYSM) return Magician_T5_Cataclysm_Duration[p];
    return 0;
}

static void ChannelTickCosts(DWORD skillCode, int point, int &mana, int &stamina)
{
    mana = stamina = 0;
    if(point < 1 || point > 10) return;
    const int p = point - 1;
    if(skillCode == SKILL_T5_WHIRLWIND)
    {
        mana = Fighter_T5_Whirlwind_MPDrain[p];
        stamina = Fighter_T5_Whirlwind_SPDrain[p];
    }
    else if(skillCode == SKILL_T5_BOMBARDMENT)
    {
        mana = Archer_T5_Bombard_MPDrain[p];
        stamina = Archer_T5_Bombard_SPDrain[p];
    }
    else if(skillCode == SKILL_T5_CATACLYSM)
    {
        mana = Magician_T5_Cataclysm_MPDrain[p];
        stamina = Magician_T5_Cataclysm_SPDrain[p];
    }
}

static void StopCompanionChannel(CompanionSkillRuntime &runtime)
{
    runtime.channelSkillCode = 0;
    runtime.channelEndTime = 0;
    runtime.channelNextTick = 0;
    runtime.channelPoint = 0;
}

static void PrepareAttackTriggeredBuffs(smCHAR *caster, CompanionSkillRuntime &runtime);
static void FinishAttackTriggeredBuffs(smCHAR *caster, CompanionSkillRuntime &runtime);

static bool ExecuteCompanionChannelTick(smCHAR *caster, rsPLAYINFO *owner, smCHAR *target,
    DWORD skillCode, int point, CompanionSkillRuntime &runtime, DWORD now,
    CompanionSkillAttempt *attempt, bool beginVisual)
{
    if(!caster || !owner || !target || !target->Flag || target->smCharInfo.Life[0] <= 0)
    {
        StopCompanionChannel(runtime);
        if(attempt) attempt->reason = COMPANION_SKILL_NO_TARGET;
        return false;
    }
    if(runtime.channelEndTime && (int)(runtime.channelEndTime - now) <= 0)
    {
        StopCompanionChannel(runtime);
        if(attempt) attempt->reason = COMPANION_SKILL_COOLDOWN;
        return false;
    }
    if(runtime.channelNextTick && (int)(runtime.channelNextTick - now) > 0)
    {
        if(attempt) attempt->reason = COMPANION_SKILL_COOLDOWN;
        return false;
    }

    const int range = SkillRange(skillCode, caster, point);
    const int dx = (target->pX - caster->pX) >> FLOATNS;
    const int dz = (target->pZ - caster->pZ) >> FLOATNS;
    if(attempt)
    {
        attempt->range = range;
        attempt->distanceSquared = dx * dx + dz * dz;
    }
    if(!InRange(caster, target, range))
    {
        StopCompanionChannel(runtime);
        if(attempt) attempt->reason = COMPANION_SKILL_RANGE;
        return false;
    }

    int manaDrain = 0, staminaDrain = 0;
    ChannelTickCosts(skillCode, point, manaDrain, staminaDrain);
    if(manaDrain <= 0 || staminaDrain <= 0 ||
       caster->smCharInfo.Mana[0] < manaDrain || caster->smCharInfo.Stamina[0] < staminaDrain)
    {
        StopCompanionChannel(runtime);
        if(attempt)
            attempt->reason = caster->smCharInfo.Mana[0] < manaDrain ? COMPANION_SKILL_MP : COMPANION_SKILL_STAMINA;
        return false;
    }

    PrepareAttackTriggeredBuffs(caster, runtime);
    if(!ApplyCompanionSkillDamage(caster, owner, target, skillCode, point, runtime))
    {
        FinishAttackTriggeredBuffs(caster, runtime);
        StopCompanionChannel(runtime);
        if(attempt) attempt->reason = COMPANION_SKILL_EFFECT_FAILED;
        return false;
    }
    FinishAttackTriggeredBuffs(caster, runtime);

    caster->smCharInfo.Mana[0] = (short)std::max(0, (int)caster->smCharInfo.Mana[0] - manaDrain);
    caster->smCharInfo.Stamina[0] = (short)std::max(0, (int)caster->smCharInfo.Stamina[0] - staminaDrain);
    runtime.channelNextTick = now + 900U;
    runtime.nextDamageSkillTime = runtime.channelNextTick;
    // Legacy channels replay their attack/skill clip every tick. Re-emitting a
    // visual sequence keeps remote companions synchronized without giving the
    // client any damage authority.
    if(beginVisual)
        BeginCast(caster, runtime, skillCode, point, target->dwObjectSerial, now, 650U,
            target->pX, target->pY, target->pZ);
    if(attempt) attempt->reason = COMPANION_SKILL_OK;
    return true;
}

static void PrepareAttackTriggeredBuffs(smCHAR *caster, CompanionSkillRuntime &runtime)
{
    if(!caster) return;
    for(int i = 0; i < 4; ++i)
    {
        CompanionBuffModifier &modifier = runtime.buffModifiers[i];
        if(modifier.skillCode != SKILL_T5_RUTHLESS_OF_ZECRAM)
            continue;

        modifier.genericDamagePercent = 0;
        if(modifier.lifeDrainPercent > 0.0f && caster->smCharInfo.Life[1] > 0 && caster->smCharInfo.Life[0] > 1)
        {
            int drain = (int)((float)caster->smCharInfo.Life[1] * modifier.lifeDrainPercent / 100.0f);
            if(drain < 1) drain = 1;
            if(drain >= caster->smCharInfo.Life[0]) drain = caster->smCharInfo.Life[0] - 1;
            if(drain > 0) caster->smCharInfo.Life[0] = (short)(caster->smCharInfo.Life[0] - drain);
        }
        ++modifier.triggerCounter;
        if(modifier.triggerCounter >= std::max(1, modifier.triggerThreshold))
        {
            modifier.triggerCounter = 0;
            const int point = GetCompanionSkillPoint(runtime, 19); // Fighter Rank5 slot 3.
            if(point >= 1 && point <= 10)
                modifier.genericDamagePercent = Fighter_T5_Ruthless_DamageBoost[point - 1];
        }
    }
}

static void FinishAttackTriggeredBuffs(smCHAR *caster, CompanionSkillRuntime &runtime)
{
    for(int i = 0; i < 4; ++i)
    {
        CompanionBuffModifier &modifier = runtime.buffModifiers[i];
        if(modifier.skillCode == SKILL_T5_RUTHLESS_OF_ZECRAM)
            modifier.genericDamagePercent = 0;
        if(modifier.skillCode && modifier.consumeOnAttack)
            RemoveBuffModifier(caster, modifier);
    }
}

// V43 scheduling/falcon helpers retained alongside Tier-5 summon/channel helpers.
static smCHAR *FindScheduledTarget(smCHAR *caster, DWORD serial)
{
    if(!caster || !serial)
        return NULL;
    STG_AREA *area = static_cast<STG_AREA *>(caster->lpExt4);
    if(!area)
        return NULL;
    for(int i = 0; i < area->MonsterLastPoint; ++i)
    {
        smCHAR *candidate = area->lpCharMonster[i];
        if(candidate && candidate->Flag && candidate->dwObjectSerial == serial)
            return candidate;
    }
    return NULL;
}

static void ApplyRavingLifeCost(smCHAR *caster, int point)
{
    if(!caster || point < 1 || point > 10)
        return;
    const int p = point - 1;
    const int cost = ((int)(Raving_UseLife[p] * 10) * (int)caster->smCharInfo.Life[1]) / 1000;
    if(cost <= 0)
        return;
    caster->smCharInfo.Life[0] = (short)std::max(1, (int)caster->smCharInfo.Life[0] - cost);
}

static void StartMultiHitSchedule(smCHAR *caster, CompanionSkillRuntime &runtime,
    DWORD skillCode, int point, DWORD targetSerial, int totalHits, DWORD now)
{
    runtime.multiHitSkillCode = skillCode;
    runtime.multiHitPoint = point;
    runtime.multiHitTargetSerial = targetSerial;
    runtime.multiHitRemaining = std::max(0, totalHits - 1); // first hit already applied
    runtime.multiHitIntervalMs = 180U;
    runtime.multiHitNextTime = runtime.multiHitRemaining > 0 ? now + runtime.multiHitIntervalMs : 0;
    if(runtime.multiHitRemaining > 0)
    {
        const DWORD totalWindow = runtime.multiHitIntervalMs * (DWORD)runtime.multiHitRemaining + 300U;
        const DWORD lockUntil = now + std::max<DWORD>(700U, totalWindow);
        if((int)(lockUntil - runtime.skillLockUntil) > 0)
            runtime.skillLockUntil = lockUntil;
    }
}

static int CompanionFalconDamage(CompanionSkillRuntime &runtime, DWORD now)
{
    if(!runtime.falconExpiresAt || (int)(runtime.falconExpiresAt - now) <= 0 ||
       runtime.falconPoint < 1 || runtime.falconPoint > 10)
    {
        if(runtime.falconExpiresAt && (int)(runtime.falconExpiresAt - now) <= 0)
        {
            runtime.falconExpiresAt = 0;
            runtime.falconPoint = 0;
            runtime.goldenFalconPoint = 0;
            runtime.falconDamageMin = 0;
            runtime.falconDamageMax = 0;
        }
        return 0;
    }
    int damage = RandomBetween(runtime.falconDamageMin, runtime.falconDamageMax);
    if(runtime.forceNatureExpiresAt && (int)(runtime.forceNatureExpiresAt - now) > 0)
        damage += std::max(0, runtime.forceNatureFalconDamage);
    return std::max(0, damage);
}

static bool ApplyCompanionFalconStrike(smCHAR *caster, rsPLAYINFO *owner, smCHAR *target,
    CompanionSkillRuntime &runtime, DWORD now)
{
    const int damage = CompanionFalconDamage(runtime, now);
    return damage > 0 && ApplySkillDamage(caster, owner, target, damage);
}

static void CancelMultiHitSchedule(CompanionSkillRuntime &runtime)
{
    runtime.multiHitSkillCode = 0;
    runtime.multiHitTargetSerial = 0;
    runtime.multiHitPoint = 0;
    runtime.multiHitRemaining = 0;
    runtime.multiHitNextTime = 0;
    runtime.multiHitIntervalMs = 0;
}

static bool TryDamageCode(smCHAR *caster, rsPLAYINFO *owner, smCHAR *target,
    DWORD rightItemCode, DWORD leftItemCode, DWORD skillCode, int point,
    CompanionSkillRuntime &runtime, DWORD now, CompanionSkillAttempt *attempt)
{
    if(attempt)
    {
        attempt->Reset();
        attempt->skillCode = skillCode;
        attempt->point = point;
    }
    if(!skillCode || !IsOffensiveSkillCode(skillCode) || point <= 0)
    {
        if(attempt) attempt->reason = point <= 0 ? COMPANION_SKILL_NO_POINT : COMPANION_SKILL_NO_JOB_SKILL;
        return false;
    }

    const sSKILL_INFO *info = NULL;
    int mana = 0;
    int stamina = 0;
    if(!CanCast(caster, rightItemCode, leftItemCode, skillCode, point, &info, &mana, &stamina, attempt, true))
        return false;

    const int range = SkillRange(skillCode, caster, point);
    const int dx = (target->pX - caster->pX) >> FLOATNS;
    const int dz = (target->pZ - caster->pZ) >> FLOATNS;
    if(attempt)
    {
        attempt->range = range;
        attempt->distanceSquared = dx * dx + dz * dz;
    }
    if(!InRange(caster, target, range))
    {
        if(attempt) attempt->reason = COMPANION_SKILL_RANGE;
        return false;
    }

    if(IsChannelSkillCode(skillCode))
    {
        // Starting a channel pays the normal activation cost once. Every
        // authoritative tick then pays the dedicated legacy drain tables.
        const int duration = ChannelDurationSeconds(skillCode, point);
        if(duration <= 0)
        {
            if(attempt) attempt->reason = COMPANION_SKILL_EFFECT_FAILED;
            return false;
        }
        caster->smCharInfo.Mana[0] = (short)std::max(0, (int)caster->smCharInfo.Mana[0] - mana);
        caster->smCharInfo.Stamina[0] = (short)std::max(0, (int)caster->smCharInfo.Stamina[0] - stamina);
        runtime.channelSkillCode = skillCode;
        runtime.channelPoint = (BYTE)point;
        runtime.channelEndTime = now + (DWORD)duration * 1000U;
        runtime.channelNextTick = now;
        if(!ExecuteCompanionChannelTick(caster, owner, target, skillCode, point, runtime, now, attempt, true))
            return false;
        return true;
    }

    if(skillCode == SKILL_CHARGING_STRIKE)
    {
        caster->smCharInfo.Mana[0] = (short)std::max(0, (int)caster->smCharInfo.Mana[0] - mana);
        caster->smCharInfo.Stamina[0] = (short)std::max(0, (int)caster->smCharInfo.Stamina[0] - stamina);
        BeginCast(caster, runtime, skillCode, point, target->dwObjectSerial, now, 700U,
            target->pX, target->pY, target->pZ);
        caster->MotionLoopSpeed = 60 + (Charging_Strike_Time[point - 1] * 2);
        caster->MotionLoop = 1;

        DWORD chargeMs = 700U;
        if(caster->MotionInfo && caster->MotionLoopSpeed > 0 && caster->MotionInfo->EventFrame[2] > 0)
        {
            const DWORD ticks = ((DWORD)caster->MotionInfo->EventFrame[2] +
                (DWORD)caster->MotionLoopSpeed - 1U) / (DWORD)caster->MotionLoopSpeed;
            chargeMs = std::max<DWORD>(100U, std::min<DWORD>(4000U, (ticks * 1000U + 69U) / 70U));
        }
        runtime.chargedStrikeTargetSerial = target->dwObjectSerial;
        runtime.chargedStrikePoint = point;
        runtime.chargedStrikeExecuteAt = now + chargeMs;
        runtime.skillLockUntil = runtime.chargedStrikeExecuteAt + 350U;
        CancelMultiHitSchedule(runtime);
        runtime.nextDamageSkillTime = now + 2500U;
        if(attempt) attempt->reason = COMPANION_SKILL_OK;
        return true;
    }

    const bool statusSkill = skillCode == SKILL_PIKE_WIND || skillCode == SKILL_SHIELD_STRIKE || skillCode == SKILL_SOUL_SUCKER ||
        skillCode == SKILL_GROUND_PIKE || skillCode == SKILL_ROAR ||
        skillCode == SKILL_GLACIAL_SPIKE || skillCode == SKILL_DISTORTION || skillCode == SKILL_EXTINCTION ||
        skillCode == SKILL_VENOM_SPEAR || skillCode == SKILL_HOLY_MIND ||
        skillCode == SKILL_ASSASSIN_EYE || skillCode == SKILL_HOLY_INCANTATION;
    if(statusSkill)
    {
        // Legacy range/control packets cancel Vanish without applying its
        // single-target damage bonus.
        if(ActiveVanishPoint(runtime, now) > 0)
            ConsumeCompanionVanish(caster, runtime);
        bool applied = false;
        if(skillCode == SKILL_PIKE_WIND)
            applied = ApplyPikeWind(caster, owner, target, point, runtime);
        else if(skillCode == SKILL_SHIELD_STRIKE)
            applied = ApplyShieldStrike(caster, owner, target, point, runtime);
        else if(skillCode == SKILL_SOUL_SUCKER)
            applied = ApplySoulSucker(caster, owner, target, point);
        else
            applied = ApplyCompanionStatusSkill(caster, owner, target, skillCode, point, runtime);
        if(!applied)
        {
            if(attempt) attempt->reason = COMPANION_SKILL_EFFECT_FAILED;
            return false;
        }
        ApplyCompanionFalconStrike(caster, owner, target, runtime, now);
        caster->smCharInfo.Mana[0] = (short)std::max(0, (int)caster->smCharInfo.Mana[0] - mana);
        caster->smCharInfo.Stamina[0] = (short)std::max(0, (int)caster->smCharInfo.Stamina[0] - stamina);
        BeginCast(caster, runtime, skillCode, point, target->dwObjectSerial, now, 700U,
            target->pX, target->pY, target->pZ);
        CancelMultiHitSchedule(runtime);
        runtime.nextDamageSkillTime = now + 2500U;
        if(attempt) attempt->reason = COMPANION_SKILL_OK;
        return true;
    }

    // Canonical/Tier-5 control skills that are not part of the legacy V43
    // status packet family keep the current registry-backed implementation.
    if(IsControlSkillCode(skillCode))
    {
        if(!ApplyCompanionControlSkill(caster, owner, target, skillCode, point))
        {
            if(attempt) attempt->reason = COMPANION_SKILL_EFFECT_FAILED;
            return false;
        }
        caster->smCharInfo.Mana[0] = (short)std::max(0, (int)caster->smCharInfo.Mana[0] - mana);
        caster->smCharInfo.Stamina[0] = (short)std::max(0, (int)caster->smCharInfo.Stamina[0] - stamina);
        BeginCast(caster, runtime, skillCode, point, target->dwObjectSerial, now, 700U,
            target->pX, target->pY, target->pZ);
        CancelMultiHitSchedule(runtime);
        runtime.nextDamageSkillTime = now + 2500U;
        if(attempt) attempt->reason = COMPANION_SKILL_OK;
        return true;
    }

    PrepareAttackTriggeredBuffs(caster, runtime);

    int visualParam = 0;
    if(skillCode == SKILL_MULTISPARK)
    {
        const int maxSparks = std::max(1, M_Spark_Num[point - 1]);
        visualParam = RandomBetween((maxSparks / 2) + 1, maxSparks);
        // DamageForSkill reads the encoded param, matching SKILL_PLAY_MULTI_SPARK.
        runtime.lastSkillPlayCode = SkillPlayCode(skillCode) | ((DWORD)(visualParam & 0xF) << 12);
    }

    int damage = DamageForSkill(caster, target, skillCode, point, &runtime);
    const int savedAttackRating = caster->smCharInfo.Attack_Rating;
    if(skillCode == SKILL_FARINA)
        caster->smCharInfo.Attack_Rating += Farina_AttackRate[point - 1] + caster->smCharInfo.Level;

    bool damageApplied = false;
    const int vanishPoint = ActiveVanishPoint(runtime, now);
    const bool rangeAttack = CompanionSkillAreaRadius(skillCode, point) > 0 ||
        CompanionSkillBoxHalfWidth(skillCode, point) > 0;
    if(vanishPoint > 0 && !rangeAttack)
    {
        // Single-target legacy packets receive Vanish before later flat/element
        // modifiers. Use the same DamageForSkill formula but defer those
        // post-formula runtime additions until after the Vanish multiplier.
        damage = DamageForSkill(caster, target, skillCode, point, &runtime, false);
        damage += (damage * Vanish_Damage[vanishPoint - 1]) / 100;
        damage += ActiveOutgoingFlatDamage(runtime) + ActiveForceNatureDamage(runtime, now);
        if(!IsMagicDamageSkillCode(skillCode))
            damage = ApplyEnchantWeaponBonus(runtime, target, damage, now);
        ConsumeCompanionVanish(caster, runtime);
        damageApplied = damage > 0 && ApplySkillDamage(caster, owner, target, damage);
    }
    else
    {
        if(vanishPoint > 0)
            ConsumeCompanionVanish(caster, runtime);
        damageApplied = damage > 0 &&
            ApplyCompanionSkillDamage(caster, owner, target, skillCode, point, runtime);
    }
    caster->smCharInfo.Attack_Rating = savedAttackRating;
    FinishAttackTriggeredBuffs(caster, runtime);
    if(!damageApplied)
    {
        if(attempt) attempt->reason = COMPANION_SKILL_EFFECT_FAILED;
        return false;
    }
    ApplyCompanionFalconStrike(caster, owner, target, runtime, now);

    caster->smCharInfo.Mana[0] = (short)std::max(0, (int)caster->smCharInfo.Mana[0] - mana);
    caster->smCharInfo.Stamina[0] = (short)std::max(0, (int)caster->smCharInfo.Stamina[0] - stamina);
    BeginCast(caster, runtime, skillCode, point, target->dwObjectSerial, now, 700U,
        target->pX, target->pY, target->pZ, visualParam);
    if(skillCode == SKILL_FARINA)
    {
        caster->MotionLoopSpeed = GetAttackSpeedFrame(caster->smCharInfo.Attack_Speed, Farina_Speed[point - 1]);
        caster->MotionLoop = 1;
    }

    if(skillCode == SKILL_RAVING)
    {
        ApplyRavingLifeCost(caster, point);
        StartMultiHitSchedule(caster, runtime, skillCode, point, target->dwObjectSerial,
            std::max(1, Ravind_Speed[point - 1]), now);
    }
    else if(skillCode == SKILL_AVALANCHE)
    {
        StartMultiHitSchedule(caster, runtime, skillCode, point, target->dwObjectSerial,
            std::max(1, Avalanche_ArrowNum[point - 1]), now);
    }
    else
    {
        CancelMultiHitSchedule(runtime);
    }

    runtime.nextDamageSkillTime = now + 2500U;
    if(attempt) attempt->reason = COMPANION_SKILL_OK;
    return true;
}

} // namespace

CompanionSkillAttempt::CompanionSkillAttempt()
{
    Reset();
}

void CompanionSkillAttempt::Reset()
{
    reason = COMPANION_SKILL_INVALID;
    skillCode = 0;
    point = 0;
    requiredLevel = 0;
    manaCost = 0;
    staminaCost = 0;
    manaReserve = 0;
    staminaReserve = 0;
    range = 0;
    distanceSquared = -1;
}

const char *CompanionSkillRejectReasonName(CompanionSkillRejectReason reason)
{
    switch(reason)
    {
        case COMPANION_SKILL_OK: return "OK";
        case COMPANION_SKILL_INVALID: return "INVALID";
        case COMPANION_SKILL_COOLDOWN: return "COOLDOWN";
        case COMPANION_SKILL_LOCKED: return "LOCKED";
        case COMPANION_SKILL_NO_JOB_SKILL: return "NO_JOB_SKILL";
        case COMPANION_SKILL_NO_POINT: return "NO_POINT";
        case COMPANION_SKILL_NO_METADATA: return "NO_METADATA";
        case COMPANION_SKILL_LEVEL: return "LEVEL";
        case COMPANION_SKILL_WEAPON: return "WEAPON";
        case COMPANION_SKILL_MP: return "MP";
        case COMPANION_SKILL_STAMINA: return "STAMINA";
        case COMPANION_SKILL_MP_RESERVE: return "MP_RESERVE";
        case COMPANION_SKILL_STAMINA_RESERVE: return "STAMINA_RESERVE";
        case COMPANION_SKILL_RANGE: return "RANGE";
        case COMPANION_SKILL_NO_TARGET: return "NO_TARGET";
        case COMPANION_SKILL_MIN_TARGETS: return "MIN_TARGETS";
        case COMPANION_SKILL_NOT_NEEDED: return "NOT_NEEDED";
        case COMPANION_SKILL_EFFECT_FAILED: return "EFFECT_FAILED";
        case COMPANION_SKILL_INVALID_TARGET: return "INVALID_TARGET";
        default: return "UNKNOWN";
    }
}

CompanionSkillDescriptor::CompanionSkillDescriptor()
    : skillCode(0), point(0), useSlot(0)
{
    name[0] = 0;
}

CompanionBuffModifier::CompanionBuffModifier()
{
    Reset();
}

void CompanionBuffModifier::Reset()
{
    skillCode = 0;
    expiresAt = 0;
    attackMinDelta = 0;
    attackMaxDelta = 0;
    attackRatingDelta = 0;
    defenceDelta = 0;
    attackSpeedDelta = 0;
    shootingRangeDelta = 0;
    absorptionDelta = 0;
    chanceBlockDelta = 0;
    maxLifeDelta = 0;
    lifeRegenDelta = 0.0f;
    incomingFlatReduction = 0;
    incomingDamageReductionPercent = 0;
    undeadDamageReductionPercent = 0;
    evasionPercent = 0;
    energyShieldPercent = 0;
    outgoingFlatDamage = 0;
    frostJavelinPoint = 0;
    sparkShieldDamage = 0;
    holyReflectionPercent = 0;
    divineInhalationPoint = 0;
    vanishPoint = 0;
    manaRegenDelta = 0.0f;
    ZeroMemory(resistanceDelta, sizeof(resistanceDelta));
    magicSkillDamagePercent = 0;
    genericDamagePercent = 0;
    undeadDamagePercent = 0;
    criticalChancePercent = 0;
    evadePercent = 0;
    reactiveDamageFlat = 0;
    reactiveDamagePercent = 0;
    triggerCounter = 0;
    triggerThreshold = 0;
    lifeDrainPercent = 0.0f;
    consumeOnAttack = false;
}

static CompanionSkillRole CompanionRoleFromBehavior(gameplay::skills::SkillBehavior behavior)
{
    using gameplay::skills::SkillBehavior;
    switch(behavior)
    {
        case SkillBehavior::Passive: return COMPANION_SKILL_ROLE_PASSIVE;
        case SkillBehavior::DamageSingle: return COMPANION_SKILL_ROLE_DAMAGE_SINGLE;
        case SkillBehavior::DamageArea: return COMPANION_SKILL_ROLE_DAMAGE_AOE;
        case SkillBehavior::Control: return COMPANION_SKILL_ROLE_CONTROL;
        case SkillBehavior::BuffSelf: return COMPANION_SKILL_ROLE_BUFF_SELF;
        case SkillBehavior::BuffGroup: return COMPANION_SKILL_ROLE_BUFF_GROUP;
        case SkillBehavior::BuffTarget: return COMPANION_SKILL_ROLE_BUFF_TARGET;
        case SkillBehavior::HealSingle: return COMPANION_SKILL_ROLE_HEAL_SINGLE;
        case SkillBehavior::HealArea: return COMPANION_SKILL_ROLE_HEAL_AOE;
        case SkillBehavior::DebuffSingle: return COMPANION_SKILL_ROLE_DEBUFF_SINGLE;
        case SkillBehavior::DebuffArea: return COMPANION_SKILL_ROLE_DEBUFF_AOE;
        case SkillBehavior::Resource: return COMPANION_SKILL_ROLE_RESOURCE;
        case SkillBehavior::Summon: return COMPANION_SKILL_ROLE_SUMMON;
        case SkillBehavior::Reactive: return COMPANION_SKILL_ROLE_REACTIVE;
        case SkillBehavior::Utility: return COMPANION_SKILL_ROLE_UTILITY;
        default: return COMPANION_SKILL_ROLE_UTILITY;
    }
}

static void CopyCompanionCatalogEntry(const gameplay::skills::SkillDefinition &definition,
    CompanionSkillCatalogEntry *entry)
{
    entry->skillCode = (DWORD)definition.id;
    entry->role = CompanionRoleFromBehavior(definition.behavior);
    entry->executorReady = definition.IsExecutable();
    entry->name = definition.name.c_str();
    entry->jobCode = (DWORD)definition.ownerClassId;
    entry->useSlot = (BYTE)definition.useSlot;
    entry->tier = (BYTE)definition.tier;
}

int GetCompanionSkillCatalogCount()
{
    return (int)gameplay::skills::DefaultSkillRegistry().AllSkills().size();
}

bool GetCompanionSkillCatalogEntryAt(int index, CompanionSkillCatalogEntry *entry)
{
    const std::vector<gameplay::skills::SkillDefinition> &all =
        gameplay::skills::DefaultSkillRegistry().AllSkills();
    if(!entry || index < 0 || index >= (int)all.size())
        return false;
    CopyCompanionCatalogEntry(all[(std::size_t)index], entry);
    return true;
}

bool GetCompanionSkillCatalogEntry(DWORD skillCode, CompanionSkillCatalogEntry *entry)
{
    if(!entry)
        return false;
    const gameplay::skills::SkillDefinition *definition =
        gameplay::skills::DefaultLegacySkillAdapter().Find((gameplay::skills::SkillId)skillCode);
    if(!definition)
        return false;
    CopyCompanionCatalogEntry(*definition, entry);
    return true;
}

const char *CompanionSkillRoleName(CompanionSkillRole role)
{
    switch(role)
    {
        case COMPANION_SKILL_ROLE_PASSIVE: return "PASSIVE";
        case COMPANION_SKILL_ROLE_DAMAGE_SINGLE: return "DAMAGE_SINGLE";
        case COMPANION_SKILL_ROLE_DAMAGE_AOE: return "DAMAGE_AOE";
        case COMPANION_SKILL_ROLE_CONTROL: return "CONTROL";
        case COMPANION_SKILL_ROLE_BUFF_SELF: return "BUFF_SELF";
        case COMPANION_SKILL_ROLE_BUFF_GROUP: return "BUFF_GROUP";
        case COMPANION_SKILL_ROLE_BUFF_TARGET: return "BUFF_TARGET";
        case COMPANION_SKILL_ROLE_HEAL_SINGLE: return "HEAL_SINGLE";
        case COMPANION_SKILL_ROLE_HEAL_AOE: return "HEAL_AOE";
        case COMPANION_SKILL_ROLE_DEBUFF_SINGLE: return "DEBUFF_SINGLE";
        case COMPANION_SKILL_ROLE_DEBUFF_AOE: return "DEBUFF_AOE";
        case COMPANION_SKILL_ROLE_RESOURCE: return "RESOURCE";
        case COMPANION_SKILL_ROLE_SUMMON: return "SUMMON";
        case COMPANION_SKILL_ROLE_REACTIVE: return "REACTIVE";
        case COMPANION_SKILL_ROLE_UTILITY: return "UTILITY";
        default: return "UNKNOWN";
    }
}

bool IsCompanionDamageSkillSupported(DWORD skillCode)
{
    return IsOffensiveSkillCode(skillCode);
}

bool IsCompanionBuffSkillSupported(DWORD skillCode)
{
    return IsBuffSkillCode(skillCode);
}

bool IsCompanionSupportSkillSupported(DWORD skillCode)
{
    return IsSupportSkillCode(skillCode);
}

int GetCompanionBuffDurationSeconds(DWORD skillCode, int point)
{
    return BuffDurationSeconds(skillCode, point);
}

int GetCompanionSupportDefaultIntervalSeconds(DWORD skillCode, int point)
{
    if(skillCode == SKILL_AGONY) return 10;
    if(skillCode == SKILL_GRAND_HEALING) return 5;
    if(skillCode == SKILL_METAL_GOLEM || skillCode == SKILL_RECALL_WOLVERIN ||
       skillCode == SKILL_FIRE_ELEMENTAL) return 350;
    const int duration = BuffDurationSeconds(skillCode, point);
    return duration > 0 ? duration : 60;
}

bool GetCompanionSkillDescriptor(DWORD jobCode, const BYTE *points, int useSlot,
    CompanionSkillDescriptor *descriptor)
{
    if(!descriptor)
        return false;
    *descriptor = CompanionSkillDescriptor();
    descriptor->useSlot = useSlot;

    DWORD code = 0;
    int point = 0;
    const char *name = "";
    if(!ResolveUseSlot(jobCode, points, useSlot, &code, &point, &name))
        return false;

    descriptor->skillCode = code;
    descriptor->point = point;
    const char *canonicalName = CanonicalCompanionSkillName(code);
    if(canonicalName && canonicalName[0])
        strncpy_s(descriptor->name, sizeof(descriptor->name), canonicalName, _TRUNCATE);
    else if(name && name[0])
        strncpy_s(descriptor->name, sizeof(descriptor->name), name, _TRUNCATE);
    return true;
}

int CycleCompanionSkillUseSlot(DWORD jobCode, const BYTE *points, int currentUseSlot,
    int direction, bool buffSkill)
{
    int choices[COMPANION_MAX_SKILL_SLOTS + 1] = { 0 };
    int count = 1; // zero = disabled slot
    for(int useSlot = 1; useSlot <= COMPANION_MAX_SKILL_SLOTS; ++useSlot)
    {
        DWORD code = 0;
        int point = 0;
        if(!ResolveUseSlot(jobCode, points, useSlot, &code, &point, NULL) || point <= 0)
            continue;
        const bool supported = buffSkill ? IsSupportSkillCode(code) : IsOffensiveSkillCode(code);
        if(supported && count < COMPANION_MAX_SKILL_SLOTS + 1)
            choices[count++] = useSlot;
    }

    int index = 0;
    for(int i = 0; i < count; ++i)
    {
        if(choices[i] == currentUseSlot)
        {
            index = i;
            break;
        }
    }
    index += direction >= 0 ? 1 : -1;
    if(index >= count) index = 0;
    if(index < 0) index = count - 1;
    return choices[index];
}

CompanionSkillRuntime::CompanionSkillRuntime()
{
    Reset();
}

void CompanionSkillRuntime::Reset()
{
    ZeroMemory(skillPoints, sizeof(skillPoints));
    nextPotionUseTime = 0;
    ZeroMemory(nextPotionDebugTime, sizeof(nextPotionDebugTime));
    for(int i = 0; i < 3; ++i)
    {
        lastPotionStatus[i] = 0x7FFFFFFF;
        lastPotionTier[i] = -2;
        lastPotionCode[i] = 0xFFFFFFFF;
    }
    lastResourceRegenTime = 0;
    lifeRegenAccumulator = 0.0f;
    manaRegenAccumulator = 0.0f;
    staminaRegenAccumulator = 0.0f;
    nextDamageSkillTime = 0;
    nextHealSkillTime = 0;
    skillLockStartedAt = 0;
    skillLockUntil = 0;
    lastSkillCode = 0;
    lastSkillPlayCode = 0;
    lastSkillPoint = 0;
    lastSkillTargetSerial = 0;
    lastSkillTargetX = 0;
    lastSkillTargetY = 0;
    lastSkillTargetZ = 0;
    lastSkillMotionState = 0;
    skillVisualSequence = 0;
    forceNatureExpiresAt = 0;
    forceNatureDamage = 0;
    forceNatureFalconDamage = 0;
    holyValorExpiresAt = 0;
    holyValorPercent = 0;
    triumphValhallaExpiresAt = 0;
    triumphValhallaMaxDamageBonus = 0;
    hallValhallaExpiresAt = 0;
    hallValhallaEvasionPercent = 0;
    hallValhallaMaxDamageBonus = 0;
    regenerationFieldExpiresAt = 0;
    regenerationFieldPoint = 0;
    regenerationFieldNextPulseAt = 0;
    regenerationFieldPulseExpiresAt = 0;
    regenerationFieldPulsePoint = 0;
    enchantWeaponExpiresAt = 0;
    enchantWeaponPoint = 0;
    enchantWeaponElement = 0;
    falconExpiresAt = 0;
    falconPoint = 0;
    goldenFalconPoint = 0;
    falconDamageMin = 0;
    falconDamageMax = 0;
    persistentEffectSkillCode = 0;
    persistentEffectExpiresAt = 0;
    persistentEffectNextAttackAt = 0;
    persistentEffectPoint = 0;
    persistentEffectElement = 0;
    multiHitSkillCode = 0;
    multiHitTargetSerial = 0;
    multiHitPoint = 0;
    multiHitRemaining = 0;
    multiHitNextTime = 0;
    multiHitIntervalMs = 0;
    chargedStrikeTargetSerial = 0;
    chargedStrikeExecuteAt = 0;
    chargedStrikePoint = 0;
    activeSkillCursor = 0;
    channelSkillCode = 0;
    channelEndTime = 0;
    channelNextTick = 0;
    channelPoint = 0;
    passiveEvadePercent = 0;
    passiveStatsApplied = false;
    ZeroMemory(nextSkillRejectLogTime, sizeof(nextSkillRejectLogTime));
    ZeroMemory(lastSkillRejectCode, sizeof(lastSkillRejectCode));
    for(int i = 0; i < 3; ++i)
        lastSkillRejectReason[i] = -1;
    ZeroMemory(nextBuffSkillTime, sizeof(nextBuffSkillTime));
    for(int i = 0; i < 4; ++i)
        buffModifiers[i].Reset();
}

void LoadCompanionSkillPoints(CompanionSkillRuntime &runtime, const BYTE *points, int count)
{
    ZeroMemory(runtime.skillPoints, sizeof(runtime.skillPoints));
    if(!points || count <= 0)
        return;

    // Legacy RecordSkill()/LoadSkill() maps UseSkill[1..16] through (slot & 15):
    // logical slot 16 therefore lives in persisted byte 0. Normalize that quirk here.
    const int legacyCount = std::min(count, 16);
    for(int useSlot = 1; useSlot <= 16; ++useSlot)
    {
        const int sourceIndex = useSlot & 15;
        if(sourceIndex < legacyCount)
            runtime.skillPoints[useSlot - 1] = (BYTE)std::min(10, (int)points[sourceIndex]);
    }
}

void LoadCompanionLogicalSkillPoints(CompanionSkillRuntime &runtime, const BYTE *points, int count)
{
    ZeroMemory(runtime.skillPoints, sizeof(runtime.skillPoints));
    if(!points || count <= 0)
        return;

    const int logicalCount = std::min(count, COMPANION_MAX_SKILL_SLOTS);
    for(int index = 0; index < logicalCount; ++index)
        runtime.skillPoints[index] = (BYTE)std::min(10, (int)points[index]);
}

void SetCompanionSkillPoint(CompanionSkillRuntime &runtime, int useSlot, int point)
{
    if(useSlot < 1 || useSlot > COMPANION_MAX_SKILL_SLOTS)
        return;
    if(point < 0) point = 0;
    if(point > 10) point = 10;
    runtime.skillPoints[useSlot - 1] = (BYTE)point;
}

int GetCompanionSkillPoint(const CompanionSkillRuntime &runtime, int useSlot)
{
    if(useSlot < 1 || useSlot > COMPANION_MAX_SKILL_SLOTS)
        return 0;
    return (int)runtime.skillPoints[useSlot - 1];
}

void ApplyCompanionPassiveSkills(smCHAR *caster, CompanionSkillRuntime &runtime)
{
    if(!caster || runtime.passiveStatsApplied)
        return;
    runtime.passiveStatsApplied = true;

    const DWORD jobCode = caster->smCharInfo.JOB_CODE;
    const DWORD rightCode = caster->smCharInfo.dwTemp[smCOMPANION_NET_TEMP_RIGHT_ITEM_CODE];
    const DWORD leftCode = caster->smCharInfo.dwTemp[smCOMPANION_NET_TEMP_LEFT_ITEM_CODE];

    const std::vector<const gameplay::skills::SkillDefinition*> classSkills =
        gameplay::skills::DefaultLegacySkillAdapter().SkillsForClass((int)jobCode);
    for(std::size_t i = 0; i < classSkills.size(); ++i)
    {
        const gameplay::skills::SkillDefinition &entry = *classSkills[i];
        if(!entry.IsPassive() || entry.useSlot < 1)
            continue;

        const int point = GetCompanionSkillPoint(runtime, entry.useSlot);
        if(point < 1 || point > 10)
            continue;
        const int p = point - 1;

        // Passives are intentionally applied from the normalized catalog instead
        // of job/tier conditionals. New tiers only need a catalog row plus a
        // behavior implementation when the passive introduces a new stat family.
        switch(entry.id)
        {
            case SKILL_POISON_ATTRIBUTE:
                caster->smCharInfo.Resistance[sITEMINFO_POISON] = (short)std::min(32767,
                    (int)caster->smCharInfo.Resistance[sITEMINFO_POISON] + PlusPoison[p]);
                break;
            case SKILL_ICE_ATTRIBUTE:
                caster->smCharInfo.Resistance[sITEMINFO_ICE] = (short)std::min(32767,
                    (int)caster->smCharInfo.Resistance[sITEMINFO_ICE] + PlusIce[p]);
                break;
            case SKILL_FIRE_ATTRIBUTE:
                caster->smCharInfo.Resistance[sITEMINFO_FIRE] = (short)std::min(32767,
                    (int)caster->smCharInfo.Resistance[sITEMINFO_FIRE] + PlusFire[p]);
                break;

            case SKILL_MELEE_MASTERY:
            case SKILL_SHOOTING_MASTERY:
            case SKILL_THROWING_MASTERY:
            case SKILL_SWORD_MASTERY:
            case SKILL_MECHANIC_WEAPON:
            {
                const sSKILL_INFO *info = FindSkillInfo(entry.id);
                if(entry.id != SKILL_MECHANIC_WEAPON && !WeaponAllowed(info, rightCode, leftCode))
                    break;
                int percent = 0;
                switch(entry.id)
                {
                    case SKILL_MELEE_MASTERY: percent = Melee_Mastery_DamagePercent[p]; break;
                    case SKILL_SHOOTING_MASTERY: percent = S_Mastery_DamagePercent[p]; break;
                    case SKILL_THROWING_MASTERY: percent = T_Mastery_Damage[p]; break;
                    case SKILL_SWORD_MASTERY: percent = Sword_Mastery_DamagePercent[p]; break;
                    case SKILL_MECHANIC_WEAPON: percent = M_Weapon_Mastey[p]; break;
                }
                caster->smCharInfo.Attack_Damage[0] += (caster->smCharInfo.Attack_Damage[0] * percent) / 100;
                caster->smCharInfo.Attack_Damage[1] += (caster->smCharInfo.Attack_Damage[1] * percent) / 100;
                break;
            }

            case SKILL_WEAPONE_DEFENCE_MASTERY:
                if(WeaponAllowed(FindSkillInfo(entry.id), rightCode, leftCode))
                    caster->smCharInfo.Chance_Block += W_D_Mastery_Block[p];
                break;

            case SKILL_DIONS_EYE:
                if(WeaponAllowed(FindSkillInfo(entry.id), rightCode, leftCode))
                    caster->smCharInfo.Attack_Rating +=
                        (caster->smCharInfo.Attack_Rating * D_Eye_Attack_Rate[p]) / 100;
                break;

            case SKILL_CRITICAL_MASTERY:
                if(WeaponAllowed(FindSkillInfo(entry.id), rightCode, leftCode))
                    caster->smCharInfo.Critical_Hit += Critical_Mastery_Critical[p];
                break;

            case SKILL_EVASION_MASTERY:
                runtime.passiveEvadePercent = (BYTE)std::max((int)runtime.passiveEvadePercent,
                    std::max(0, std::min(100, Evasion_Mastery_AddPercent[p])));
                break;

            case SKILL_PHYSICAL_TRANING:
            {
                const int oldMax = caster->smCharInfo.Stamina[1];
                const int add = (oldMax * P_Traning_Stamina[p]) / 100;
                const int newMax = std::min(32767, std::max(1, oldMax + add));
                caster->smCharInfo.Stamina[1] = (short)newMax;
                if(caster->smCharInfo.Stamina[0] >= oldMax)
                    caster->smCharInfo.Stamina[0] = (short)newMax;
                break;
            }

            case SKILL_MEDITATION:
                caster->smCharInfo.Mana_Regen += Meditation_Regen[p];
                break;

            case SKILL_MENTAL_MASTERY:
            {
                const int oldMax = caster->smCharInfo.Mana[1];
                const int add = (oldMax * Mental_Mastery_Mana[p]) / 100;
                const int newMax = std::min(32767, std::max(1, oldMax + add));
                caster->smCharInfo.Mana[1] = (short)newMax;
                if(caster->smCharInfo.Mana[0] >= oldMax)
                    caster->smCharInfo.Mana[0] = (short)newMax;
                break;
            }

            case SKILL_BOOST_HEALTH:
            {
                const int oldMax = caster->smCharInfo.Life[1];
                const int newMax = std::min(32767, std::max(1, oldMax + Boost_Health_Life[p]));
                caster->smCharInfo.Life[1] = (short)newMax;
                if(caster->smCharInfo.Life[0] >= oldMax)
                    caster->smCharInfo.Life[0] = (short)newMax;
                break;
            }

            case SKILL_T5_SURVIVAL_INSTINCT:
            {
                const int oldMax = caster->smCharInfo.Life[1];
                const int newMax = std::min(32767, std::max(1, oldMax + Fighter_T5_Survival_Life[p]));
                caster->smCharInfo.Life[1] = (short)newMax;
                if(caster->smCharInfo.Life[0] >= oldMax)
                    caster->smCharInfo.Life[0] = (short)newMax;
                caster->smCharInfo.Life_Regen += (float)Fighter_T5_Survival_Regen[p];
                break;
            }

            case SKILL_T5_GOLDEN_APPLE:
                caster->smCharInfo.Move_Speed +=
                    (caster->smCharInfo.Move_Speed * Atalanta_T5_GoldenApple_Speed[p]) / 100;
                runtime.passiveEvadePercent = (BYTE)std::max((int)runtime.passiveEvadePercent,
                    std::max(0, std::min(100, Atalanta_T5_GoldenApple_Evade[p])));
                break;

            default:
                break;
        }
    }
}

bool IsCompanionSkillLocked(const CompanionSkillRuntime &runtime, DWORD now)
{
    return runtime.skillLockUntil && (int)(runtime.skillLockUntil - now) > 0;
}

void UpdateCompanionRegenerationFieldAura(smCHAR *caster, smCHAR *const *companions,
    CompanionSkillRuntime *const *companionRuntimes, int companionCount,
    CompanionSkillRuntime &runtime, DWORD now)
{
    if(!caster || runtime.regenerationFieldPoint < 1 || runtime.regenerationFieldPoint > 10 ||
       !runtime.regenerationFieldExpiresAt || (int)(runtime.regenerationFieldExpiresAt - now) <= 0)
        return;
    if(runtime.regenerationFieldNextPulseAt && (int)(runtime.regenerationFieldNextPulseAt - now) > 0)
        return;

    const int p = runtime.regenerationFieldPoint - 1;
    const int area = std::max(1, Regeneration_Field_Area[p]);
    const int areaSquared = area * area;
    for(int i = 0; i < companionCount; ++i)
    {
        smCHAR *candidate = companions ? companions[i] : NULL;
        CompanionSkillRuntime *candidateRuntime = companionRuntimes ? companionRuntimes[i] : NULL;
        if(!candidate || !candidateRuntime || candidate == caster || !candidate->Flag || candidate->smCharInfo.Life[0] <= 0)
            continue;
        const int dx = (candidate->pX - caster->pX) >> FLOATNS;
        const int dy = (candidate->pY - caster->pY) >> FLOATNS;
        const int dz = (candidate->pZ - caster->pZ) >> FLOATNS;
        if(dx * dx + dz * dz >= areaSquared || abs(dy) >= 16)
            continue;
        candidateRuntime->regenerationFieldPulseExpiresAt = now + 5000U;
        candidateRuntime->regenerationFieldPulsePoint = runtime.regenerationFieldPoint;
    }
    runtime.regenerationFieldNextPulseAt = now + 4000U;
}

bool UpdateCompanionResourceRegeneration(smCHAR *caster, CompanionSkillRuntime &runtime, DWORD now)
{
    if(!caster || caster->smCharInfo.Life[0] <= 0 || caster->smCharInfo.Life[1] <= 0)
    {
        runtime.lastResourceRegenTime = now;
        runtime.lifeRegenAccumulator = 0.0f;
        runtime.manaRegenAccumulator = 0.0f;
        runtime.staminaRegenAccumulator = 0.0f;
        return false;
    }

    if(!runtime.lastResourceRegenTime)
    {
        runtime.lastResourceRegenTime = now;
        return false;
    }

    DWORD elapsedMs = now - runtime.lastResourceRegenTime;
    if(elapsedMs < 50U)
        return false;
    // Do not grant a large offline/stall burst after a long server hitch.
    if(elapsedMs > 2000U) elapsedMs = 2000U;
    runtime.lastResourceRegenTime = now;
    const float elapsedSeconds = (float)elapsedMs / 1000.0f;

    const float level = (float)caster->smCharInfo.Level;
    const float strength = (float)caster->smCharInfo.Strength;
    const float health = (float)caster->smCharInfo.Health;
    const float spirit = (float)caster->smCharInfo.Spirit;

    // Exact legacy sinRegen() formulas, made time-based instead of frame-based.
    float fieldLifeRegen = 0.0f;
    float fieldManaRegen = 0.0f;
    if(runtime.regenerationFieldExpiresAt && (int)(runtime.regenerationFieldExpiresAt - now) > 0 &&
       runtime.regenerationFieldPoint >= 1 && runtime.regenerationFieldPoint <= 10)
    {
        const int p = runtime.regenerationFieldPoint - 1;
        fieldLifeRegen += Regeneration_Field_LifeRegen[p];
        fieldManaRegen += Regeneration_Field_ManaRegen[p];
    }
    if(runtime.regenerationFieldPulseExpiresAt && (int)(runtime.regenerationFieldPulseExpiresAt - now) > 0 &&
       runtime.regenerationFieldPulsePoint >= 1 && runtime.regenerationFieldPulsePoint <= 10)
    {
        const int p = runtime.regenerationFieldPulsePoint - 1;
        fieldLifeRegen += Regeneration_Field_LifeRegen[p];
        fieldManaRegen += Regeneration_Field_ManaRegen[p] / 2.0f;
    }
    if(runtime.falconExpiresAt && (int)(runtime.falconExpiresAt - now) > 0 &&
       runtime.goldenFalconPoint >= 1 && runtime.goldenFalconPoint <= 10)
        fieldLifeRegen += Golden_Falcon_LifeRegen[runtime.goldenFalconPoint - 1];

    const float lifePerSecond = (((level + (strength / 2.0f) + health) / 180.0f) +
        caster->smCharInfo.Life_Regen + fieldLifeRegen) / 1.5f;
    const float manaPerSecond = ((level + (spirit * 1.2f) + (health / 2.0f)) / 115.0f) +
        caster->smCharInfo.Mana_Regen + fieldManaRegen;

    float staminaPerSecond = 0.0f;
    const DWORD motion = caster->MotionInfo ? (DWORD)caster->MotionInfo->State : (DWORD)CHRMOTION_STATE_STAND;
    if(motion == CHRMOTION_STATE_STAND || motion == CHRMOTION_STATE_WALK)
    {
        float base = 3.8f;
        if(caster->smCharInfo.StaminaFunction == 2) base = 3.3f;
        else if(caster->smCharInfo.StaminaFunction == 3) base = 2.9f;
        staminaPerSecond = base + ((float)caster->smCharInfo.Level / 7.0f) + caster->smCharInfo.Stamina_Regen;
        if(motion == CHRMOTION_STATE_WALK) staminaPerSecond *= 0.6f;
    }

    bool changed = false;
    if(caster->smCharInfo.Life[0] < caster->smCharInfo.Life[1] && lifePerSecond > 0.0f)
    {
        runtime.lifeRegenAccumulator += lifePerSecond * elapsedSeconds;
        const int add = (int)runtime.lifeRegenAccumulator;
        if(add > 0)
        {
            const int before = caster->smCharInfo.Life[0];
            caster->smCharInfo.Life[0] = (short)std::min((int)caster->smCharInfo.Life[1], before + add);
            runtime.lifeRegenAccumulator -= (float)add;
            changed = caster->smCharInfo.Life[0] != before || changed;
        }
    }
    else if(caster->smCharInfo.Life[0] >= caster->smCharInfo.Life[1]) runtime.lifeRegenAccumulator = 0.0f;

    if(caster->smCharInfo.Mana[1] > 0 && caster->smCharInfo.Mana[0] < caster->smCharInfo.Mana[1] && manaPerSecond > 0.0f)
    {
        runtime.manaRegenAccumulator += manaPerSecond * elapsedSeconds;
        const int add = (int)runtime.manaRegenAccumulator;
        if(add > 0)
        {
            const int before = caster->smCharInfo.Mana[0];
            caster->smCharInfo.Mana[0] = (short)std::min((int)caster->smCharInfo.Mana[1], before + add);
            runtime.manaRegenAccumulator -= (float)add;
            changed = caster->smCharInfo.Mana[0] != before || changed;
        }
    }
    else if(caster->smCharInfo.Mana[0] >= caster->smCharInfo.Mana[1]) runtime.manaRegenAccumulator = 0.0f;

    if(caster->smCharInfo.Stamina[1] > 0 && caster->smCharInfo.Stamina[0] < caster->smCharInfo.Stamina[1] && staminaPerSecond > 0.0f)
    {
        runtime.staminaRegenAccumulator += staminaPerSecond * elapsedSeconds;
        const int add = (int)runtime.staminaRegenAccumulator;
        if(add > 0)
        {
            const int before = caster->smCharInfo.Stamina[0];
            caster->smCharInfo.Stamina[0] = (short)std::min((int)caster->smCharInfo.Stamina[1], before + add);
            runtime.staminaRegenAccumulator -= (float)add;
            changed = caster->smCharInfo.Stamina[0] != before || changed;
        }
    }
    else if(caster->smCharInfo.Stamina[0] >= caster->smCharInfo.Stamina[1]) runtime.staminaRegenAccumulator = 0.0f;

    return changed;
}

bool TryUseCompanionConsumable(const char *accountName, const char *characterName,
    smCHAR *caster, CompanionSkillRuntime &runtime, DWORD now)
{
    if(!caster || caster->smCharInfo.Life[0] <= 0 ||
       !accountName || !accountName[0] || !characterName || !characterName[0])
        return false;
    if(!AutoUsePotionsEnabled())
        return false;
    if(runtime.nextPotionUseTime && (int)(runtime.nextPotionUseTime - now) > 0)
        return false;

    const int lifeThreshold = PotionThreshold("PotionLifePercent", 45);
    const int manaThreshold = PotionThreshold("PotionManaPercent", 25);
    const int staminaThreshold = PotionThreshold("PotionStaminaPercent", 25);

    if(lifeThreshold > 0 && caster->smCharInfo.Life[1] > 0 &&
       caster->smCharInfo.Life[0] * 100 <= caster->smCharInfo.Life[1] * lifeThreshold &&
       ConsumeAndApplyPotion(accountName, characterName, caster, runtime, 0, lifeThreshold, now))
        return true;

    if(manaThreshold > 0 && caster->smCharInfo.Mana[1] > 0 &&
       caster->smCharInfo.Mana[0] * 100 <= caster->smCharInfo.Mana[1] * manaThreshold &&
       ConsumeAndApplyPotion(accountName, characterName, caster, runtime, 1, manaThreshold, now))
        return true;

    if(staminaThreshold > 0 && caster->smCharInfo.Stamina[1] > 0 &&
       caster->smCharInfo.Stamina[0] * 100 <= caster->smCharInfo.Stamina[1] * staminaThreshold &&
       ConsumeAndApplyPotion(accountName, characterName, caster, runtime, 2, staminaThreshold, now))
        return true;

    return false;
}

void LogCompanionPotionState(const char *characterName, smCHAR *caster,
    const CompanionSkillRuntime &runtime, DWORD now)
{
    if(!caster || !CompanionPotionDebugEnabled())
        return;
    const int hpThreshold = PotionThreshold("PotionLifePercent", 45);
    const int mpThreshold = PotionThreshold("PotionManaPercent", 25);
    const int resThreshold = PotionThreshold("PotionStaminaPercent", 25);
    const int hpNeed = caster->smCharInfo.Life[1] > 0 && caster->smCharInfo.Life[0] * 100 <= caster->smCharInfo.Life[1] * hpThreshold;
    const int mpNeed = caster->smCharInfo.Mana[1] > 0 && caster->smCharInfo.Mana[0] * 100 <= caster->smCharInfo.Mana[1] * mpThreshold;
    const int resNeed = caster->smCharInfo.Stamina[1] > 0 && caster->smCharInfo.Stamina[0] * 100 <= caster->smCharInfo.Stamina[1] * resThreshold;
    FILE *file = fopen("LOGS\\companion_debug_server.log", "a");
    if(!file) file = fopen("companion_debug_server.log", "a");
    if(!file) return;
    const int manaReserve = SkillResourceReserveAmount(
        caster->smCharInfo.Mana[1], "SkillManaReservePercent", 10);
    const int staminaReserve = SkillResourceReserveAmount(
        caster->smCharInfo.Stamina[1], "SkillStaminaReservePercent", 10);
    fprintf(file,
        "[%lu] POTION_STATE name='%s' life=%d/%d needHP=%d thresholdHP=%d mana=%d/%d needMP=%d thresholdMP=%d reserveMP=%d stamina=%d/%d needRES=%d thresholdRES=%d reserveRES=%d enabled=%d cooldown=%d\n",
        (unsigned long)GetTickCount(), characterName ? characterName : "",
        caster->smCharInfo.Life[0], caster->smCharInfo.Life[1], hpNeed, hpThreshold,
        caster->smCharInfo.Mana[0], caster->smCharInfo.Mana[1], mpNeed, mpThreshold, manaReserve,
        caster->smCharInfo.Stamina[0], caster->smCharInfo.Stamina[1], resNeed, resThreshold, staminaReserve,
        AutoUsePotionsEnabled() ? 1 : 0,
        runtime.nextPotionUseTime && (int)(runtime.nextPotionUseTime - now) > 0 ? 1 : 0);
    fflush(file);
    fclose(file);
}

static smCHAR *FindNearestPersistentEffectTarget(smCHAR *caster)
{
    if(!caster)
        return NULL;
    STG_AREA *area = static_cast<STG_AREA *>(caster->lpExt4);
    if(!area)
        return NULL;
    smCHAR *best = NULL;
    int bestDistance = 300 * 300;
    for(int i = 0; i < area->MonsterLastPoint; ++i)
    {
        smCHAR *candidate = area->lpCharMonster[i];
        if(!candidate || !candidate->Flag || candidate == caster || candidate->smCharInfo.Life[0] <= 0 ||
           candidate->smCharInfo.State != smCHAR_STATE_ENEMY || candidate->smCharInfo.Brood == smCHAR_MONSTER_USER ||
           candidate->lpMasterPlayInfo)
            continue;
        const int dx = (candidate->pX - caster->pX) >> FLOATNS;
        const int dy = (candidate->pY - caster->pY) >> FLOATNS;
        const int dz = (candidate->pZ - caster->pZ) >> FLOATNS;
        const int distance = dx * dx + dy * dy + dz * dz;
        if(distance < bestDistance)
        {
            bestDistance = distance;
            best = candidate;
        }
    }
    return best;
}

bool UpdateCompanionPersistentEffect(smCHAR *caster, rsPLAYINFO *owner,
    CompanionSkillRuntime &runtime, DWORD now)
{
    if(!caster || !owner || !runtime.persistentEffectSkillCode ||
       !runtime.persistentEffectExpiresAt || (int)(runtime.persistentEffectExpiresAt - now) <= 0)
    {
        if(runtime.persistentEffectExpiresAt && (int)(runtime.persistentEffectExpiresAt - now) <= 0)
        {
            runtime.persistentEffectSkillCode = 0;
            runtime.persistentEffectExpiresAt = 0;
            runtime.persistentEffectNextAttackAt = 0;
            runtime.persistentEffectPoint = 0;
            runtime.persistentEffectElement = 0;
        }
        return false;
    }
    if(runtime.persistentEffectPoint < 1 || runtime.persistentEffectPoint > 10)
        return false;
    if(runtime.persistentEffectNextAttackAt && (int)(runtime.persistentEffectNextAttackAt - now) > 0)
        return false;

    smCHAR *target = FindNearestPersistentEffectTarget(caster);
    const int p = runtime.persistentEffectPoint - 1;
    DWORD nextDelay = 1000U;
    bool applied = false;
    if(runtime.persistentEffectSkillCode == SKILL_MAGNETIC_SPHERE)
    {
        nextDelay = (DWORD)std::max(1, Magnetic_Sphere_AttackDelay[p]) * 1000U;
        if(target)
        {
            int damage = RandomBetween(Magnetic_Sphere_AddDamage[p][0], Magnetic_Sphere_AddDamage[p][1]);
            if(target->smCharInfo.Brood == smCHAR_MONSTER_MECHANIC)
                damage += (damage * 30) / 100;
            damage = ApplyLegacyResistance(damage, target, sITEMINFO_LIGHTING);
            if(damage > 0)
                applied = ApplySkillDamage(caster, owner, target, damage);
        }
    }
    else if(runtime.persistentEffectSkillCode == SKILL_DANCING_SWORD)
    {
        // The legacy server validates Dancing Sword at AttackDelay * 900ms.
        nextDelay = (DWORD)std::max(1, Dancing_Sword_AttackDelay[p]) * 900U;
        if(target)
        {
            int damage = RandomBetween(Dancing_Sword_Damage[p][0], Dancing_Sword_Damage[p][1]);
            const int resistanceIndex = runtime.persistentEffectElement == 0 ? sITEMINFO_ICE : sITEMINFO_FIRE;
            damage = ApplyLegacyResistance(damage, target, resistanceIndex);
            if(damage > 0)
                applied = ApplySkillDamage(caster, owner, target, damage);
            if(applied && runtime.persistentEffectElement == 0)
            {
                const int resistance = ClampLegacyResistance(target->smCharInfo.Resistance[sITEMINFO_ICE]);
                combat::legacy::ServerLegacyStatusEffectAdapter().ApplyElementalSlow16Hz(
                    EntityId(static_cast<std::uint32_t>(target->dwObjectSerial)),
                    5, 230 - 10 * runtime.persistentEffectPoint, resistance);
            }
        }
    }
    else if(runtime.persistentEffectSkillCode == SKILL_SUMMON_MUSPELL)
    {
        nextDelay = (DWORD)std::max(1, Summon_Muspell_Attack_Delay[p]) * 1000U;
        if(target)
            applied = ApplySkillDamage(caster, owner, target,
                RandomBetween(Summon_Muspell_Damage[p][0], Summon_Muspell_Damage[p][1]));
    }
    else
    {
        runtime.persistentEffectSkillCode = 0;
        runtime.persistentEffectExpiresAt = 0;
        runtime.persistentEffectNextAttackAt = 0;
        runtime.persistentEffectPoint = 0;
        runtime.persistentEffectElement = 0;
        return false;
    }

    runtime.persistentEffectNextAttackAt = now + nextDelay;
    return applied;
}

bool UpdateCompanionMultiHit(smCHAR *caster, rsPLAYINFO *owner,
    CompanionSkillRuntime &runtime, DWORD now)
{
    if(!caster || !owner || !runtime.multiHitSkillCode || runtime.multiHitRemaining <= 0)
        return false;
    if(runtime.multiHitNextTime && (int)(runtime.multiHitNextTime - now) > 0)
        return false;

    smCHAR *target = FindScheduledTarget(caster, runtime.multiHitTargetSerial);
    if(!target || target->smCharInfo.Life[0] <= 0 || target->smCharInfo.State != smCHAR_STATE_ENEMY)
    {
        CancelMultiHitSchedule(runtime);
        return false;
    }

    const DWORD skillCode = runtime.multiHitSkillCode;
    const int point = runtime.multiHitPoint;
    if(point < 1 || point > 10 || (skillCode != SKILL_RAVING && skillCode != SKILL_AVALANCHE))
    {
        CancelMultiHitSchedule(runtime);
        return false;
    }

    const bool applied = ApplySkillDamage(caster, owner, target,
        DamageForSkill(caster, target, skillCode, point, &runtime));
    if(applied && skillCode == SKILL_RAVING)
        ApplyRavingLifeCost(caster, point);

    --runtime.multiHitRemaining;
    if(!applied || runtime.multiHitRemaining <= 0 || target->smCharInfo.Life[0] <= 0)
        CancelMultiHitSchedule(runtime);
    else
        runtime.multiHitNextTime = now + std::max<DWORD>(1U, runtime.multiHitIntervalMs);
    return applied;
}

bool UpdateCompanionChargedStrike(smCHAR *caster, rsPLAYINFO *owner,
    CompanionSkillRuntime &runtime, DWORD now)
{
    if(!caster || !owner || !runtime.chargedStrikeTargetSerial ||
       runtime.chargedStrikePoint < 1 || runtime.chargedStrikePoint > 10)
        return false;
    if(runtime.chargedStrikeExecuteAt && (int)(runtime.chargedStrikeExecuteAt - now) > 0)
        return false;

    smCHAR *target = FindScheduledTarget(caster, runtime.chargedStrikeTargetSerial);
    const int point = runtime.chargedStrikePoint;
    runtime.chargedStrikeTargetSerial = 0;
    runtime.chargedStrikeExecuteAt = 0;
    runtime.chargedStrikePoint = 0;
    caster->MotionLoop = 0;
    if(!target || target->smCharInfo.Life[0] <= 0 ||
       !InRange(caster, target, SkillRange(SKILL_CHARGING_STRIKE, caster, point)))
        return false;

    int chargedDamage = DamageForSkill(caster, target, SKILL_CHARGING_STRIKE, point, &runtime);
    const int vanishPoint = ActiveVanishPoint(runtime, now);
    if(vanishPoint > 0)
    {
        chargedDamage = DamageForSkill(caster, target, SKILL_CHARGING_STRIKE, point, &runtime, false);
        chargedDamage += (chargedDamage * Vanish_Damage[vanishPoint - 1]) / 100;
        chargedDamage += ActiveOutgoingFlatDamage(runtime) + ActiveForceNatureDamage(runtime, now);
        chargedDamage = ApplyEnchantWeaponBonus(runtime, target, chargedDamage, now);
        ConsumeCompanionVanish(caster, runtime);
    }
    const bool applied = ApplySkillDamage(caster, owner, target, chargedDamage);
    if(applied)
        ApplyCompanionFalconStrike(caster, owner, target, runtime, now);
    return applied;
}

bool TryExecuteCompanionDamageSkill(smCHAR *caster, rsPLAYINFO *owner, smCHAR *target,
    DWORD rightItemCode, DWORD leftItemCode, CompanionSkillRuntime &runtime, DWORD now,
    CompanionSkillAttempt *attempt)
{
    if(attempt) attempt->Reset();
    if(!caster || !owner || !target || !target->Flag || target->smCharInfo.Life[0] <= 0)
    {
        if(attempt) attempt->reason = COMPANION_SKILL_INVALID;
        return false;
    }
    if(runtime.nextDamageSkillTime && (int)(runtime.nextDamageSkillTime - now) > 0)
    {
        if(attempt) attempt->reason = COMPANION_SKILL_COOLDOWN;
        return false;
    }
    if(IsCompanionSkillLocked(runtime, now))
    {
        if(attempt) attempt->reason = COMPANION_SKILL_LOCKED;
        return false;
    }

    const DWORD skillCode = DamageSkillForJob(caster->smCharInfo.JOB_CODE);
    const int point = LearnedPointForCode(caster->smCharInfo.JOB_CODE, runtime.skillPoints, skillCode);
    return TryDamageCode(caster, owner, target, rightItemCode, leftItemCode,
        skillCode, point, runtime, now, attempt);
}

static bool IsConfiguredLoadoutHostile(smCHAR *target)
{
    return target && target->Flag && target->smCharInfo.Life[0] > 0 &&
        target->smCharInfo.State == smCHAR_STATE_ENEMY &&
        target->smCharInfo.Brood != smCHAR_MONSTER_USER && !target->lpMasterPlayInfo;
}

static smCHAR *NearestConfiguredLoadoutTarget(smCHAR *caster)
{
    if(!caster)
        return NULL;
    STG_AREA *area = static_cast<STG_AREA *>(caster->lpExt4);
    if(!area)
        return NULL;
    smCHAR *best = NULL;
    long long bestDistance = 0x7FFFFFFFFFFFFFFFLL;
    for(int i = 0; i < area->MonsterLastPoint; ++i)
    {
        smCHAR *candidate = area->lpCharMonster[i];
        if(!IsConfiguredLoadoutHostile(candidate))
            continue;
        const long long dx = (candidate->pX - caster->pX) >> FLOATNS;
        const long long dz = (candidate->pZ - caster->pZ) >> FLOATNS;
        const long long distance = dx * dx + dz * dz;
        if(!best || distance < bestDistance)
        {
            best = candidate;
            bestDistance = distance;
        }
    }
    return best;
}

static smCHAR *ResolveConfiguredLoadoutTarget(smCHAR *caster, rsPLAYINFO *owner,
    smCHAR *fallback, int targetMode)
{
    switch(targetMode)
    {
        case 1: // OWNER_TARGET
            return owner && IsConfiguredLoadoutHostile(owner->lpChrTarget) ? owner->lpChrTarget : NULL;
        case 2:
            return NearestConfiguredLoadoutTarget(caster);
        default:
            return IsConfiguredLoadoutHostile(fallback) ? fallback : NULL;
    }
}

static bool UsesCasterCenteredConfiguredArea(DWORD skillCode)
{
    switch(skillCode)
    {
        case SKILL_PIKE_WIND:
        case SKILL_GROUND_PIKE:
        case SKILL_ROAR:
        case SKILL_VENOM_SPEAR:
        case SKILL_EXTINCTION:
        case SKILL_DISTORTION:
            return true;
        default:
            return false;
    }
}

static int CountConfiguredSkillTargets(smCHAR *caster, smCHAR *primary, DWORD skillCode, int point)
{
    if(!caster || !IsConfiguredLoadoutHostile(primary))
        return 0;
    int maxTargets = CompanionSkillMaxTargets(skillCode, point);
    if(skillCode == SKILL_VENOM_SPEAR && point >= 1 && point <= 10)
        maxTargets = std::max(1, VenomSpear_Num[point - 1]);
    const int boxHalfWidth = CompanionSkillBoxHalfWidth(skillCode, point);
    const int boxForwardLength = CompanionSkillBoxForwardLength(caster, skillCode, point);
    int areaRadius = CompanionSkillAreaRadius(skillCode, point);
    const bool casterCentered = UsesCasterCenteredConfiguredArea(skillCode);
    if(casterCentered)
        areaRadius = SkillRange(skillCode, caster, point);
    if(boxHalfWidth <= 0 && areaRadius <= 0)
        return 1;

    STG_AREA *area = static_cast<STG_AREA *>(caster->lpExt4);
    if(!area)
        return 1;
    int count = 0;
    const int radiusSquared = areaRadius > 0 ? areaRadius * areaRadius : 0;
    for(int i = -1; i < area->MonsterLastPoint; ++i)
    {
        smCHAR *candidate = i < 0 ? primary : area->lpCharMonster[i];
        if(!candidate || (i >= 0 && candidate == primary) || !IsConfiguredLoadoutHostile(candidate))
            continue;
        bool inside = false;
        if(boxHalfWidth > 0 && boxForwardLength > 0)
            inside = IsInsideCompanionForwardBox(caster, primary, candidate, boxHalfWidth, boxForwardLength);
        else
        {
            const smCHAR *center = casterCentered ? caster : primary;
            const int dx = (candidate->pX - center->pX) >> FLOATNS;
            const int dz = (candidate->pZ - center->pZ) >> FLOATNS;
            inside = abs((candidate->pY - center->pY) >> FLOATNS) <= 128 &&
                dx * dx + dz * dz <= radiusSquared;
        }
        if(!inside)
            continue;
        ++count;
        if(maxTargets > 0 && count >= maxTargets)
            break;
    }
    return count;
}

static bool MeetsConfiguredResourceReserve(smCHAR *caster, DWORD skillCode, int point,
    int reservePercent, CompanionSkillAttempt *attempt)
{
    if(!caster || reservePercent <= 0)
        return true;
    const sSKILL_INFO *info = FindSkillInfo(skillCode);
    if(!info)
        return true;
    reservePercent = std::max(0, std::min(90, reservePercent));
    const int mana = ManaCost(info, point);
    const int stamina = StaminaCost(info, point);
    const int manaReserve = caster->smCharInfo.Mana[1] > 0 ?
        ((int)caster->smCharInfo.Mana[1] * reservePercent + 99) / 100 : 0;
    const int staminaReserve = caster->smCharInfo.Stamina[1] > 0 ?
        ((int)caster->smCharInfo.Stamina[1] * reservePercent + 99) / 100 : 0;
    if(attempt)
    {
        attempt->manaCost = mana;
        attempt->staminaCost = stamina;
        attempt->manaReserve = manaReserve;
        attempt->staminaReserve = staminaReserve;
    }
    if(mana > 0 && (int)caster->smCharInfo.Mana[0] - mana < manaReserve)
    {
        if(attempt) attempt->reason = COMPANION_SKILL_MP_RESERVE;
        return false;
    }
    if(stamina > 0 && (int)caster->smCharInfo.Stamina[0] - stamina < staminaReserve)
    {
        if(attempt) attempt->reason = COMPANION_SKILL_STAMINA_RESERVE;
        return false;
    }
    return true;
}

bool TryExecuteCompanionDamageLoadout(smCHAR *caster, rsPLAYINFO *owner, smCHAR *target,
    DWORD rightItemCode, DWORD leftItemCode, const BYTE activeSkillSlots[4],
    const BYTE activeMinResourcePercent[4], const BYTE activeMinimumTargets[4],
    const BYTE activeTargetModes[4], bool loadoutActive, CompanionSkillRuntime &runtime,
    DWORD now, CompanionSkillAttempt *attempt)
{
    if(!loadoutActive)
        return TryExecuteCompanionDamageSkill(caster, owner, target, rightItemCode, leftItemCode,
            runtime, now, attempt);

    if(attempt) attempt->Reset();
    if(!caster || !owner || !activeSkillSlots)
    {
        if(attempt) attempt->reason = COMPANION_SKILL_INVALID;
        return false;
    }
    if(runtime.channelSkillCode)
    {
        if(runtime.channelEndTime && (int)(runtime.channelEndTime - now) <= 0)
            StopCompanionChannel(runtime);
        else if(runtime.channelPoint >= 1 && runtime.channelPoint <= 10)
        {
            CompanionSkillAttempt channelAttempt;
            channelAttempt.skillCode = runtime.channelSkillCode;
            channelAttempt.point = runtime.channelPoint;
            if(ExecuteCompanionChannelTick(caster, owner, target, runtime.channelSkillCode,
                runtime.channelPoint, runtime, now, &channelAttempt, true))
            {
                if(attempt) *attempt = channelAttempt;
                return true;
            }
            if(channelAttempt.reason == COMPANION_SKILL_COOLDOWN ||
               channelAttempt.reason == COMPANION_SKILL_LOCKED)
            {
                if(attempt) *attempt = channelAttempt;
                return false;
            }
        }
    }
    if(runtime.nextDamageSkillTime && (int)(runtime.nextDamageSkillTime - now) > 0)
    {
        if(attempt) attempt->reason = COMPANION_SKILL_COOLDOWN;
        return false;
    }
    if(IsCompanionSkillLocked(runtime, now))
    {
        if(attempt) attempt->reason = COMPANION_SKILL_LOCKED;
        return false;
    }

    CompanionSkillAttempt lastAttempt;
    bool hadConfiguredSkill = false;
    // V30: slot order is the explicit priority contract; slot 1 is highest.
    for(int slot = 0; slot < 4; ++slot)
    {
        const int useSlot = activeSkillSlots[slot];
        if(useSlot <= 0)
            continue;

        CompanionSkillDescriptor descriptor;
        if(!GetCompanionSkillDescriptor(caster->smCharInfo.JOB_CODE, runtime.skillPoints,
            useSlot, &descriptor) || descriptor.point <= 0 || !IsOffensiveSkillCode(descriptor.skillCode))
        {
            lastAttempt.Reset();
            lastAttempt.skillCode = descriptor.skillCode;
            lastAttempt.point = descriptor.point;
            lastAttempt.reason = descriptor.point <= 0 ? COMPANION_SKILL_NO_POINT : COMPANION_SKILL_NO_JOB_SKILL;
            continue;
        }
        hadConfiguredSkill = true;
        CompanionSkillAttempt current;
        current.skillCode = descriptor.skillCode;
        current.point = descriptor.point;

        const int targetMode = activeTargetModes ? std::max(0, std::min(2, (int)activeTargetModes[slot])) : 0;
        smCHAR *slotTarget = ResolveConfiguredLoadoutTarget(caster, owner, target, targetMode);
        if(!slotTarget)
        {
            current.reason = COMPANION_SKILL_NO_TARGET;
            lastAttempt = current;
            continue;
        }

        const int minTargets = activeMinimumTargets ?
            std::max(1, std::min(16, (int)activeMinimumTargets[slot])) : 1;
        if(CountConfiguredSkillTargets(caster, slotTarget, descriptor.skillCode, descriptor.point) < minTargets)
        {
            current.reason = COMPANION_SKILL_MIN_TARGETS;
            lastAttempt = current;
            continue;
        }

        const int minResource = activeMinResourcePercent ?
            std::max(0, std::min(90, (int)activeMinResourcePercent[slot])) : 0;
        if(!MeetsConfiguredResourceReserve(caster, descriptor.skillCode, descriptor.point, minResource, &current))
        {
            lastAttempt = current;
            continue;
        }

        if(TryDamageCode(caster, owner, slotTarget, rightItemCode, leftItemCode,
            descriptor.skillCode, descriptor.point, runtime, now, &current))
        {
            runtime.activeSkillCursor = (BYTE)slot;
            if(attempt) *attempt = current;
            return true;
        }
        lastAttempt = current;
    }

    if(attempt)
    {
        if(hadConfiguredSkill)
            *attempt = lastAttempt;
        else
            attempt->reason = COMPANION_SKILL_NO_JOB_SKILL;
    }
    return false;
}

void UpdateCompanionBuffModifiers(smCHAR *caster, CompanionSkillRuntime &runtime, DWORD now)
{
    for(int i = 0; i < 4; ++i)
    {
        CompanionBuffModifier &modifier = runtime.buffModifiers[i];
        if(modifier.skillCode && modifier.expiresAt && (int)(now - modifier.expiresAt) >= 0)
            RemoveBuffModifier(caster, modifier);
    }
}

bool CanMonsterSeeCompanionWithVanish(smCHAR *companion, smCHAR *monster,
    CompanionSkillRuntime &runtime, DWORD now)
{
    if(!companion || !monster)
        return true;
    UpdateCompanionBuffModifiers(companion, runtime, now);
    const int point = ActiveVanishPoint(runtime, now);
    return point <= 0 || IsWithinVanishSight(companion, monster, point);
}

void ApplyCompanionIncomingDamageModifiers(smCHAR *target, smCHAR *attacker, rsPLAYINFO *owner,
    CompanionSkillRuntime &runtime, int *damage, DWORD now)
{
    if(!target || !damage || *damage <= 0)
        return;

    UpdateCompanionBuffModifiers(target, runtime, now);

    // Vanish reduces monster detection rather than absorbing damage. If a
    // monster retained a companion target from an earlier retaliation event
    // outside the reduced sight radius, drop that target before resolving the
    // hit. Boss exceptions match the legacy player scan.
    const int vanishPoint = ActiveVanishPoint(runtime, now);
    if(vanishPoint > 0 && attacker && !IsWithinVanishSight(target, attacker, vanishPoint))
    {
        combat::legacy::ServerLegacyTargetingAdapter().ReleaseTarget(
            EntityId(static_cast<std::uint32_t>(attacker->dwObjectSerial)),
            EntityId(static_cast<std::uint32_t>(target->dwObjectSerial)),
            combat::TargetReleaseMode::AcquiredAndForced);
        *damage = 0;
        return;
    }

    // Holy Reflection runs when an Undead attacks, before the normal hit/block
    // resolution in the legacy monster attack loop. Its retaliation uses the
    // attacker's own damage roll, Bionic resistance and then Absorption.
    if(attacker && owner && attacker->Flag && attacker->smCharInfo.Life[0] > 0 &&
       attacker->smCharInfo.Brood == smCHAR_MONSTER_UNDEAD)
    {
        for(int i = 0; i < 4; ++i)
        {
            const int percent = runtime.buffModifiers[i].holyReflectionPercent;
            if(percent <= 0)
                continue;
            int reflected = RandomBetween(attacker->smCharInfo.Attack_Damage[0],
                attacker->smCharInfo.Attack_Damage[1]);
            reflected = (reflected * percent) / 100;
            int resistance = (int)attacker->smCharInfo.Resistance[sITEMINFO_BIONIC];
            resistance = std::max(-100, std::min(100, resistance));
            reflected -= (reflected * resistance) / 100;
            if(reflected > 0)
                ApplySkillDamage(target, owner, attacker, reflected);
            break;
        }
    }

    // Summon Muspell is both an autonomous familiar and a defensive aura.
    // Preserve the legacy order: its block roll happens before normal shield
    // block, and its Undead reduction applies to hits that were not blocked.
    if(runtime.persistentEffectSkillCode == SKILL_SUMMON_MUSPELL &&
       runtime.persistentEffectExpiresAt && (int)(runtime.persistentEffectExpiresAt - now) > 0 &&
       runtime.persistentEffectPoint >= 1 && runtime.persistentEffectPoint <= 10)
    {
        const int p = runtime.persistentEffectPoint - 1;
        const int muspellBlock = ClampPercent(Summon_Muspell_BlockPercent[p]);
        if(muspellBlock > 0 && (rand() % 100) < muspellBlock)
        {
            *damage = 0;
            return;
        }
        if(attacker && attacker->smCharInfo.Brood == smCHAR_MONSTER_UNDEAD)
            *damage -= (*damage * ClampPercent(Summon_Muspell_UndeadAbsorbPercent[p])) / 100;
        if(*damage <= 0)
        {
            *damage = 0;
            return;
        }
    }

    const int hallEvasion = ActiveHallValhallaEvasion(runtime, now);
    if(hallEvasion > 0 && (rand() % 100) < hallEvasion)
    {
        *damage = 0;
        return;
    }

    // Companions do not traverse the player-client block path. Reproduce the
    // same server-authoritative roll here so Extreme Shield/native shield block
    // and Divine Inhalation are meaningful. Divine Inhalation only contributes
    // its extra block/heal semantics against Undead attackers.
    int blockChance = (int)target->smCharInfo.Chance_Block;
    int divinePoint = 0;
    if(attacker && attacker->smCharInfo.Brood == smCHAR_MONSTER_UNDEAD)
    {
        for(int i = 0; i < 4; ++i)
        {
            const int point = runtime.buffModifiers[i].divineInhalationPoint;
            if(point >= 1 && point <= 10)
            {
                divinePoint = point;
                blockChance += D_Inhalation_Block[point - 1];
                break;
            }
        }
    }
    if(blockChance > 0)
    {
        if(attacker)
            blockChance += ((int)target->smCharInfo.Level - (int)attacker->smCharInfo.Level) * 25 / 100;
        blockChance = std::max(0, std::min(50, blockChance));
        if(blockChance > 0 && (rand() % 100) < blockChance)
        {
            if(divinePoint > 0 && attacker)
            {
                int resistance = (int)attacker->smCharInfo.Resistance[sITEMINFO_BIONIC];
                resistance = std::max(-100, std::min(100, resistance));
                int heal = (D_Inhalation_Life[divinePoint - 1] * resistance) / 100;
                heal &= 0xFF; // preserve legacy AttackState low-byte encoding
                if(heal > 0)
                    target->smCharInfo.Life[0] = (short)std::min((int)target->smCharInfo.Life[1],
                        (int)target->smCharInfo.Life[0] + heal);
            }

            // Spark Shield is triggered by the block event, not by every hit.
            if(attacker && owner && attacker->Flag && attacker->smCharInfo.Life[0] > 0)
            {
                for(int i = 0; i < 4; ++i)
                {
                    int sparkDamage = runtime.buffModifiers[i].sparkShieldDamage;
                    if(sparkDamage <= 0)
                        continue;
                    if(attacker->smCharInfo.Brood == smCHAR_MONSTER_MECHANIC)
                        sparkDamage += (sparkDamage * 50) / 100;
                    int resistance = (int)attacker->smCharInfo.Resistance[sITEMINFO_LIGHTING];
                    resistance = std::max(-100, std::min(100, resistance));
                    sparkDamage -= (sparkDamage * resistance) / 100;
                    if(sparkDamage > 0)
                        ApplySkillDamage(target, owner, attacker, sparkDamage);
                    break;
                }
            }
            *damage = 0;
            return;
        }
    }

    int value = *damage;
    for(int i = 0; i < 4 && value > 0; ++i)
    {
        CompanionBuffModifier &modifier = runtime.buffModifiers[i];
        if(!modifier.skillCode)
            continue;

        if(modifier.evasionPercent > 0 &&
           (rand() % 100) < ClampPercent(modifier.evasionPercent))
        {
            value = 0;
            break;
        }

        if(modifier.incomingFlatReduction > 0)
            value = std::max(0, value - modifier.incomingFlatReduction);

        if(value > 0 && modifier.undeadDamageReductionPercent > 0 && attacker &&
           attacker->smCharInfo.Brood == smCHAR_MONSTER_UNDEAD)
            value -= (value * ClampPercent(modifier.undeadDamageReductionPercent)) / 100;

        if(value > 0 && modifier.incomingDamageReductionPercent > 0)
            value -= (value * ClampPercent(modifier.incomingDamageReductionPercent)) / 100;

        if(value > 0 && modifier.energyShieldPercent > 0)
        {
            const int absorbed = (value * ClampPercent(modifier.energyShieldPercent)) / 100;
            value -= absorbed;
            target->smCharInfo.Mana[0] = (short)std::max(0,
                (int)target->smCharInfo.Mana[0] - absorbed);
        }
    }

    *damage = std::max(0, value);
}


void ApplyCompanionOutgoingBasicDamageModifiers(smCHAR *caster, smCHAR *target,
    DWORD rightItemCode, CompanionSkillRuntime &runtime, int *damage, DWORD now)
{
    if(!caster || !target || !damage || *damage <= 0)
        return;

    UpdateCompanionBuffModifiers(caster, runtime, now);
    const int valhallaBonus = ActiveValhallaMaxDamageBonus(runtime, now);
    int value = valhallaBonus > 0
        ? RandomBetween(caster->smCharInfo.Attack_Damage[0], caster->smCharInfo.Attack_Damage[1] + valhallaBonus)
        : *damage;
    const int vanishPoint = ActiveVanishPoint(runtime, now);
    if(vanishPoint > 0)
    {
        value += (value * Vanish_Damage[vanishPoint - 1]) / 100;
        ConsumeCompanionVanish(caster, runtime);
    }
    value += ActiveOutgoingFlatDamage(runtime) + ActiveForceNatureDamage(runtime, now);
    value = ApplyEnchantWeaponBonus(runtime, target, value, now);
    // Legacy Falcon sends an additional non-critical damage event whenever the
    // Archer performs an offensive action. Folding the raw Falcon roll into the
    // same pre-absorption basic event preserves the authoritative total while
    // the client-side Falcon effect remains responsible only for presentation.
    value += CompanionFalconDamage(runtime, now);

    if((rightItemCode & sinITEM_MASK2) == sinWT1)
    {
        for(int i = 0; i < 4; ++i)
        {
            const int point = runtime.buffModifiers[i].frostJavelinPoint;
            if(point < 1 || point > 10)
                continue;
            const int p = point - 1;
            const int iceRoll = RandomBetween(Frost_Javelin_IceAddDamage[p][0],
                Frost_Javelin_IceAddDamage[p][1]);
            int resistance = (int)target->smCharInfo.Resistance[sITEMINFO_ICE];
            resistance = std::max(-100, std::min(100, resistance));
            int iceDamage = iceRoll - (iceRoll * resistance) / 100;
            if(iceDamage < 0) iceDamage = 0;
            value += iceDamage;

            combat::legacy::ServerLegacyStatusEffectAdapter().ApplyElementalSlow16Hz(
                EntityId(static_cast<std::uint32_t>(target->dwObjectSerial)),
                Frost_Javelin_IceTime[p],
                240 - (240 * Frost_Javelin_SpeedSubPercent[p]) / 100, resistance);
            break;
        }
    }

    *damage = std::max(0, value);
}

void ApplyCompanionOutgoingBasicPostAbsorptionModifiers(smCHAR *caster, smCHAR *target,
    CompanionSkillRuntime &runtime, int *damage, DWORD now)
{
    if(!caster || !target || !damage || *damage <= 0)
        return;
    const int holyValor = ActiveHolyValorPercent(runtime, now);
    if(holyValor > 0 && target->smCharInfo.Brood == smCHAR_MONSTER_UNDEAD)
        *damage += (*damage * holyValor) / 100;
}

void ResetCompanionBuffModifier(smCHAR *caster, CompanionSkillRuntime &runtime, int slot)
{
    if(slot < 0 || slot >= 4)
        return;
    RemoveBuffModifier(caster, runtime.buffModifiers[slot]);
    runtime.nextBuffSkillTime[slot] = 0;
}

void ResetAllCompanionBuffModifiers(smCHAR *caster, CompanionSkillRuntime &runtime)
{
    for(int i = 0; i < 4; ++i)
        ResetCompanionBuffModifier(caster, runtime, i);
}

static int ApplyCompanionCompulsionTaunt(smCHAR *caster, int point, DWORD now)
{
    if(!caster || point < 1 || point > 10)
        return 0;
    const int p = point - 1;
    const int range = std::max(1, Compulsion_Area[p]);
    const int rangeSquared = range * range;
    const DWORD expiresAt = now + (DWORD)std::max(1, Compulsion_Time[p]) * 1000U;
    int affected = 0;
    for(int i = 0; i < OTHER_PLAYER_MAX; ++i)
    {
        smCHAR *target = &chrOtherPlayer[i];
        if(!target->Flag || target == caster || target->smCharInfo.Life[0] <= 0 ||
           target->smCharInfo.State == smCHAR_STATE_NPC || target->smCharInfo.Brood == smCHAR_MONSTER_USER)
            continue;
        if(target->smCharInfo.dwCharSoundCode == snCHAR_SOUND_DEATHKNIGHT ||
           target->smCharInfo.dwCharSoundCode == snCHAR_SOUND_BABEL ||
           target->smCharInfo.dwCharSoundCode == snCHAR_SOUND_KELVEZU ||
           target->smCharInfo.dwCharSoundCode == snCHAR_SOUND_MOKOVA)
            continue;
        const int dx = (target->pX - caster->pX) >> FLOATNS;
        const int dy = (target->pY - caster->pY) >> FLOATNS;
        const int dz = (target->pZ - caster->pZ) >> FLOATNS;
        if(dx * dx + dz * dz > rangeSquared || abs(dy) >= 128)
            continue;
        const combat::TargetingResult targeting = combat::legacy::ServerLegacyTargetingAdapter().ForceTarget(
            EntityId(static_cast<std::uint32_t>(target->dwObjectSerial)),
            EntityId(static_cast<std::uint32_t>(caster->dwObjectSerial)),
            static_cast<std::uint32_t>(expiresAt));
        if(targeting.Ok())
            ++affected;
    }
    return affected;
}

bool TryExecuteCompanionBuffLoadout(smCHAR *caster, DWORD rightItemCode, DWORD leftItemCode,
    const BYTE buffSkillSlots[4], const WORD buffIntervalSeconds[4], bool loadoutActive,
    CompanionSkillRuntime &runtime, DWORD now, CompanionSkillAttempt *attempt)
{
    if(attempt) attempt->Reset();
    UpdateCompanionBuffModifiers(caster, runtime, now);
    if(!loadoutActive || !caster || !buffSkillSlots || !buffIntervalSeconds || caster->smCharInfo.Life[0] <= 0)
        return false;
    if(IsCompanionSkillLocked(runtime, now))
    {
        if(attempt) attempt->reason = COMPANION_SKILL_LOCKED;
        return false;
    }

    for(int slot = 0; slot < 4; ++slot)
    {
        const int useSlot = buffSkillSlots[slot];
        if(useSlot <= 0)
            continue;
        if(runtime.nextBuffSkillTime[slot] && (int)(runtime.nextBuffSkillTime[slot] - now) > 0)
            continue;

        CompanionSkillDescriptor descriptor;
        if(!GetCompanionSkillDescriptor(caster->smCharInfo.JOB_CODE, runtime.skillPoints,
            useSlot, &descriptor) || descriptor.point <= 0 || !IsBuffSkillCode(descriptor.skillCode))
        {
            runtime.nextBuffSkillTime[slot] = now + 5000U;
            if(attempt)
            {
                attempt->skillCode = descriptor.skillCode;
                attempt->point = descriptor.point;
                attempt->reason = descriptor.point <= 0 ? COMPANION_SKILL_NO_POINT : COMPANION_SKILL_NO_JOB_SKILL;
            }
            continue;
        }

        const sSKILL_INFO *info = NULL;
        int mana = 0;
        int stamina = 0;
        CompanionSkillAttempt current;
        if(!CanCast(caster, rightItemCode, leftItemCode, descriptor.skillCode, descriptor.point,
            &info, &mana, &stamina, &current, false))
        {
            runtime.nextBuffSkillTime[slot] = now + 1000U;
            if(attempt) *attempt = current;
            continue;
        }

        if(!ApplyBuffModifier(caster, runtime.buffModifiers[slot], descriptor.skillCode, descriptor.point, now, &runtime))
        {
            current.reason = COMPANION_SKILL_EFFECT_FAILED;
            runtime.nextBuffSkillTime[slot] = now + 5000U;
            if(attempt) *attempt = current;
            continue;
        }

        caster->smCharInfo.Mana[0] = (short)std::max(0, (int)caster->smCharInfo.Mana[0] - mana);
        caster->smCharInfo.Stamina[0] = (short)std::max(0, (int)caster->smCharInfo.Stamina[0] - stamina);
        BeginCast(caster, runtime, descriptor.skillCode, descriptor.point, 0, now, 700U);
        int interval = (int)buffIntervalSeconds[slot];
        if(interval < 5) interval = 5;
        if(interval > 3600) interval = 3600;
        runtime.nextBuffSkillTime[slot] = now + (DWORD)interval * 1000U;
        current.reason = COMPANION_SKILL_OK;
        current.skillCode = descriptor.skillCode;
        current.point = descriptor.point;
        if(attempt) *attempt = current;
        return true;
    }
    return false;
}

bool TryExecuteCompanionSupportLoadout(smCHAR *caster, rsPLAYINFO *owner,
    smCHAR *const *companions, CompanionSkillRuntime *const *companionRuntimes, int companionCount,
    DWORD rightItemCode, DWORD leftItemCode,
    const BYTE supportSkillSlots[4], const WORD supportIntervalSeconds[4], bool loadoutActive,
    CompanionSkillRuntime &runtime, DWORD now, CompanionSkillAttempt *attempt)
{
    if(attempt) attempt->Reset();
    UpdateCompanionBuffModifiers(caster, runtime, now);
    if(!loadoutActive || !caster || !supportSkillSlots || !supportIntervalSeconds || caster->smCharInfo.Life[0] <= 0)
        return false;
    if(IsCompanionSkillLocked(runtime, now))
    {
        if(attempt) attempt->reason = COMPANION_SKILL_LOCKED;
        return false;
    }

    for(int slot = 0; slot < 4; ++slot)
    {
        const int useSlot = supportSkillSlots[slot];
        if(useSlot <= 0)
            continue;
        if(runtime.nextBuffSkillTime[slot] && (int)(runtime.nextBuffSkillTime[slot] - now) > 0)
            continue;

        CompanionSkillDescriptor descriptor;
        if(!GetCompanionSkillDescriptor(caster->smCharInfo.JOB_CODE, runtime.skillPoints,
            useSlot, &descriptor) || descriptor.point <= 0 || !IsSupportSkillCode(descriptor.skillCode))
        {
            runtime.nextBuffSkillTime[slot] = now + 5000U;
            if(attempt)
            {
                attempt->skillCode = descriptor.skillCode;
                attempt->point = descriptor.point;
                attempt->reason = descriptor.point <= 0 ? COMPANION_SKILL_NO_POINT : COMPANION_SKILL_NO_JOB_SKILL;
            }
            continue;
        }

        CompanionSkillAttempt current;
        current.skillCode = descriptor.skillCode;
        current.point = descriptor.point;
        const int intervalSeconds = std::max(5, std::min(3600, (int)supportIntervalSeconds[slot]));

        if(descriptor.skillCode == SKILL_RESURRECTION)
        {
            smCHAR *deadTarget = NULL;
            int bestDistance = 300 * 300;
            for(int i = 0; i < companionCount; ++i)
            {
                smCHAR *candidate = companions ? companions[i] : NULL;
                if(!candidate || candidate == caster || !candidate->Flag || candidate->smCharInfo.Life[0] > 0 ||
                   candidate->smCharInfo.Life[1] <= 0 || candidate->lpMasterPlayInfo != owner)
                    continue;
                const int dx = (candidate->pX - caster->pX) >> FLOATNS;
                const int dy = (candidate->pY - caster->pY) >> FLOATNS;
                const int dz = (candidate->pZ - caster->pZ) >> FLOATNS;
                const int distance = dx * dx + dy * dy + dz * dz;
                if(distance < bestDistance)
                {
                    bestDistance = distance;
                    deadTarget = candidate;
                }
            }
            if(!deadTarget)
            {
                runtime.nextBuffSkillTime[slot] = now + 1000U;
                current.reason = COMPANION_SKILL_NOT_NEEDED;
                if(attempt) *attempt = current;
                continue;
            }
            const sSKILL_INFO *info = NULL;
            int mana = 0, stamina = 0;
            if(!CanCast(caster, rightItemCode, leftItemCode, descriptor.skillCode, descriptor.point,
                &info, &mana, &stamina, &current, false))
            {
                runtime.nextBuffSkillTime[slot] = now + 1000U;
                if(attempt) *attempt = current;
                continue;
            }
            caster->smCharInfo.Mana[0] = (short)std::max(0, (int)caster->smCharInfo.Mana[0] - mana);
            caster->smCharInfo.Stamina[0] = (short)std::max(0, (int)caster->smCharInfo.Stamina[0] - stamina);
            const bool resurrected = (rand() % 100) < Resurrection_Percent[descriptor.point - 1];
            if(resurrected)
            {
                deadTarget->smCharInfo.Life[0] = (short)std::max(1, (int)deadTarget->smCharInfo.Life[1] / 2);
                combat::legacy::ServerLegacyTargetingAdapter().ClearAcquiredTarget(
                    EntityId(static_cast<std::uint32_t>(deadTarget->dwObjectSerial)));
                deadTarget->SetMotionFromCode(CHRMOTION_STATE_RESTART);
            }
            BeginCast(caster, runtime, descriptor.skillCode, descriptor.point, deadTarget->dwObjectSerial, now, 750U,
                deadTarget->pX, deadTarget->pY, deadTarget->pZ);
            runtime.nextBuffSkillTime[slot] = now + (DWORD)intervalSeconds * 1000U;
            current.reason = COMPANION_SKILL_OK;
            if(attempt) *attempt = current;
            return true;
        }

        if(descriptor.skillCode == SKILL_FORCE_OF_NATURE || descriptor.skillCode == SKILL_HOLY_VALOR ||
           descriptor.skillCode == SKILL_TRIUMPH_OF_VALHALLA || descriptor.skillCode == SKILL_HALL_OF_VALHALLA ||
           descriptor.skillCode == SKILL_REGENERATION_FIELD || descriptor.skillCode == SKILL_ENCHANT_WEAPON ||
           descriptor.skillCode == SKILL_MAGNETIC_SPHERE || descriptor.skillCode == SKILL_DANCING_SWORD ||
           descriptor.skillCode == SKILL_SUMMON_MUSPELL || descriptor.skillCode == SKILL_METAL_GOLEM ||
           descriptor.skillCode == SKILL_RECALL_WOLVERIN || descriptor.skillCode == SKILL_FIRE_ELEMENTAL ||
           descriptor.skillCode == SKILL_FALCON || descriptor.skillCode == SKILL_GOLDEN_FALCON ||
           descriptor.skillCode == SKILL_COMPULSION)
        {
            const sSKILL_INFO *info = NULL;
            int mana = 0, stamina = 0;
            if(!CanCast(caster, rightItemCode, leftItemCode, descriptor.skillCode, descriptor.point,
                &info, &mana, &stamina, &current, false))
            {
                runtime.nextBuffSkillTime[slot] = now + 1000U;
                if(attempt) *attempt = current;
                continue;
            }

            const int p = descriptor.point - 1;
            int visualParam = 0;
            smCHAR *spawnedSkillPet = NULL;
            if(descriptor.skillCode == SKILL_FALCON || descriptor.skillCode == SKILL_GOLDEN_FALCON)
            {
                int falconPoint = descriptor.skillCode == SKILL_FALCON ? descriptor.point :
                    LearnedPointForCode(caster->smCharInfo.JOB_CODE, runtime.skillPoints, SKILL_FALCON);
                if(falconPoint < 1 || falconPoint > 10)
                {
                    current.reason = COMPANION_SKILL_EFFECT_FAILED;
                    runtime.nextBuffSkillTime[slot] = now + 5000U;
                    if(attempt) *attempt = current;
                    continue;
                }
                runtime.falconPoint = falconPoint;
                runtime.goldenFalconPoint = descriptor.skillCode == SKILL_GOLDEN_FALCON ? descriptor.point : 0;
                runtime.falconExpiresAt = now + (DWORD)std::max(1, descriptor.skillCode == SKILL_GOLDEN_FALCON ?
                    Golden_Falcon_Time[p] : Falcon_Time[falconPoint - 1]) * 1000U;
                runtime.falconDamageMin = Falcon_Damage[falconPoint - 1][0];
                runtime.falconDamageMax = Falcon_Damage[falconPoint - 1][1];
                if(descriptor.skillCode == SKILL_GOLDEN_FALCON)
                {
                    runtime.falconDamageMin = (runtime.falconDamageMin * Golden_Falcon_Damage[p]) / 100;
                    runtime.falconDamageMax = (runtime.falconDamageMax * Golden_Falcon_Damage[p]) / 100;
                }
            }
            else if(descriptor.skillCode == SKILL_COMPULSION)
            {
                if(!ApplyBuffModifier(caster, runtime.buffModifiers[slot], descriptor.skillCode, descriptor.point, now, &runtime))
                {
                    current.reason = COMPANION_SKILL_EFFECT_FAILED;
                    runtime.nextBuffSkillTime[slot] = now + 5000U;
                    if(attempt) *attempt = current;
                    continue;
                }
                ApplyCompanionCompulsionTaunt(caster, descriptor.point, now);
            }
            else if(descriptor.skillCode == SKILL_METAL_GOLEM || descriptor.skillCode == SKILL_RECALL_WOLVERIN ||
               descriptor.skillCode == SKILL_FIRE_ELEMENTAL)
            {
                spawnedSkillPet = OpenMonsterFromSkill(SkillPlayCode(descriptor.skillCode),
                    caster->pX, caster->pY, caster->pZ, owner, caster);
                if(!spawnedSkillPet)
                {
                    current.reason = COMPANION_SKILL_EFFECT_FAILED;
                    runtime.nextBuffSkillTime[slot] = now + 5000U;
                    if(attempt) *attempt = current;
                    continue;
                }
                spawnedSkillPet->dwUpdateCharInfoTime = now + 355U * 1000U;
                if(descriptor.skillCode == SKILL_METAL_GOLEM)
                {
                    spawnedSkillPet->smCharInfo.Attack_Damage[0] = Metal_Golem_Damage[p][0];
                    spawnedSkillPet->smCharInfo.Attack_Damage[1] = Metal_Golem_Damage[p][1];
                    spawnedSkillPet->smCharInfo.Attack_Rating = Metal_Golem_Hit[p];
                    spawnedSkillPet->smCharInfo.Defence = Metal_Golem_Defense[p];
                    spawnedSkillPet->smCharInfo.Life[0] = (short)(Metal_Golem_Life[p] + caster->smCharInfo.Strength * 2);
                    spawnedSkillPet->smCharInfo.Life[1] = spawnedSkillPet->smCharInfo.Life[0];
                    spawnedSkillPet->sMoveStepCount[0] = 1;
                    spawnedSkillPet->sMoveStepCount[1] = 2;
                    spawnedSkillPet->smCharInfo.wPlayClass[1] = fONE + p * 15;
                }
                else if(descriptor.skillCode == SKILL_RECALL_WOLVERIN)
                {
                    spawnedSkillPet->smCharInfo.Attack_Damage[0] = Recall_Wolverin_Damage[p][0];
                    spawnedSkillPet->smCharInfo.Attack_Damage[1] = Recall_Wolverin_Damage[p][1];
                    spawnedSkillPet->smCharInfo.Attack_Rating = Recall_Wolverin_Hit[p];
                    spawnedSkillPet->smCharInfo.Defence = Recall_Wolverin_Defense[p] + caster->smCharInfo.Dexterity * 2;
                    spawnedSkillPet->smCharInfo.Life[0] = (short)Recall_Wolverin_Life[p];
                    spawnedSkillPet->smCharInfo.Life[1] = spawnedSkillPet->smCharInfo.Life[0];
                    spawnedSkillPet->sMoveStepCount[0] = 1;
                    spawnedSkillPet->sMoveStepCount[1] = 2;
                    spawnedSkillPet->smCharInfo.wPlayClass[1] = (fONE / 2) + p * 12;
                }
                else
                {
                    spawnedSkillPet->smCharInfo.Attack_Damage[0] = Fire_Elemental_Damage[p][0];
                    spawnedSkillPet->smCharInfo.Attack_Damage[1] = Fire_Elemental_Damage[p][1];
                    spawnedSkillPet->smCharInfo.Attack_Rating = Fire_Elemental_Hit[p];
                    spawnedSkillPet->smCharInfo.Life[0] = (short)Fire_Elemental_Life[p];
                    spawnedSkillPet->smCharInfo.Life[1] = spawnedSkillPet->smCharInfo.Life[0];
                    spawnedSkillPet->sMoveStepCount[0] = 1;
                    spawnedSkillPet->sMoveStepCount[1] = 2;
                }
            }
            else if(descriptor.skillCode == SKILL_MAGNETIC_SPHERE || descriptor.skillCode == SKILL_DANCING_SWORD ||
               descriptor.skillCode == SKILL_SUMMON_MUSPELL)
            {
                runtime.persistentEffectSkillCode = descriptor.skillCode;
                runtime.persistentEffectPoint = descriptor.point;
                runtime.persistentEffectElement = 0;
                if(descriptor.skillCode == SKILL_MAGNETIC_SPHERE)
                {
                    runtime.persistentEffectExpiresAt = now + (DWORD)std::max(1, Magnetic_Sphere_Time[p]) * 1000U;
                    runtime.persistentEffectNextAttackAt = now + (DWORD)std::max(1, Magnetic_Sphere_AttackDelay[p]) * 1000U;
                }
                else if(descriptor.skillCode == SKILL_DANCING_SWORD)
                {
                    runtime.persistentEffectExpiresAt = now + (DWORD)std::max(1, Dancing_Sword_Time[p]) * 1000U;
                    runtime.persistentEffectNextAttackAt = now + (DWORD)std::max(1, Dancing_Sword_AttackDelay[p]) * 900U;
                    runtime.persistentEffectElement = rand() % 2;
                    visualParam = runtime.persistentEffectElement;
                }
                else
                {
                    runtime.persistentEffectExpiresAt = now + (DWORD)std::max(1, Summon_Muspell_Time[p]) * 1000U;
                    runtime.persistentEffectNextAttackAt = now + (DWORD)std::max(1, Summon_Muspell_Attack_Delay[p]) * 1000U;
                }
            }
            else if(descriptor.skillCode == SKILL_ENCHANT_WEAPON)
            {
                runtime.enchantWeaponExpiresAt = now + (DWORD)std::max(1, Enchant_Weapon_Time[p]) * 1000U;
                runtime.enchantWeaponPoint = descriptor.point;
                runtime.enchantWeaponElement = rand() % 3;
                visualParam = runtime.enchantWeaponElement;
            }
            else if(descriptor.skillCode == SKILL_REGENERATION_FIELD)
            {
                runtime.regenerationFieldExpiresAt = now + (DWORD)std::max(1, Regeneration_Field_Time[p]) * 1000U;
                runtime.regenerationFieldPoint = descriptor.point;
                runtime.regenerationFieldNextPulseAt = 0;
            }
            else if(descriptor.skillCode == SKILL_FORCE_OF_NATURE)
            {
                const DWORD expiresAt = now + (DWORD)std::max(1, Force_Of_Nature_Time[p]) * 1000U;
                runtime.forceNatureExpiresAt = expiresAt;
                runtime.forceNatureDamage = Force_Of_Nature_AddDamage[p];
                runtime.forceNatureFalconDamage = Force_Of_Nature_FalconAddDamage[p];

                const int rangeSquared = 500 * 500;
                if(owner && owner->smCharInfo.Life[0] > 0)
                {
                    const int dx = (owner->Position.x - caster->pX) >> FLOATNS;
                    const int dy = (owner->Position.y - caster->pY) >> FLOATNS;
                    const int dz = (owner->Position.z - caster->pZ) >> FLOATNS;
                    if(dx * dx + dz * dz < rangeSquared && abs(dy) < 80)
                    {
                        owner->dwSkill_ForceOfNature_Time = expiresAt;
                        owner->wSkill_ForceOfNature_Param[0] = (WORD)(Force_Of_Nature_AddDamage[p] / 2);
                        owner->wSkill_ForceOfNature_Param[1] = (WORD)Force_Of_Nature_FalconAddDamage[p];
                    }
                }
                for(int i = 0; i < companionCount; ++i)
                {
                    smCHAR *candidate = companions ? companions[i] : NULL;
                    CompanionSkillRuntime *candidateRuntime = companionRuntimes ? companionRuntimes[i] : NULL;
                    if(!candidate || !candidateRuntime || candidate == caster || !candidate->Flag || candidate->smCharInfo.Life[0] <= 0)
                        continue;
                    const int dx = (candidate->pX - caster->pX) >> FLOATNS;
                    const int dy = (candidate->pY - caster->pY) >> FLOATNS;
                    const int dz = (candidate->pZ - caster->pZ) >> FLOATNS;
                    if(dx * dx + dz * dz >= rangeSquared || abs(dy) >= 80)
                        continue;
                    candidateRuntime->forceNatureExpiresAt = expiresAt;
                    candidateRuntime->forceNatureDamage = Force_Of_Nature_AddDamage[p] / 2;
                    candidateRuntime->forceNatureFalconDamage = Force_Of_Nature_FalconAddDamage[p];
                }
            }
            else if(descriptor.skillCode == SKILL_HOLY_VALOR)
            {
                const DWORD expiresAt = now + (DWORD)std::max(1, Holy_Valor_Time[p]) * 1000U;
                runtime.holyValorExpiresAt = expiresAt;
                runtime.holyValorPercent = Holy_Valor_Damage[p];
                for(int b = 0; b < 4; ++b)
                    if(runtime.buffModifiers[b].skillCode == SKILL_HOLY_BODY)
                        RemoveBuffModifier(caster, runtime.buffModifiers[b]);
                if(owner)
                {
                    owner->dwSkill_HolyBody_Time = 0;
                    owner->dwSkill_HolyBody_Param = 0;
                    owner->dwSkill_HolyValor_Time = expiresAt;
                    owner->dwSkill_HolyValor_Param = Holy_Valor_Damage[p];
                }
                for(int i = 0; i < companionCount; ++i)
                {
                    smCHAR *candidate = companions ? companions[i] : NULL;
                    CompanionSkillRuntime *candidateRuntime = companionRuntimes ? companionRuntimes[i] : NULL;
                    if(!candidate || !candidateRuntime || !candidate->Flag || candidate->smCharInfo.Life[0] <= 0)
                        continue;
                    candidateRuntime->holyValorExpiresAt = expiresAt;
                    candidateRuntime->holyValorPercent = Holy_Valor_Damage[p];
                    for(int b = 0; b < 4; ++b)
                        if(candidateRuntime->buffModifiers[b].skillCode == SKILL_HOLY_BODY)
                            RemoveBuffModifier(candidate, candidateRuntime->buffModifiers[b]);
                }
            }
            else if(descriptor.skillCode == SKILL_TRIUMPH_OF_VALHALLA)
            {
                const int bonus = T_Of_Valhalla_Damage[p] + ((caster->smCharInfo.Level > 0 && caster->smCharInfo.Level < CHAR_LEVEL_MAX) ? caster->smCharInfo.Level / 4 : 0);
                const DWORD expiresAt = now + (DWORD)std::max(1, T_Of_Valhalla_Time[p]) * 1000U;
                runtime.triumphValhallaExpiresAt = expiresAt;
                runtime.triumphValhallaMaxDamageBonus = bonus;
                runtime.hallValhallaExpiresAt = 0;
                runtime.hallValhallaEvasionPercent = 0;
                runtime.hallValhallaMaxDamageBonus = 0;
                if(owner && (!owner->dwSkill_HallOfValhalla_Time || owner->dwSkill_HallOfValhalla_Time < now))
                {
                    owner->dwSkill_TriumphOfValhalla_Time = expiresAt;
                    owner->dwSkill_TriumphOfValhalla_Param = bonus / 2;
                }
                for(int i = 0; i < companionCount; ++i)
                {
                    smCHAR *candidate = companions ? companions[i] : NULL;
                    CompanionSkillRuntime *candidateRuntime = companionRuntimes ? companionRuntimes[i] : NULL;
                    if(!candidate || !candidateRuntime || candidate == caster || !candidate->Flag || candidate->smCharInfo.Life[0] <= 0)
                        continue;
                    if(candidateRuntime->hallValhallaExpiresAt && (int)(candidateRuntime->hallValhallaExpiresAt - now) > 0)
                        continue;
                    candidateRuntime->triumphValhallaExpiresAt = expiresAt;
                    candidateRuntime->triumphValhallaMaxDamageBonus = bonus / 2;
                }
            }
            else
            {
                const int triumphPoint = LearnedPointForCode(caster->smCharInfo.JOB_CODE, runtime.skillPoints, SKILL_TRIUMPH_OF_VALHALLA);
                if(triumphPoint < 1 || triumphPoint > 10)
                {
                    current.reason = COMPANION_SKILL_EFFECT_FAILED;
                    runtime.nextBuffSkillTime[slot] = now + 5000U;
                    if(attempt) *attempt = current;
                    continue;
                }
                const int levelBonus = (caster->smCharInfo.Level > 0 && caster->smCharInfo.Level < CHAR_LEVEL_MAX) ? caster->smCharInfo.Level / 4 : 0;
                const int maxBonus = T_Of_Valhalla_Damage[triumphPoint - 1] + levelBonus;
                const DWORD expiresAt = now + (DWORD)std::max(1, Hall_Of_Valhalla_Time[p]) * 1000U;
                runtime.hallValhallaExpiresAt = expiresAt;
                runtime.hallValhallaEvasionPercent = Hall_Of_Valhalla_AddEvation[p];
                runtime.hallValhallaMaxDamageBonus = maxBonus;
                runtime.triumphValhallaExpiresAt = 0;
                runtime.triumphValhallaMaxDamageBonus = 0;

                const int rangeSquared = 500 * 500;
                if(owner && (!owner->dwSkill_TriumphOfValhalla_Time || owner->dwSkill_TriumphOfValhalla_Time < now))
                {
                    const int dx = (owner->Position.x - caster->pX) >> FLOATNS;
                    const int dy = (owner->Position.y - caster->pY) >> FLOATNS;
                    const int dz = (owner->Position.z - caster->pZ) >> FLOATNS;
                    if(dx * dx + dz * dz < rangeSquared && abs(dy) < 80)
                    {
                        owner->dwSkill_HallOfValhalla_Time = expiresAt;
                        owner->wSkill_HallOfValhalla_Param[0] = (WORD)(Hall_Of_Valhalla_AddEvation[p] / 2);
                        owner->wSkill_HallOfValhalla_Param[1] = (WORD)(maxBonus / 2);
                    }
                }
                for(int i = 0; i < companionCount; ++i)
                {
                    smCHAR *candidate = companions ? companions[i] : NULL;
                    CompanionSkillRuntime *candidateRuntime = companionRuntimes ? companionRuntimes[i] : NULL;
                    if(!candidate || !candidateRuntime || candidate == caster || !candidate->Flag || candidate->smCharInfo.Life[0] <= 0)
                        continue;
                    if(candidateRuntime->triumphValhallaExpiresAt && (int)(candidateRuntime->triumphValhallaExpiresAt - now) > 0)
                        continue;
                    const int dx = (candidate->pX - caster->pX) >> FLOATNS;
                    const int dy = (candidate->pY - caster->pY) >> FLOATNS;
                    const int dz = (candidate->pZ - caster->pZ) >> FLOATNS;
                    if(dx * dx + dz * dz >= rangeSquared || abs(dy) >= 80)
                        continue;
                    candidateRuntime->hallValhallaExpiresAt = expiresAt;
                    candidateRuntime->hallValhallaEvasionPercent = Hall_Of_Valhalla_AddEvation[p] / 2;
                    candidateRuntime->hallValhallaMaxDamageBonus = maxBonus / 2;
                }
            }

            caster->smCharInfo.Mana[0] = (short)std::max(0, (int)caster->smCharInfo.Mana[0] - mana);
            caster->smCharInfo.Stamina[0] = (short)std::max(0, (int)caster->smCharInfo.Stamina[0] - stamina);
            BeginCast(caster, runtime, descriptor.skillCode, descriptor.point, caster->dwObjectSerial, now, 700U,
                caster->pX, caster->pY, caster->pZ, visualParam);
            runtime.nextBuffSkillTime[slot] = now + (DWORD)intervalSeconds * 1000U;
            current.reason = COMPANION_SKILL_OK;
            if(attempt) *attempt = current;
            return true;
        }

        if(IsBuffSkillCode(descriptor.skillCode))
        {
            const sSKILL_INFO *info = NULL;
            int mana = 0, stamina = 0;
            if(!CanCast(caster, rightItemCode, leftItemCode, descriptor.skillCode, descriptor.point,
                &info, &mana, &stamina, &current, false))
            {
                runtime.nextBuffSkillTime[slot] = now + 1000U;
                if(attempt) *attempt = current;
                continue;
            }
            if(!ApplyBuffModifier(caster, runtime.buffModifiers[slot], descriptor.skillCode, descriptor.point, now, &runtime))
            {
                current.reason = COMPANION_SKILL_EFFECT_FAILED;
                runtime.nextBuffSkillTime[slot] = now + 5000U;
                if(attempt) *attempt = current;
                continue;
            }
            caster->smCharInfo.Mana[0] = (short)std::max(0, (int)caster->smCharInfo.Mana[0] - mana);
            caster->smCharInfo.Stamina[0] = (short)std::max(0, (int)caster->smCharInfo.Stamina[0] - stamina);
            BeginCast(caster, runtime, descriptor.skillCode, descriptor.point, 0, now, 700U);
            runtime.nextBuffSkillTime[slot] = now + (DWORD)intervalSeconds * 1000U;
            current.reason = COMPANION_SKILL_OK;
            if(attempt) *attempt = current;
            return true;
        }

        if(descriptor.skillCode == SKILL_METAL_GOLEM || descriptor.skillCode == SKILL_RECALL_WOLVERIN ||
           descriptor.skillCode == SKILL_FIRE_ELEMENTAL || descriptor.skillCode == SKILL_T5_SUMMON_TIGER ||
           descriptor.skillCode == SKILL_T5_ICE_ELEMENTAL)
        {
            if(TryCompanionSummonSkill(caster, owner, descriptor.skillCode, descriptor.point,
                rightItemCode, leftItemCode, runtime, now, &current))
            {
                runtime.nextBuffSkillTime[slot] = now + (DWORD)intervalSeconds * 1000U;
                if(attempt) *attempt = current;
                return true;
            }
            runtime.nextBuffSkillTime[slot] = now + 1500U;
            if(attempt) *attempt = current;
            continue;
        }

        if(descriptor.skillCode == SKILL_AGONY)
        {
            // Conditional resource conversion. Preserve HP safety and avoid
            // burning the skill while mana is already healthy.
            if(caster->smCharInfo.Mana[1] <= 0 || caster->smCharInfo.Mana[0] * 100 > caster->smCharInfo.Mana[1] * 70)
            {
                runtime.nextBuffSkillTime[slot] = now + 2000U;
                current.reason = COMPANION_SKILL_NOT_NEEDED;
                if(attempt) *attempt = current;
                continue;
            }
            const sSKILL_INFO *info = NULL;
            int mana = 0, stamina = 0;
            if(!CanCast(caster, rightItemCode, leftItemCode, descriptor.skillCode, descriptor.point,
                &info, &mana, &stamina, &current, false))
            {
                runtime.nextBuffSkillTime[slot] = now + 1000U;
                if(attempt) *attempt = current;
                continue;
            }
            const int p = descriptor.point - 1;
            const int lifeCost = ((int)caster->smCharInfo.Life[1] * Agony_ConvLife[p]) / 100;
            const int manaGain = ((int)caster->smCharInfo.Mana[1] * Agony_RecoveryMana[p]) / 100;
            if(lifeCost <= 0 || manaGain <= 0 || (int)caster->smCharInfo.Life[0] - lifeCost <= 0)
            {
                runtime.nextBuffSkillTime[slot] = now + 2000U;
                current.reason = COMPANION_SKILL_NOT_NEEDED;
                if(attempt) *attempt = current;
                continue;
            }
            const combat::LifeCommandResult lifeResult = combat::legacy::ServerLegacyLifeAdapter().ConsumeNonLethal(
                EntityId(static_cast<std::uint32_t>(caster->dwObjectSerial)), lifeCost);
            if(!lifeResult.Ok())
            {
                runtime.nextBuffSkillTime[slot] = now + 2000U;
                current.reason = COMPANION_SKILL_NOT_NEEDED;
                if(attempt) *attempt = current;
                continue;
            }
            caster->smCharInfo.Mana[0] = (short)std::max(0, (int)caster->smCharInfo.Mana[0] - mana);
            caster->smCharInfo.Stamina[0] = (short)std::max(0, (int)caster->smCharInfo.Stamina[0] - stamina);
            caster->smCharInfo.Mana[0] = (short)std::min((int)caster->smCharInfo.Mana[1], (int)caster->smCharInfo.Mana[0] + manaGain);
            BeginCast(caster, runtime, descriptor.skillCode, descriptor.point, caster->dwObjectSerial, now, 700U);
            runtime.nextBuffSkillTime[slot] = now + (DWORD)intervalSeconds * 1000U;
            current.reason = COMPANION_SKILL_OK;
            if(attempt) *attempt = current;
            return true;
        }

        if(descriptor.skillCode == SKILL_RESURRECTION)
        {
            smCHAR *reviveTarget = NULL;
            for(int i = 0; i < companionCount; ++i)
            {
                smCHAR *candidate = companions ? companions[i] : NULL;
                if(candidate && candidate != caster && candidate->Flag && candidate->smCharInfo.Life[0] <= 0 &&
                   candidate->smCharInfo.Life[1] > 0 && InRange(caster, candidate, 320))
                {
                    reviveTarget = candidate;
                    break;
                }
            }
            if(!reviveTarget)
            {
                runtime.nextBuffSkillTime[slot] = now + 2000U;
                current.reason = COMPANION_SKILL_NOT_NEEDED;
                if(attempt) *attempt = current;
                continue;
            }
            const sSKILL_INFO *info = NULL;
            int mana = 0, stamina = 0;
            if(!CanCast(caster, rightItemCode, leftItemCode, descriptor.skillCode, descriptor.point,
                &info, &mana, &stamina, &current))
            {
                runtime.nextBuffSkillTime[slot] = now + 1000U;
                if(attempt) *attempt = current;
                continue;
            }
            const int p = descriptor.point - 1;
            const int percent = std::max(1, std::min(100, Resurrection_Percent[p]));
            const combat::LifeCommandResult reviveResult = combat::legacy::ServerLegacyLifeAdapter().Revive(
                EntityId(static_cast<std::uint32_t>(reviveTarget->dwObjectSerial)), percent);
            if(!reviveResult.Ok())
            {
                runtime.nextBuffSkillTime[slot] = now + 1000U;
                current.reason = COMPANION_SKILL_INVALID_TARGET;
                if(attempt) *attempt = current;
                continue;
            }
            caster->smCharInfo.Mana[0] = (short)std::max(0, (int)caster->smCharInfo.Mana[0] - mana);
            caster->smCharInfo.Stamina[0] = (short)std::max(0, (int)caster->smCharInfo.Stamina[0] - stamina);
            BeginCast(caster, runtime, descriptor.skillCode, descriptor.point, reviveTarget->dwObjectSerial,
                now, 900U, reviveTarget->pX, reviveTarget->pY, reviveTarget->pZ);
            runtime.nextBuffSkillTime[slot] = now + (DWORD)intervalSeconds * 1000U;
            current.reason = COMPANION_SKILL_OK;
            if(attempt) *attempt = current;
            return true;
        }

        if(descriptor.skillCode == SKILL_GRAND_HEALING)
        {
            const int p = descriptor.point - 1;
            const int range = std::max(1, Grand_Healing_Range[p]);
            bool needed = false;
            if(owner && owner->smCharInfo.Life[1] > 0 && owner->smCharInfo.Life[0] > 0)
            {
                const int dx = (owner->Position.x - caster->pX) >> FLOATNS;
                const int dz = (owner->Position.z - caster->pZ) >> FLOATNS;
                needed = owner->smCharInfo.Life[0] * 100 <= owner->smCharInfo.Life[1] * 85 && dx * dx + dz * dz <= range * range;
            }
            for(int i = 0; !needed && i < companionCount; ++i)
            {
                smCHAR *candidate = companions ? companions[i] : NULL;
                if(candidate && candidate->Flag && candidate->smCharInfo.Life[0] > 0 && candidate->smCharInfo.Life[1] > 0 &&
                   candidate->smCharInfo.Life[0] * 100 <= candidate->smCharInfo.Life[1] * 85 && InRange(caster, candidate, range))
                    needed = true;
            }
            if(!needed)
            {
                runtime.nextBuffSkillTime[slot] = now + 1500U;
                current.reason = COMPANION_SKILL_NOT_NEEDED;
                if(attempt) *attempt = current;
                continue;
            }
            const sSKILL_INFO *info = NULL;
            int mana = 0, stamina = 0;
            if(!CanCast(caster, rightItemCode, leftItemCode, descriptor.skillCode, descriptor.point,
                &info, &mana, &stamina, &current, false))
            {
                runtime.nextBuffSkillTime[slot] = now + 1000U;
                if(attempt) *attempt = current;
                continue;
            }
            const int heal = RandomBetween(Grand_Healing[p][0], Grand_Healing[p][1]);
            bool applied = false;
            if(owner && owner->dwObjectSerial && owner->smCharInfo.Life[0] > 0 && owner->smCharInfo.Life[1] > 0)
            {
                const int dx = (owner->Position.x - caster->pX) >> FLOATNS;
                const int dz = (owner->Position.z - caster->pZ) >> FLOATNS;
                if(dx * dx + dz * dz <= range * range && owner->smCharInfo.Life[0] < owner->smCharInfo.Life[1])
                {
                    const combat::HealingResult healResult = combat::legacy::ServerLegacyHealingAdapter().Heal(
                        EntityId(static_cast<std::uint32_t>(caster->dwObjectSerial)),
                        PlayerId(static_cast<std::uint32_t>(owner->dwObjectSerial)), heal);
                    if(healResult.Accepted())
                        applied = true;
                }
            }
            for(int i = 0; i < companionCount; ++i)
            {
                smCHAR *candidate = companions ? companions[i] : NULL;
                if(!candidate || !candidate->Flag || candidate->smCharInfo.Life[0] <= 0 || candidate->smCharInfo.Life[1] <= 0 || !InRange(caster, candidate, range))
                    continue;
                if(candidate->smCharInfo.Life[0] >= candidate->smCharInfo.Life[1])
                    continue;
                const combat::HealingResult healResult = combat::legacy::ServerLegacyHealingAdapter().Heal(
                    EntityId(static_cast<std::uint32_t>(caster->dwObjectSerial)),
                    EntityId(static_cast<std::uint32_t>(candidate->dwObjectSerial)), heal);
                if(healResult.Accepted()) applied = true;
            }
            if(!applied)
            {
                runtime.nextBuffSkillTime[slot] = now + 1500U;
                current.reason = COMPANION_SKILL_NOT_NEEDED;
                if(attempt) *attempt = current;
                continue;
            }
            caster->smCharInfo.Mana[0] = (short)std::max(0, (int)caster->smCharInfo.Mana[0] - mana);
            caster->smCharInfo.Stamina[0] = (short)std::max(0, (int)caster->smCharInfo.Stamina[0] - stamina);
            BeginCast(caster, runtime, descriptor.skillCode, descriptor.point, caster->dwObjectSerial, now, 750U);
            runtime.nextBuffSkillTime[slot] = now + (DWORD)intervalSeconds * 1000U;
            current.reason = COMPANION_SKILL_OK;
            if(attempt) *attempt = current;
            return true;
        }
    }
    return false;
}

bool TryExecuteCompanionHeal(smCHAR *caster, rsPLAYINFO *owner, smCHAR *const *companions,
    int companionCount, DWORD rightItemCode, DWORD leftItemCode, CompanionSkillRuntime &runtime, DWORD now,
    CompanionSkillAttempt *attempt)
{
    if(attempt) attempt->Reset();
    if(!caster || !owner || !HasCompanionHealingRole(
        static_cast<classes::PlayableClassId>(caster->smCharInfo.JOB_CODE)))
    {
        if(attempt) attempt->reason = COMPANION_SKILL_INVALID;
        return false;
    }
    if(runtime.nextHealSkillTime && (int)(runtime.nextHealSkillTime - now) > 0)
    {
        if(attempt) attempt->reason = COMPANION_SKILL_COOLDOWN;
        return false;
    }
    if(IsCompanionSkillLocked(runtime, now))
    {
        if(attempt) attempt->reason = COMPANION_SKILL_LOCKED;
        return false;
    }

    const int point = LearnedPointForCode(caster->smCharInfo.JOB_CODE, runtime.skillPoints, SKILL_HEALING);
    if(attempt)
    {
        attempt->skillCode = SKILL_HEALING;
        attempt->point = point;
        attempt->range = 320;
    }
    if(point <= 0)
    {
        if(attempt) attempt->reason = COMPANION_SKILL_NO_POINT;
        return false;
    }

    const sSKILL_INFO *info = NULL;
    int mana = 0;
    int stamina = 0;
    if(!CanCast(caster, rightItemCode, leftItemCode, SKILL_HEALING, point, &info, &mana, &stamina, attempt, false))
        return false;

    DWORD targetSerial = 0;
    int targetX = 0;
    int targetY = 0;
    int targetZ = 0;
    int bestPercent = 101;
    bool ownerTarget = false;

    if(owner->smCharInfo.Life[1] > 0 && owner->smCharInfo.Life[0] > 0)
    {
        const int percent = (owner->smCharInfo.Life[0] * 100) / owner->smCharInfo.Life[1];
        const int dx = (owner->Position.x - caster->pX) >> FLOATNS;
        const int dz = (owner->Position.z - caster->pZ) >> FLOATNS;
        if(percent < 70 && dx * dx + dz * dz <= 320 * 320)
        {
            bestPercent = percent;
            targetSerial = owner->dwObjectSerial;
            targetX = owner->Position.x;
            targetY = owner->Position.y;
            targetZ = owner->Position.z;
            ownerTarget = true;
        }
    }

    for(int i = 0; i < companionCount; ++i)
    {
        smCHAR *candidate = companions ? companions[i] : NULL;
        if(!candidate || candidate == caster || !candidate->Flag || candidate->smCharInfo.Life[0] <= 0 || candidate->smCharInfo.Life[1] <= 0)
            continue;
        if(!InRange(caster, candidate, 320))
            continue;
        const int percent = (candidate->smCharInfo.Life[0] * 100) / candidate->smCharInfo.Life[1];
        if(percent < 70 && percent < bestPercent)
        {
            bestPercent = percent;
            targetSerial = candidate->dwObjectSerial;
            targetX = candidate->pX;
            targetY = candidate->pY;
            targetZ = candidate->pZ;
            ownerTarget = false;
        }
    }

    if(!targetSerial)
    {
        if(attempt) attempt->reason = COMPANION_SKILL_NO_TARGET;
        return false;
    }

    const int p = point - 1;
    const int heal = RandomBetween(Healing_Heal[p][0], Healing_Heal[p][1]) + caster->smCharInfo.Level / 3;
    if(heal <= 0)
    {
        if(attempt) attempt->reason = COMPANION_SKILL_EFFECT_FAILED;
        return false;
    }

    combat::HealingResult healResult;
    if(ownerTarget)
        healResult = combat::legacy::ServerLegacyHealingAdapter().Heal(
            EntityId(static_cast<std::uint32_t>(caster->dwObjectSerial)),
            PlayerId(static_cast<std::uint32_t>(targetSerial)), heal);
    else
        healResult = combat::legacy::ServerLegacyHealingAdapter().Heal(
            EntityId(static_cast<std::uint32_t>(caster->dwObjectSerial)),
            EntityId(static_cast<std::uint32_t>(targetSerial)), heal);
    if(!healResult.Accepted())
    {
        if(attempt) attempt->reason = COMPANION_SKILL_EFFECT_FAILED;
        return false;
    }

    caster->smCharInfo.Mana[0] = (short)std::max(0, (int)caster->smCharInfo.Mana[0] - mana);
    caster->smCharInfo.Stamina[0] = (short)std::max(0, (int)caster->smCharInfo.Stamina[0] - stamina);
    BeginCast(caster, runtime, SKILL_HEALING, point, targetSerial, now, 750U,
        targetX, targetY, targetZ);
    runtime.nextHealSkillTime = now + 2000U;
    if(attempt) attempt->reason = COMPANION_SKILL_OK;
    return true;
}

} // namespace companions
} // namespace gameplay
