#pragma once

#include <windows.h>

struct smCHAR;
struct rsPLAYINFO;

namespace gameplay
{
namespace companions
{

static const int COMPANION_MAX_SKILL_SLOTS = 64; // 16 tiers x 4 slots; Tier 10 fits without ABI changes.

enum CompanionSkillRejectReason
{
    COMPANION_SKILL_OK = 0,
    COMPANION_SKILL_INVALID,
    COMPANION_SKILL_COOLDOWN,
    COMPANION_SKILL_LOCKED,
    COMPANION_SKILL_NO_JOB_SKILL,
    COMPANION_SKILL_NO_POINT,
    COMPANION_SKILL_NO_METADATA,
    COMPANION_SKILL_LEVEL,
    COMPANION_SKILL_WEAPON,
    COMPANION_SKILL_MP,
    COMPANION_SKILL_STAMINA,
    COMPANION_SKILL_MP_RESERVE,
    COMPANION_SKILL_STAMINA_RESERVE,
    COMPANION_SKILL_RANGE,
    COMPANION_SKILL_NO_TARGET,
    COMPANION_SKILL_MIN_TARGETS,
    COMPANION_SKILL_NOT_NEEDED,
    COMPANION_SKILL_EFFECT_FAILED,
    COMPANION_SKILL_INVALID_TARGET
};

struct CompanionSkillAttempt
{
    CompanionSkillAttempt();
    void Reset();

    CompanionSkillRejectReason reason;
    DWORD skillCode;
    int point;
    int requiredLevel;
    int manaCost;
    int staminaCost;
    int manaReserve;
    int staminaReserve;
    int range;
    int distanceSquared;
};

const char *CompanionSkillRejectReasonName(CompanionSkillRejectReason reason);

enum CompanionSkillRole
{
    COMPANION_SKILL_ROLE_PASSIVE = 0,
    COMPANION_SKILL_ROLE_DAMAGE_SINGLE,
    COMPANION_SKILL_ROLE_DAMAGE_AOE,
    COMPANION_SKILL_ROLE_CONTROL,
    COMPANION_SKILL_ROLE_BUFF_SELF,
    COMPANION_SKILL_ROLE_BUFF_GROUP,
    COMPANION_SKILL_ROLE_BUFF_TARGET,
    COMPANION_SKILL_ROLE_HEAL_SINGLE,
    COMPANION_SKILL_ROLE_HEAL_AOE,
    COMPANION_SKILL_ROLE_DEBUFF_SINGLE,
    COMPANION_SKILL_ROLE_DEBUFF_AOE,
    COMPANION_SKILL_ROLE_RESOURCE,
    COMPANION_SKILL_ROLE_SUMMON,
    COMPANION_SKILL_ROLE_REACTIVE,
    COMPANION_SKILL_ROLE_UTILITY
};

struct CompanionSkillCatalogEntry
{
    DWORD skillCode;
    CompanionSkillRole role;
    bool executorReady;
    const char *name;
    // Every catalog row declares its owning job + logical slot. The runtime does
    // not derive slots by scanning legacy sSkill[], so adding a tier is a data-only
    // catalog extension until a new behavior family is actually introduced.
    DWORD jobCode;
    BYTE useSlot;
    BYTE tier;
};

struct CompanionSkillDescriptor
{
    CompanionSkillDescriptor();

    DWORD skillCode;
    int point;
    int useSlot;
    char name[32];
};

struct CompanionBuffModifier
{
    CompanionBuffModifier();
    void Reset();

    DWORD skillCode;
    DWORD expiresAt;
    int attackMinDelta;
    int attackMaxDelta;
    int attackRatingDelta;
    int defenceDelta;
    int attackSpeedDelta;
    int shootingRangeDelta;
    int absorptionDelta;
    int chanceBlockDelta;
    int maxLifeDelta;
    float lifeRegenDelta;
    // Incoming-damage semantics that cannot be represented as persistent
    // smCharInfo sheet deltas. They are evaluated server-authoritatively in
    // the monster -> companion damage hook while this modifier is active.
    int incomingFlatReduction;
    int incomingDamageReductionPercent;
    int undeadDamageReductionPercent;
    int evasionPercent;
    int energyShieldPercent;
    int outgoingFlatDamage;
    int frostJavelinPoint;
    int sparkShieldDamage;
    int holyReflectionPercent;
    int divineInhalationPoint;
    int vanishPoint;
    float manaRegenDelta;
    int resistanceDelta[8];
    int magicSkillDamagePercent;
    int genericDamagePercent;
    int undeadDamagePercent;
    int criticalChancePercent;
    int evadePercent;
    int reactiveDamageFlat;
    int reactiveDamagePercent;
    int triggerCounter;
    int triggerThreshold;
    float lifeDrainPercent;
    bool consumeOnAttack;
};

struct CompanionSkillRuntime
{
    CompanionSkillRuntime();
    void Reset();

    BYTE skillPoints[COMPANION_MAX_SKILL_SLOTS];
    DWORD nextPotionUseTime;
    DWORD nextPotionDebugTime[3];
    int lastPotionStatus[3];
    int lastPotionTier[3];
    DWORD lastPotionCode[3];
    DWORD lastResourceRegenTime;
    float lifeRegenAccumulator;
    float manaRegenAccumulator;
    float staminaRegenAccumulator;
    DWORD nextDamageSkillTime;
    DWORD nextHealSkillTime;
    DWORD skillLockStartedAt;
    DWORD skillLockUntil;
    DWORD lastSkillCode;
    DWORD lastSkillPlayCode;
    DWORD lastSkillPoint;
    DWORD lastSkillTargetSerial;
    int lastSkillTargetX;
    int lastSkillTargetY;
    int lastSkillTargetZ;
    DWORD lastSkillMotionState;
    DWORD skillVisualSequence;

    // Received/group buffs live outside the four configurable support slots so
    // party effects never evict the companion's own self-buff loadout.
    DWORD forceNatureExpiresAt;
    int forceNatureDamage;
    int forceNatureFalconDamage;
    DWORD holyValorExpiresAt;
    int holyValorPercent;
    DWORD triumphValhallaExpiresAt;
    int triumphValhallaMaxDamageBonus;
    DWORD hallValhallaExpiresAt;
    int hallValhallaEvasionPercent;
    int hallValhallaMaxDamageBonus;
    DWORD regenerationFieldExpiresAt;
    int regenerationFieldPoint;
    DWORD regenerationFieldNextPulseAt;
    DWORD regenerationFieldPulseExpiresAt;
    int regenerationFieldPulsePoint;
    DWORD enchantWeaponExpiresAt;
    int enchantWeaponPoint;
    int enchantWeaponElement;

    // Falcon is a persistent archer familiar effect, not an entity summon.
    // Golden Falcon reuses the learned Falcon damage table and adds HP regen.
    DWORD falconExpiresAt;
    int falconPoint;
    int goldenFalconPoint;
    int falconDamageMin;
    int falconDamageMax;

    // Persistent companion familiars reuse the legacy timed-effect contract.
    // The class set is mutually exclusive, so one compact state is sufficient.
    DWORD persistentEffectSkillCode;
    DWORD persistentEffectExpiresAt;
    DWORD persistentEffectNextAttackAt;
    int persistentEffectPoint;
    int persistentEffectElement;

    // V9 multi-hit scheduler. Raving/Avalanche are one cast with repeated
    // authoritative hit events, not one aggregated burst.
    DWORD multiHitSkillCode;
    DWORD multiHitTargetSerial;
    int multiHitPoint;
    int multiHitRemaining;
    DWORD multiHitNextTime;
    DWORD multiHitIntervalMs;

    // Charging Strike uses the legacy charge animation as an authoritative
    // delayed single hit. Companion AI always lets the charge complete.
    DWORD chargedStrikeTargetSerial;
    DWORD chargedStrikeExecuteAt;
    int chargedStrikePoint;

    BYTE activeSkillCursor;
    // Local scalable/Tier-5 channel + passive state.
    DWORD channelSkillCode;
    DWORD channelEndTime;
    DWORD channelNextTick;
    BYTE channelPoint;
    BYTE passiveEvadePercent;
    bool passiveStatsApplied;
    // V30+ diagnostics / loadout reject throttling.
    DWORD nextSkillRejectLogTime[3];
    DWORD lastSkillRejectCode[3];
    int lastSkillRejectReason[3];
    DWORD nextBuffSkillTime[4];
    CompanionBuffModifier buffModifiers[4];
};

// Normalizes the legacy 16-byte DAT layout into logical slots 1..N.
// The legacy layout stores slot 16 at index 0; callers should not depend on that quirk.
void LoadCompanionSkillPoints(CompanionSkillRuntime &runtime, const BYTE *points, int count);
void LoadCompanionLogicalSkillPoints(CompanionSkillRuntime &runtime, const BYTE *points, int count);
void SetCompanionSkillPoint(CompanionSkillRuntime &runtime, int useSlot, int point);
int GetCompanionSkillPoint(const CompanionSkillRuntime &runtime, int useSlot);

// Applies learned passive skills once after the companion entity has been built
// from the legacy character record. The normalized runtime makes passives from
// future tiers data-addressable without changing RECORD_SKILL/DAT ABI.
void ApplyCompanionPassiveSkills(smCHAR *caster, CompanionSkillRuntime &runtime);

// Resolve/cycle only skills that are both learned in the character DAT and
// implemented by the server-authoritative companion executor. useSlot == 0
// represents an explicitly disabled UI slot.
bool GetCompanionSkillDescriptor(
    DWORD jobCode,
    const BYTE *points,
    int useSlot,
    CompanionSkillDescriptor *descriptor);

int CycleCompanionSkillUseSlot(
    DWORD jobCode,
    const BYTE *points,
    int currentUseSlot,
    int direction,
    bool buffSkill);

int GetCompanionSkillCatalogCount();
bool GetCompanionSkillCatalogEntryAt(int index, CompanionSkillCatalogEntry *entry);
bool GetCompanionSkillCatalogEntry(DWORD skillCode, CompanionSkillCatalogEntry *entry);
const char *CompanionSkillRoleName(CompanionSkillRole role);
bool IsCompanionDamageSkillSupported(DWORD skillCode);
bool IsCompanionBuffSkillSupported(DWORD skillCode);
bool IsCompanionSupportSkillSupported(DWORD skillCode);
int GetCompanionBuffDurationSeconds(DWORD skillCode, int point);
int GetCompanionSupportDefaultIntervalSeconds(DWORD skillCode, int point);


// Refreshes 5-second party pulses from an active Regeneration Field every
// four seconds while recipients remain inside the legacy area/height bounds.
void UpdateCompanionRegenerationFieldAura(smCHAR *caster, smCHAR *const *companions,
    CompanionSkillRuntime *const *companionRuntimes, int companionCount,
    CompanionSkillRuntime &runtime, DWORD now);

// Applies the same natural resource-regeneration formulas used by the legacy
// player loop. Fractional progress is kept in the companion runtime.
bool UpdateCompanionResourceRegeneration(
    smCHAR *caster,
    CompanionSkillRuntime &runtime,
    DWORD now);

// Consumes one validated potion stack from the offline character DAT and then
// applies its persisted restore value. The owner's inventory is never touched.
bool TryUseCompanionConsumable(
    const char *accountName,
    const char *characterName,
    smCHAR *caster,
    CompanionSkillRuntime &runtime,
    DWORD now);

// Periodic debug snapshot that distinguishes "potion not needed" from a
// failed consume path. It never mutates inventory or resources.
void LogCompanionPotionState(const char *characterName, smCHAR *caster,
    const CompanionSkillRuntime &runtime, DWORD now);

// Advances Magnetic Sphere / Dancing Sword / Summon Muspell autonomous attacks.
// Returns true when an authoritative familiar hit was applied this tick.
bool UpdateCompanionPersistentEffect(
    smCHAR *caster,
    rsPLAYINFO *owner,
    CompanionSkillRuntime &runtime,
    DWORD now);

// Advances a scheduled multi-hit cast. Returns true when at least one hit was
// applied this tick. A dead/missing target cancels the remainder safely.
bool UpdateCompanionMultiHit(
    smCHAR *caster,
    rsPLAYINFO *owner,
    CompanionSkillRuntime &runtime,
    DWORD now);

bool UpdateCompanionChargedStrike(
    smCHAR *caster,
    rsPLAYINFO *owner,
    CompanionSkillRuntime &runtime,
    DWORD now);

// First server-authoritative skill slice: one learned damage skill per legacy
// class. Returns true only when a real cast was executed and MP/stamina were consumed.
bool TryExecuteCompanionDamageSkill(
    smCHAR *caster,
    rsPLAYINFO *owner,
    smCHAR *target,
    DWORD rightItemCode,
    DWORD leftItemCode,
    CompanionSkillRuntime &runtime,
    DWORD now,
    CompanionSkillAttempt *attempt = NULL);

// Skill Loadout V30: when enabled, tries configured UseSkill slots in
// deterministic priority order (slot 1 highest). Per-slot policy can require
// a post-cast resource reserve, minimum affected targets, and target mode.
bool TryExecuteCompanionDamageLoadout(
    smCHAR *caster,
    rsPLAYINFO *owner,
    smCHAR *target,
    DWORD rightItemCode,
    DWORD leftItemCode,
    const BYTE activeSkillSlots[4],
    const BYTE activeMinResourcePercent[4],
    const BYTE activeMinimumTargets[4],
    const BYTE activeTargetModes[4],
    bool loadoutActive,
    CompanionSkillRuntime &runtime,
    DWORD now,
    CompanionSkillAttempt *attempt);

// Executes at most one due self-buff per AI tick. The interval belongs to the
// profile slot and controls recast cadence independently from effect duration.
bool TryExecuteCompanionBuffLoadout(
    smCHAR *caster,
    DWORD rightItemCode,
    DWORD leftItemCode,
    const BYTE buffSkillSlots[4],
    const WORD buffIntervalSeconds[4],
    bool loadoutActive,
    CompanionSkillRuntime &runtime,
    DWORD now,
    CompanionSkillAttempt *attempt);

// V5 support loadout. Reuses the persisted four buff slots, but also accepts
// conditional RESOURCE and HEAL_AOE skills. Existing self-buffs remain fully
// compatible and retain their per-slot modifiers.
bool TryExecuteCompanionSupportLoadout(
    smCHAR *caster,
    rsPLAYINFO *owner,
    smCHAR *const *companions,
    CompanionSkillRuntime *const *companionRuntimes,
    int companionCount,
    DWORD rightItemCode,
    DWORD leftItemCode,
    const BYTE supportSkillSlots[4],
    const WORD supportIntervalSeconds[4],
    bool loadoutActive,
    CompanionSkillRuntime &runtime,
    DWORD now,
    CompanionSkillAttempt *attempt);

void UpdateCompanionBuffModifiers(smCHAR *caster, CompanionSkillRuntime &runtime, DWORD now);
// Applies companion-only defensive effects before the legacy Absorption step.
// A completely blocked/evaded hit is represented by *damage == 0; the result
// is always clamped to a non-negative value.
void ApplyCompanionIncomingDamageModifiers(smCHAR *target, smCHAR *attacker, rsPLAYINFO *owner,
    CompanionSkillRuntime &runtime, int *damage, DWORD now);
// Returns false when active Vanish reduces this monster's detection radius below
// the current distance. Boss exceptions match the legacy player scan.
bool CanMonsterSeeCompanionWithVanish(smCHAR *companion, smCHAR *monster,
    CompanionSkillRuntime &runtime, DWORD now);
// Applies post-formula modifiers that belong only to normal weapon attacks.
void ApplyCompanionOutgoingBasicDamageModifiers(smCHAR *caster, smCHAR *target,
    DWORD rightItemCode, CompanionSkillRuntime &runtime, int *damage, DWORD now);
void ApplyCompanionOutgoingBasicPostAbsorptionModifiers(smCHAR *caster, smCHAR *target,
    CompanionSkillRuntime &runtime, int *damage, DWORD now);
void ResetCompanionBuffModifier(smCHAR *caster, CompanionSkillRuntime &runtime, int slot);
void ResetAllCompanionBuffModifiers(smCHAR *caster, CompanionSkillRuntime &runtime);

// Priestess Healing. Chooses the most injured living owner/companion in range.
bool TryExecuteCompanionHeal(
    smCHAR *caster,
    rsPLAYINFO *owner,
    smCHAR *const *companions,
    int companionCount,
    DWORD rightItemCode,
    DWORD leftItemCode,
    CompanionSkillRuntime &runtime,
    DWORD now,
    CompanionSkillAttempt *attempt = NULL);

bool IsCompanionSkillLocked(const CompanionSkillRuntime &runtime, DWORD now);

} // namespace companions
} // namespace gameplay
