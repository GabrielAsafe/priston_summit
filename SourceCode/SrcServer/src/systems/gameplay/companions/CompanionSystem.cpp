#include "content/v1/gameplay_config/content/GameplayConfigRuntime.h"
#include "systems/gameplay/companions/CompanionSystem.h"
#include "systems/gameplay/companions/CompanionProfileStore.h"
#include "systems/gameplay/companions/CompanionTacticsStore.h"
#include "systems/gameplay/companions/CompanionSkillExecutor.h"
#include "opcodes.h"

#include <algorithm>
#include <map>
#include <mutex>
#include <string>
#include <vector>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

// Legacy character.h is not self-contained: its geometry/model types come
// from smd3d.h and its socket-facing player type expects smwsock.h. Keep the
// same include order used by the established server translation units.
#include "systems/network/transport/legacy/smwsock.h"
#include "systems/rendering/legacy_3d/smd3d.h"
#include "systems/gameplay/character/core/character.h"
#include "systems/persistence/character_record/record.h"
#include "core/server/gameplay_runtime/playmain.h"
#include "core/gameplay/legacy_support/playsub.h"
#include "core/server/legacy_runtime/onserver.h"
#include "systems/combat/skills/legacy/sinSkill.h"
#include "modules/File/File.h"

extern DWORD dwPlayServTime;
extern INT64 ExpLevelTable[];
INT64 GetExp64(smCHAR_INFO *smCharInfo);

// Implemented beside the legacy crystal spawner, where STG_AREA/crystal
// templates are already available.
extern smCHAR *OpenCharacterCompanionFromRecord(
    rsPLAYINFO *owner,
    const TRANS_RECORD_DATA *recordData);

namespace gameplay
{
namespace companions
{

namespace
{

// Used by early AI/loot helpers; definition lives later with the other
// companion target helpers.
bool IsValidHostile(smCHAR *candidate);

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

static bool CompanionDebugEnabled()
{
    const char *value = getenv("SUMMIT_COMPANION_DEBUG");
    return value && value[0] && strcmp(value, "0") != 0;
}

static void CompanionDebugLog(const char *format, ...)
{
    if(!CompanionDebugEnabled())
        return;

    FILE *file = fopen("LOGS\\companion_debug_server.log", "a");
    if(!file)
        file = fopen("companion_debug_server.log", "a");
    if(!file)
        return;

    fprintf(file, "[%lu] ", (unsigned long)dwPlayServTime);
    va_list args;
    va_start(args, format);
    vfprintf(file, format, args);
    va_end(args);
    fputc('\n', file);
    fflush(file);
    fclose(file);
}

static bool CompanionDebugTick(DWORD serial, DWORD intervalMs)
{
    static DWORD serials[64] = {};
    static DWORD times[64] = {};
    const unsigned int slot = (unsigned int)(serial % 64u);
    if(serials[slot] == serial && (DWORD)(dwPlayServTime - times[slot]) < intervalMs)
        return false;
    serials[slot] = serial;
    times[slot] = dwPlayServTime;
    return true;
}

static bool ShouldLogCompanionSkillReject(CompanionSkillRuntime &runtime, int lane,
    const CompanionSkillAttempt &attempt, DWORD now, DWORD intervalMs)
{
    if(lane < 0 || lane >= 3)
        return false;

    const int reason = (int)attempt.reason;
    const bool changed = runtime.lastSkillRejectCode[lane] != attempt.skillCode ||
        runtime.lastSkillRejectReason[lane] != reason;
    if(!changed && runtime.nextSkillRejectLogTime[lane] &&
       (int)(runtime.nextSkillRejectLogTime[lane] - now) > 0)
        return false;

    runtime.lastSkillRejectCode[lane] = attempt.skillCode;
    runtime.lastSkillRejectReason[lane] = reason;
    runtime.nextSkillRejectLogTime[lane] = now + intervalMs;
    return true;
}

struct CompanionSlot
{
    CompanionSlot()
        : entity(NULL), cachedLevel(0), changeJob(0),
          cachedExp(0), pendingExp(0), lastExpFlushTime(0),
          retreatLatched(false), retreatUntil(0), deadSince(0),
          lastProgressCheckTime(0), lastProgressX(0), lastProgressZ(0), stuckTicks(0),
          nextIdentityCheckTime(0), lootArea(NULL), lootTarget(NULL), lootTargetSince(0),
          nextLootScanTime(0), nextLootPickupTime(0),
          visualMotionState(0), visualMotionSequence(0), visualMotionFrame(0)
    {
        characterName[0] = 0;
    }

    char characterName[32];
    smCHAR *entity;
    int cachedLevel;
    int changeJob;
    INT64 cachedExp;
    INT64 pendingExp;
    DWORD lastExpFlushTime;
    bool retreatLatched;
    DWORD retreatUntil;
    DWORD deadSince;
    DWORD lastProgressCheckTime;
    int lastProgressX;
    int lastProgressZ;
    int stuckTicks;
    DWORD nextIdentityCheckTime;
    STG_AREA *lootArea;
    STG_ITEMS *lootTarget;
    DWORD lootTargetSince;
    DWORD nextLootScanTime;
    DWORD nextLootPickupTime;
    DWORD visualMotionState;
    DWORD visualMotionSequence;
    int visualMotionFrame;
    CompanionSkillRuntime skillRuntime;
};

static const int COMPANION_UI_MAX_EQUIP_CHOICES = 64;

struct EquipmentChoice
{
    EquipmentChoice() : code(0) { name[0] = 0; }
    DWORD code;
    char name[32];
};

bool IsCompanionTwoHandCode(DWORD code, int itemIndex);

struct CharacterEquipmentSnapshot
{
    CharacterEquipmentSnapshot()
        : valid(false), level(0), jobCode(0), life(0), maxLife(0), mana(0), maxMana(0),
          defaultRight(0), defaultLeft(0), rightCount(0), leftCount(0)
    {
        characterName[0] = 0;
        ZeroMemory(skillPoints, sizeof(skillPoints));
        defaultRightName[0] = 0;
        defaultLeftName[0] = 0;
    }

    bool valid;
    char characterName[32];
    int level;
    int jobCode;
    int life;
    int maxLife;
    int mana;
    int maxMana;
    BYTE skillPoints[COMPANION_MAX_SKILL_SLOTS];
    DWORD defaultRight;
    DWORD defaultLeft;
    char defaultRightName[32];
    char defaultLeftName[32];
    EquipmentChoice right[COMPANION_UI_MAX_EQUIP_CHOICES];
    EquipmentChoice left[COMPANION_UI_MAX_EQUIP_CHOICES];
    int rightCount;
    int leftCount;
};

struct LoadoutOverride
{
    LoadoutOverride()
        : loaded(false), active(false), rightCode(0), leftCode(0),
          mode(COMPANION_MODE_AGGRESSIVE), tactic(COMPANION_TACTIC_BALANCED),
          retreatHpPercent(0), preferredRange(0), formationSlot(-1),
          skillLoadoutActive(false), buffLoadoutActive(false)
    {
        characterName[0] = 0;
        ZeroMemory(activeSkillSlots, sizeof(activeSkillSlots));
        ZeroMemory(activeMinResourcePercent, sizeof(activeMinResourcePercent));
        ZeroMemory(activeTargetModes, sizeof(activeTargetModes));
        for(int i = 0; i < 4; ++i) activeMinimumTargets[i] = 1;
        ZeroMemory(buffSkillSlots, sizeof(buffSkillSlots));
        ZeroMemory(buffIntervalSeconds, sizeof(buffIntervalSeconds));
        ZeroMemory(buffTargetModes, sizeof(buffTargetModes));
    }

    bool loaded;
    bool active;
    char characterName[32];
    DWORD rightCode;
    DWORD leftCode;
    CompanionMode mode;
    CompanionTacticPreset tactic;
    int retreatHpPercent;
    int preferredRange;
    int formationSlot;
    CompanionTacticsConfig customTactics;
    bool skillLoadoutActive;
    BYTE activeSkillSlots[4];
    BYTE activeMinResourcePercent[4];
    BYTE activeMinimumTargets[4];
    BYTE activeTargetModes[4];
    bool buffLoadoutActive;
    BYTE buffSkillSlots[4];
    WORD buffIntervalSeconds[4];
    BYTE buffTargetModes[4];
};

struct OwnerState
{
    OwnerState()
        : owner(NULL), connectCount(0), mode(COMPANION_MODE_AGGRESSIVE),
          lastRuntimeSendTime(0), lastVisualSendTime(0)
    {
        selectedName[0] = 0;
    }

    rsPLAYINFO *owner;
    DWORD connectCount;
    CompanionMode mode;
    DWORD lastRuntimeSendTime;
    DWORD lastVisualSendTime;
    char selectedName[32];
    CompanionSlot slots[COMPANION_MAX_ACTIVE];
    LoadoutOverride loadouts[protocol::COMPANION_UI_MAX_CHARACTERS];
};

std::mutex g_mutex;
std::map<rsPLAYINFO *, OwnerState> g_states;

bool IsOwnerValid(const OwnerState &state)
{
    return state.owner &&
        state.owner->lpsmSock &&
        state.owner->dwObjectSerial &&
        state.owner->dwConnectCount == state.connectCount;
}

bool IsSlotActive(const OwnerState &state, const CompanionSlot &slot)
{
    return IsOwnerValid(state) &&
        slot.entity &&
        slot.entity->Flag &&
        slot.entity->smCharInfo.Life[0] > 0 &&
        slot.entity->lpMasterPlayInfo == state.owner;
}

int GetExpFlushSeconds()
{
    int seconds = SummitCompanionConfigInt("ExpFlushSeconds", 30);
    if(seconds < 5) seconds = 5;
    if(seconds > 300) seconds = 300;
    return seconds;
}

int GetResurrectionCorpseSeconds()
{
    int seconds = GetPrivateProfileIntA(
        "Companion", "ResurrectionCorpseSeconds", 30, ".//Configs//Companion.ini");
    if(seconds < 5) seconds = 5;
    if(seconds > 120) seconds = 120;
    return seconds;
}

void ClearSlot(CompanionSlot &slot)
{
    slot.entity = NULL;
    slot.characterName[0] = 0;
    slot.cachedLevel = 0;
    slot.changeJob = 0;
    slot.cachedExp = 0;
    slot.pendingExp = 0;
    slot.lastExpFlushTime = 0;
    slot.retreatLatched = false;
    slot.retreatUntil = 0;
    slot.deadSince = 0;
    slot.lastProgressCheckTime = 0;
    slot.lastProgressX = 0;
    slot.lastProgressZ = 0;
    slot.stuckTicks = 0;
    slot.nextIdentityCheckTime = 0;
    slot.lootArea = NULL;
    slot.lootTarget = NULL;
    slot.lootTargetSince = 0;
    slot.nextLootScanTime = 0;
    slot.nextLootPickupTime = 0;
    slot.visualMotionState = 0;
    slot.visualMotionSequence = 0;
    slot.visualMotionFrame = 0;
    slot.skillRuntime.Reset();
}

bool FlushSlotExperience(OwnerState &state, CompanionSlot &slot)
{
    if(slot.pendingExp <= 0 || !slot.characterName[0])
    {
        slot.lastExpFlushTime = dwPlayServTime;
        return true;
    }

    if(!state.owner || !state.owner->getAccount() || !state.owner->getAccount()[0])
        return false;

    const int previousLevel = slot.cachedLevel;
    int newLevel = slot.cachedLevel;
    INT64 newExperience = slot.cachedExp;
    if(!rsAddOfflineCharacterExp(
        state.owner->getAccount(), slot.characterName, slot.pendingExp,
        &newLevel, &newExperience))
    {
        return false;
    }

    slot.pendingExp = 0;
    slot.cachedExp = newExperience;
    slot.cachedLevel = newLevel;
    slot.lastExpFlushTime = dwPlayServTime;
    if(slot.entity && slot.entity->Flag)
    {
        // Keep the live companion's progression header coherent with the DAT
        // that rsAddOfflineCharacterExp just committed.  The legacy helper
        // grants level StatePoint through ReformCharStatePoint(), but an
        // already-summoned companion previously kept the old StatePoint/EXP
        // until dismiss + resummon.  Preserve combat/resources/equipment and
        // only refresh the progression fields that were authoritatively
        // persisted by the offline EXP path.
        slot.entity->smCharInfo.Level = newLevel;
        SetExp64(&slot.entity->smCharInfo, newExperience);

        if(newLevel != previousLevel)
        {
            TRANS_RECORD_DATA refreshed;
            ZeroMemory(&refreshed, sizeof(refreshed));
            if(rsLoadAccountCharacterRecord(state.owner->getAccount(), slot.characterName, &refreshed))
            {
                slot.entity->smCharInfo.StatePoint = refreshed.smCharInfo.StatePoint;
                slot.entity->smCharInfo.Next_Exp = refreshed.smCharInfo.Next_Exp;
                slot.entity->smCharInfo.ChangeJob = refreshed.smCharInfo.ChangeJob;
                slot.entity->smCharInfo.JobBitMask = refreshed.smCharInfo.JobBitMask;
            }

            CompanionDebugLog(
                "LEVEL_UP name='%s' serial=%lu level=%d->%d exp=%lld statePoint=%d",
                slot.characterName, (unsigned long)slot.entity->dwObjectSerial,
                previousLevel, newLevel, (long long)newExperience,
                slot.entity->smCharInfo.StatePoint);
        }
    }
    return true;
}

bool FlushSlotExperienceRetry(OwnerState &state, CompanionSlot &slot, int attempts)
{
    if(attempts < 1)
        attempts = 1;

    for(int attempt = 0; attempt < attempts; ++attempt)
    {
        if(FlushSlotExperience(state, slot))
            return true;
        Sleep(1);
    }
    return false;
}

void PruneState(OwnerState &state)
{
    const DWORD kDeadVisualHoldMs = (DWORD)GetResurrectionCorpseSeconds() * 1000U;
    for(int i = 0; i < COMPANION_MAX_ACTIVE; ++i)
    {
        CompanionSlot &slot = state.slots[i];
        if(slot.characterName[0] && slot.entity)
        {
            const bool ownershipValid = slot.entity->Flag &&
                slot.entity->lpMasterPlayInfo == state.owner;
            const bool dead = slot.entity->smCharInfo.Life[0] <= 0;

            if(ownershipValid && dead)
            {
                if(!slot.deadSince)
                    slot.deadSince = dwPlayServTime;

                // Keep the authoritative corpse registered briefly so the V2
                // visual stream can publish one stable DEAD sequence and the
                // client can finish the death clip before the slot disappears.
                if((DWORD)(dwPlayServTime - slot.deadSince) < kDeadVisualHoldMs)
                    continue;
            }
            else if(ownershipValid)
            {
                slot.deadSince = 0;
                continue;
            }

            // Natural death/expiry must not discard buffered EXP.
            smCHAR *retiringEntity = slot.entity;
            if(FlushSlotExperience(state, slot))
            {
                if(retiringEntity && retiringEntity->smCharInfo.Life[0] <= 0)
                    retiringEntity->lpMasterPlayInfo = NULL;
                ClearSlot(slot);
            }
            else
                slot.entity = NULL;
        }
        else if(slot.characterName[0] && !slot.entity && slot.pendingExp > 0)
        {
            // Retry a previously failed flush on the next state touch.
            if(FlushSlotExperience(state, slot))
                ClearSlot(slot);
        }
    }
}

OwnerState &GetOrCreateState(rsPLAYINFO *owner)
{
    OwnerState &state = g_states[owner];
    if(state.owner != owner || state.connectCount != owner->dwConnectCount)
    {
        state = OwnerState();
        state.owner = owner;
        state.connectCount = owner->dwConnectCount;
    }
    return state;
}

OwnerState *FindState(rsPLAYINFO *owner)
{
    std::map<rsPLAYINFO *, OwnerState>::iterator it = g_states.find(owner);
    if(it == g_states.end())
        return NULL;

    if(!IsOwnerValid(it->second))
    {
        g_states.erase(it);
        return NULL;
    }

    PruneState(it->second);
    return &it->second;
}


smCHAR *FindNearestHostile(smCHAR *character)
{
    if(!character)
        return NULL;

    STG_AREA *area = static_cast<STG_AREA *>(character->lpExt4);
    if(!area)
        return NULL;

    smCHAR *best = NULL;
    int bestDistance = 0;
    int sight = character->smCharInfo.Sight;
    if(sight < 220) sight = 220;
    if(sight > 450) sight = 450;
    const int maxDistance = sight * sight;

    for(int i = 0; i < area->MonsterLastPoint; ++i)
    {
        smCHAR *candidate = area->lpCharMonster[i];
        if(!candidate || candidate == character || !candidate->Flag)
            continue;
        if(candidate->smCharInfo.Life[0] <= 0)
            continue;
        if(candidate->smCharInfo.State != smCHAR_STATE_ENEMY)
            continue;
        if(candidate->smCharInfo.Brood == smCHAR_MONSTER_USER)
            continue;
        if(candidate->lpMasterPlayInfo)
            continue;

        const int dx = (candidate->pX - character->pX) >> FLOATNS;
        const int dz = (candidate->pZ - character->pZ) >> FLOATNS;
        const int distance = dx * dx + dz * dz;
        if(distance > maxDistance)
            continue;

        if(!best || distance < bestDistance)
        {
            best = candidate;
            bestDistance = distance;
        }
    }

    return best;
}

smCHAR *FindNearestThreatToParty(smCHAR *character, rsPLAYINFO *owner, const OwnerState &state)
{
    if(!character || !owner)
        return NULL;
    STG_AREA *area = static_cast<STG_AREA *>(character->lpExt4);
    if(!area)
        return NULL;

    smCHAR *best = NULL;
    long long bestDistance = 0x7FFFFFFFFFFFFFFFLL;
    for(int i = 0; i < area->MonsterLastPoint; ++i)
    {
        smCHAR *candidate = area->lpCharMonster[i];
        if(!IsValidHostile(candidate))
            continue;

        bool threatensParty = candidate->lpTargetPlayInfo == owner;
        if(!threatensParty && candidate->lpTargetChar)
        {
            for(int slot = 0; slot < COMPANION_MAX_ACTIVE; ++slot)
            {
                if(state.slots[slot].entity == candidate->lpTargetChar &&
                   state.slots[slot].entity && state.slots[slot].entity->Flag)
                {
                    threatensParty = true;
                    break;
                }
            }
        }
        if(!threatensParty)
            continue;

        const long long dx = (candidate->pX - character->pX) >> FLOATNS;
        const long long dz = (candidate->pZ - character->pZ) >> FLOATNS;
        const long long distance = dx * dx + dz * dz;
        if(!best || distance < bestDistance)
        {
            best = candidate;
            bestDistance = distance;
        }
    }
    return best;
}


long long ScoreHostileForCompanion(smCHAR *character, rsPLAYINFO *owner,
    const OwnerState &state, smCHAR *candidate, smCHAR *currentTarget, smCHAR *ownerTarget)
{
    if(!character || !IsValidHostile(candidate))
        return 0x7FFFFFFFFFFFFFFFLL;

    const long long dx = (candidate->pX - character->pX) >> FLOATNS;
    const long long dz = (candidate->pZ - character->pZ) >> FLOATNS;
    long long score = dx * dx + dz * dz;

    // Explicit owner focus is handled before scoring. The bonuses below are
    // deliberately smaller than the full sight-radius delta, so a very distant
    // stale target can still be replaced by a clearly better nearby threat.
    if(candidate == currentTarget)
        score -= 40000;
    if(candidate == ownerTarget)
        score -= 60000;
    if(candidate->lpTargetPlayInfo == owner)
        score -= 50000;

    if(candidate->lpTargetChar)
    {
        for(int slot = 0; slot < COMPANION_MAX_ACTIVE; ++slot)
        {
            if(state.slots[slot].entity == candidate->lpTargetChar &&
               state.slots[slot].entity && state.slots[slot].entity->Flag)
            {
                score -= 35000;
                break;
            }
        }
    }
    return score;
}

smCHAR *FindBestHostile(smCHAR *character, rsPLAYINFO *owner, const OwnerState &state,
    smCHAR *currentTarget, smCHAR *ownerTarget)
{
    if(!character)
        return NULL;
    STG_AREA *area = static_cast<STG_AREA *>(character->lpExt4);
    if(!area)
        return NULL;

    int sight = character->smCharInfo.Sight;
    if(sight < 220) sight = 220;
    if(sight > 450) sight = 450;
    const long long maxDistance = (long long)sight * (long long)sight;

    smCHAR *best = NULL;
    long long bestScore = 0x7FFFFFFFFFFFFFFFLL;
    for(int i = 0; i < area->MonsterLastPoint; ++i)
    {
        smCHAR *candidate = area->lpCharMonster[i];
        if(!IsValidHostile(candidate))
            continue;
        const long long dx = (candidate->pX - character->pX) >> FLOATNS;
        const long long dz = (candidate->pZ - character->pZ) >> FLOATNS;
        if(dx * dx + dz * dz > maxDistance)
            continue;

        const long long score = ScoreHostileForCompanion(
            character, owner, state, candidate, currentTarget, ownerTarget);
        if(!best || score < bestScore)
        {
            best = candidate;
            bestScore = score;
        }
    }
    return best;
}

int CountNearbyHostiles(smCHAR *character, int radius)
{
    if(!character || radius <= 0)
        return 0;
    STG_AREA *area = static_cast<STG_AREA *>(character->lpExt4);
    if(!area)
        return 0;

    const long long limit = (long long)radius * (long long)radius;
    int count = 0;
    for(int i = 0; i < area->MonsterLastPoint; ++i)
    {
        smCHAR *candidate = area->lpCharMonster[i];
        if(!IsValidHostile(candidate))
            continue;
        const long long dx = (candidate->pX - character->pX) >> FLOATNS;
        const long long dz = (candidate->pZ - character->pZ) >> FLOATNS;
        if(dx * dx + dz * dz <= limit)
            ++count;
    }
    return count;
}

bool CustomTacticConditionMatches(const protocol::CompanionUITacticRule &rule,
    smCHAR *character, rsPLAYINFO *owner, const OwnerState &state, smCHAR *ownerTarget)
{
    if(!rule.enabled || !character || !owner)
        return false;

    switch(rule.condition)
    {
        case protocol::COMPANION_TACTIC_CONDITION_ALWAYS:
            return true;

        case protocol::COMPANION_TACTIC_CONDITION_SELF_HP_BELOW:
            return character->smCharInfo.Life[1] > 0 &&
                (character->smCharInfo.Life[0] * 100) / character->smCharInfo.Life[1] < rule.threshold;

        case protocol::COMPANION_TACTIC_CONDITION_OWNER_HP_BELOW:
            return owner->smCharInfo.Life[1] > 0 &&
                (owner->smCharInfo.Life[0] * 100) / owner->smCharInfo.Life[1] < rule.threshold;

        case protocol::COMPANION_TACTIC_CONDITION_ENEMIES_NEARBY_AT_LEAST:
            return CountNearbyHostiles(character, 300) >= rule.threshold;

        case protocol::COMPANION_TACTIC_CONDITION_OWNER_HAS_TARGET:
            return ownerTarget != NULL;

        case protocol::COMPANION_TACTIC_CONDITION_PARTY_THREATENED:
            return FindNearestThreatToParty(character, owner, state) != NULL;

        default:
            return false;
    }
}

int EvaluateCustomTactic(const CompanionTacticsConfig &config, smCHAR *character,
    rsPLAYINFO *owner, const OwnerState &state, smCHAR *ownerTarget, int *matchedRule)
{
    if(matchedRule) *matchedRule = -1;
    if(!config.enabled)
        return -1;

    for(int i = 0; i < protocol::COMPANION_UI_TACTIC_RULES; ++i)
    {
        if(CustomTacticConditionMatches(config.rules[i], character, owner, state, ownerTarget))
        {
            if(matchedRule) *matchedRule = i;
            return config.rules[i].action;
        }
    }
    return -1;
}

bool HasOtherCompanionTargeting(const OwnerState &state, smCHAR *character, smCHAR *target)
{
    if(!target)
        return false;
    for(int slot = 0; slot < COMPANION_MAX_ACTIVE; ++slot)
    {
        smCHAR *entity = state.slots[slot].entity;
        if(entity && entity != character && entity->Flag && entity->lpTargetChar == target)
            return true;
    }
    return false;
}


void DisengageTarget(smCHAR *character);

bool MoveTowardInjuredAlly(smCHAR *healer, rsPLAYINFO *owner,
    smCHAR *const *companions, int companionCount, DWORD *targetSerial)
{
    if(targetSerial) *targetSerial = 0;
    if(!healer || !owner)
        return false;

    int bestPercent = 101;
    long long bestDistance = 0x7FFFFFFFFFFFFFFFLL;
    int targetX = 0;
    int targetZ = 0;
    DWORD serial = 0;

    if(owner->smCharInfo.Life[0] > 0 && owner->smCharInfo.Life[1] > 0)
    {
        const int percent = (owner->smCharInfo.Life[0] * 100) / owner->smCharInfo.Life[1];
        const long long dx = (owner->Position.x - healer->pX) >> FLOATNS;
        const long long dz = (owner->Position.z - healer->pZ) >> FLOATNS;
        const long long distance = dx * dx + dz * dz;
        if(percent < 70 && distance > (320LL * 320LL) && distance <= (600LL * 600LL))
        {
            bestPercent = percent;
            bestDistance = distance;
            targetX = owner->Position.x;
            targetZ = owner->Position.z;
            serial = owner->dwObjectSerial;
        }
    }

    for(int i = 0; i < companionCount; ++i)
    {
        smCHAR *candidate = companions ? companions[i] : NULL;
        if(!candidate || candidate == healer || !candidate->Flag ||
           candidate->smCharInfo.Life[0] <= 0 || candidate->smCharInfo.Life[1] <= 0)
            continue;
        const int percent = (candidate->smCharInfo.Life[0] * 100) / candidate->smCharInfo.Life[1];
        const long long dx = (candidate->pX - healer->pX) >> FLOATNS;
        const long long dz = (candidate->pZ - healer->pZ) >> FLOATNS;
        const long long distance = dx * dx + dz * dz;
        if(percent >= 70 || distance <= (320LL * 320LL) || distance > (600LL * 600LL))
            continue;
        if(percent < bestPercent || (percent == bestPercent && distance < bestDistance))
        {
            bestPercent = percent;
            bestDistance = distance;
            targetX = candidate->pX;
            targetZ = candidate->pZ;
            serial = candidate->dwObjectSerial;
        }
    }

    if(!serial)
        return false;

    DisengageTarget(healer);
    healer->SetTargetPosi2(targetX >> FLOATNS, targetZ >> FLOATNS);
    healer->TargetMoveCount = 2;
    healer->TragetTraceMode = FALSE;
    healer->AutoMoveStep = 0;
    healer->SetNextTarget(0, 0, 0);
    if(healer->MotionInfo &&
       healer->MotionInfo->State != CHRMOTION_STATE_DAMAGE &&
       healer->MotionInfo->State != CHRMOTION_STATE_DEAD &&
       healer->MotionInfo->State != CHRMOTION_STATE_ATTACK &&
       healer->MotionInfo->State != CHRMOTION_STATE_SKILL)
    {
        healer->SetMotionFromCode(CHRMOTION_STATE_RUN);
        if(!healer->MotionInfo || healer->MotionInfo->State != CHRMOTION_STATE_RUN)
            healer->SetMotionFromCode(CHRMOTION_STATE_WALK);
    }
    if(targetSerial) *targetSerial = serial;
    return true;
}

void CombatFormationOffset(int slot, int &x, int &z)
{
    static const int offsets[COMPANION_MAX_ACTIVE][2] =
    {
        { -36, -36 },
        {  36, -36 },
        { -36,  36 },
        {  36,  36 }
    };
    if(slot < 0 || slot >= COMPANION_MAX_ACTIVE)
        slot = 0;
    x = offsets[slot][0];
    z = offsets[slot][1];
}

void EngageTarget(smCHAR *character, smCHAR *target, int preferredRange, int formationSlot, bool combatSpacing)
{
    if(!character || !target)
        return;

    const bool targetChanged = character->lpTargetChar != target;
    character->lpTargetChar = target;
    character->lpTargetPlayInfo = NULL;

    // A follow move can leave TargetMoveCount active for a few server ticks.
    // Reset it when changing combat targets so ptNextTarget is consumed on the
    // next character tick instead of the companion appearing stuck beside its
    // owner while already having a valid enemy.
    if(targetChanged)
    {
        character->TargetMoveCount = 0;
        character->AutoMoveStep = 0;
    }

    // Drive tracing immediately instead of waiting for the current stand clip
    // to wrap and consume ptNextTarget. This matters for player motion sets
    // whose idle clips are much longer than normal monster idles.
    int range = preferredRange > 0 ? preferredRange : character->smCharInfo.Shooting_Range + 32;
    if(range < 80) range = 80;
    if(range > 600) range = 600;
    const int dx = (target->pX - character->pX) >> FLOATNS;
    const int dz = (target->pZ - character->pZ) >> FLOATNS;
    const int distance = dx * dx + dz * dz;
    const int rangeSquared = range * range;

    // preferredRange is an explicit player policy. When configured, ranged
    // companions should also back away when an enemy collapses the distance
    // instead of only using the value as a stop-chasing threshold. Keep a
    // generous inner hysteresis so melee contact does not cause oscillation.
    const int innerRange = preferredRange > 0 ? std::max(64, (range * 55) / 100) : 0;
    if(innerRange > 0 && distance < innerRange * innerRange)
    {
        int awayX = (character->pX - target->pX) >> FLOATNS;
        int awayZ = (character->pZ - target->pZ) >> FLOATNS;
        if(awayX == 0 && awayZ == 0)
            awayX = 1;
        const int normalizer = std::max(1, std::max(abs(awayX), abs(awayZ)));
        const int step = std::max(32, std::min(120, range - innerRange));
        const int moveX = (character->pX >> FLOATNS) + (awayX * step) / normalizer;
        const int moveZ = (character->pZ >> FLOATNS) + (awayZ * step) / normalizer;
        character->SetTargetPosi2(moveX, moveZ);
        character->TargetMoveCount = 2;
        character->TragetTraceMode = FALSE;
        character->SetNextTarget(0, 0, 0);
        if(character->MotionInfo &&
           character->MotionInfo->State != CHRMOTION_STATE_DAMAGE &&
           character->MotionInfo->State != CHRMOTION_STATE_DEAD &&
           character->MotionInfo->State != CHRMOTION_STATE_ATTACK &&
           character->MotionInfo->State != CHRMOTION_STATE_SKILL)
        {
            character->SetMotionFromCode(CHRMOTION_STATE_RUN);
            if(!character->MotionInfo || character->MotionInfo->State != CHRMOTION_STATE_RUN)
                character->SetMotionFromCode(CHRMOTION_STATE_WALK);
        }
    }
    else if(distance >= rangeSquared)
    {
        if(combatSpacing)
        {
            int offsetX = 0;
            int offsetZ = 0;
            CombatFormationOffset(formationSlot, offsetX, offsetZ);
            character->SetTargetPosi2((target->pX >> FLOATNS) + offsetX,
                                      (target->pZ >> FLOATNS) + offsetZ);
            // ApplyAI refreshes this moving anchor every tick. Do not let the
            // legacy trace overwrite it with the target's exact centre.
            character->TragetTraceMode = FALSE;
        }
        else
        {
            character->SetTargetPosi2(target->pX >> FLOATNS, target->pZ >> FLOATNS);
            character->TragetTraceMode = TRUE;
        }
        // We drive the trace every AI tick, so do not also arm the legacy
        // deferred ptNextTarget state machine. It can restart STAND/RUN on
        // animation boundaries and causes visible animation popping.
        character->SetNextTarget(0, 0, 0);

        if(character->MotionInfo &&
            character->MotionInfo->State != CHRMOTION_STATE_DAMAGE &&
            character->MotionInfo->State != CHRMOTION_STATE_DEAD &&
            character->MotionInfo->State != CHRMOTION_STATE_ATTACK &&
            character->MotionInfo->State != CHRMOTION_STATE_SKILL)
        {
            // Combat chase should use the RUN clip/speed even if another legacy
            // movement path previously left the companion in WALK.
            if(character->MotionInfo->State != CHRMOTION_STATE_RUN)
            {
                character->SetMotionFromCode(CHRMOTION_STATE_RUN);
                if(!character->MotionInfo || character->MotionInfo->State != CHRMOTION_STATE_RUN)
                    character->SetMotionFromCode(CHRMOTION_STATE_WALK);
            }
        }
    }
    else
    {
        character->SetNextTarget(0, 0, 0);
    }

    // A normal monster primarily acquires rsPLAYINFO targets. Companions live
    // in the smCHAR pool, so seed char-vs-char aggro when the monster is idle.
    // Once either side lands a hit the stock combat loop keeps retaliation.
    if(!target->lpTargetPlayInfo && !target->lpTargetChar)
    {
        target->lpTargetChar = character;
        target->TargetMoveCount = 0;
        target->AutoMoveStep = 0;
        target->SetNextTarget(character->pX, character->pY, character->pZ);
    }
}

void DisengageTarget(smCHAR *character)
{
    if(!character)
        return;

    smCHAR *target = character->lpTargetChar;
    if(target && target->lpTargetChar == character && !target->lpTargetPlayInfo)
    {
        target->lpTargetChar = NULL;
        target->SetNextTarget(0, 0, 0);
    }

    character->lpTargetChar = NULL;
    character->lpTargetPlayInfo = NULL;
    character->SetNextTarget(0, 0, 0);
}


struct CompanionLootConfig
{
    CompanionLootConfig()
        : enabled(false), radius(260), ownerLeash(520), pickupDistance(36),
          scanMs(350), pickupCooldownMs(300), maxChaseMs(4500), spawnDelayMs(650),
          allowGold(true), allowCrystals(true), allowQuest(false),
          allowConsumables(true), allowEquipment(true), allowOther(false),
          allowUnique(true), minimumPrice(0), minimumDropLevel(0), loadedAt(0)
    {
    }

    bool enabled;
    int radius;
    int ownerLeash;
    int pickupDistance;
    DWORD scanMs;
    DWORD pickupCooldownMs;
    DWORD maxChaseMs;
    DWORD spawnDelayMs;
    bool allowGold;
    bool allowCrystals;
    bool allowQuest;
    bool allowConsumables;
    bool allowEquipment;
    bool allowOther;
    bool allowUnique;
    int minimumPrice;
    int minimumDropLevel;
    DWORD loadedAt;
};

static int ClampLootConfigInt(int value, int low, int high)
{
    if(value < low) return low;
    if(value > high) return high;
    return value;
}

static const CompanionLootConfig &GetCompanionLootConfig()
{
    static CompanionLootConfig config;
    if(config.loadedAt && (DWORD)(dwPlayServTime - config.loadedAt) < 5000U)
        return config;

    config.enabled = GetPrivateProfileIntA("Companion", "LootEnabled", 0, ".//Configs//Companion.ini") != 0;
    config.radius = ClampLootConfigInt(
        GetPrivateProfileIntA("Companion", "LootRadius", 260, ".//Configs//Companion.ini"), 80, 600);
    config.ownerLeash = ClampLootConfigInt(
        GetPrivateProfileIntA("Companion", "LootOwnerLeash", 520, ".//Configs//Companion.ini"), 120, 900);
    config.pickupDistance = ClampLootConfigInt(
        GetPrivateProfileIntA("Companion", "LootPickupDistance", 36, ".//Configs//Companion.ini"), 18, 90);
    config.scanMs = (DWORD)ClampLootConfigInt(
        GetPrivateProfileIntA("Companion", "LootScanMs", 350, ".//Configs//Companion.ini"), 100, 3000);
    config.pickupCooldownMs = (DWORD)ClampLootConfigInt(
        GetPrivateProfileIntA("Companion", "LootPickupCooldownMs", 300, ".//Configs//Companion.ini"), 100, 5000);
    config.maxChaseMs = (DWORD)ClampLootConfigInt(
        GetPrivateProfileIntA("Companion", "LootMaxChaseMs", 4500, ".//Configs//Companion.ini"), 1000, 15000);
    config.spawnDelayMs = (DWORD)ClampLootConfigInt(
        GetPrivateProfileIntA("Companion", "LootSpawnDelayMs", 650, ".//Configs//Companion.ini"), 0, 10000);
    config.allowGold = GetPrivateProfileIntA("Companion", "LootGold", 1, ".//Configs//Companion.ini") != 0;
    config.allowCrystals = GetPrivateProfileIntA("Companion", "LootCrystals", 1, ".//Configs//Companion.ini") != 0;
    config.allowQuest = GetPrivateProfileIntA("Companion", "LootQuestItems", 0, ".//Configs//Companion.ini") != 0;
    config.allowConsumables = GetPrivateProfileIntA("Companion", "LootConsumables", 1, ".//Configs//Companion.ini") != 0;
    config.allowEquipment = GetPrivateProfileIntA("Companion", "LootEquipment", 1, ".//Configs//Companion.ini") != 0;
    config.allowOther = GetPrivateProfileIntA("Companion", "LootOther", 0, ".//Configs//Companion.ini") != 0;
    config.allowUnique = GetPrivateProfileIntA("Companion", "LootUnique", 1, ".//Configs//Companion.ini") != 0;
    config.minimumPrice = ClampLootConfigInt(
        GetPrivateProfileIntA("Companion", "LootMinimumPrice", 0, ".//Configs//Companion.ini"), 0, 2000000000);
    config.minimumDropLevel = ClampLootConfigInt(
        GetPrivateProfileIntA("Companion", "LootMinimumDropLevel", 0, ".//Configs//Companion.ini"), 0, 1000);
    config.loadedAt = dwPlayServTime ? dwPlayServTime : 1U;
    return config;
}

enum CompanionLootCategory
{
    COMPANION_LOOT_GOLD = 0,
    COMPANION_LOOT_CRYSTAL,
    COMPANION_LOOT_QUEST,
    COMPANION_LOOT_CONSUMABLE,
    COMPANION_LOOT_EQUIPMENT,
    COMPANION_LOOT_OTHER
};

static CompanionLootCategory GetCompanionLootCategory(const sITEMINFO &item)
{
    const DWORD code = item.CODE;
    if(code == (sinGG1 | sin01) || code == (sinGG1 | sin05))
        return COMPANION_LOOT_GOLD;
    if((code & sinITEM_MASK2) == sinEC1)
        return COMPANION_LOOT_CRYSTAL;
    if(item.ItemKindCode == ITEM_KIND_QUEST || item.ItemKindCode == ITEM_KIND_QUEST_WEAPON ||
       (code & sinITEM_MASK2) == sinQT1)
        return COMPANION_LOOT_QUEST;
    if((code & sinITEM_MASK1) == (sinPM1 & sinITEM_MASK1))
        return COMPANION_LOOT_CONSUMABLE;

    const DWORD family = code & sinITEM_MASK1;
    if(family == 0x01000000 || family == 0x02000000 || family == 0x03000000)
        return COMPANION_LOOT_EQUIPMENT;
    return COMPANION_LOOT_OTHER;
}

static bool IsCompanionLootEligible(const STG_ITEMS &stageItem, rsPLAYINFO *owner,
    const CompanionLootConfig &config)
{
    if(!stageItem.state || !owner)
        return false;
    if(stageItem.dwExclusiveOwnerCode && stageItem.dwExclusiveOwnerCode != owner->dwObjectSerial)
        return false;
    if((int)stageItem.Item.ItemInfo.Price < config.minimumPrice)
        return false;
    if(stageItem.Item.ItemInfo.Level < config.minimumDropLevel)
        return false;
    if(stageItem.Item.ItemInfo.UniqueItem && !config.allowUnique)
        return false;

    // Tier-5 Cry pickup is intentionally kept out of Companion Loot while
    // Rank5 remains outside the current contract. It has quest-token side
    // effects beyond generic inventory pickup.
    if(stageItem.Item.ItemInfo.CODE == sinQT136)
        return false;

    switch(GetCompanionLootCategory(stageItem.Item.ItemInfo))
    {
        case COMPANION_LOOT_GOLD: return config.allowGold;
        case COMPANION_LOOT_CRYSTAL: return config.allowCrystals;
        case COMPANION_LOOT_QUEST: return config.allowQuest;
        case COMPANION_LOOT_CONSUMABLE: return config.allowConsumables;
        case COMPANION_LOOT_EQUIPMENT: return config.allowEquipment;
        default: return config.allowOther;
    }
}

static bool IsCompanionLootClaimed(STG_ITEMS *item, const CompanionSlot *requester)
{
    if(!item)
        return false;
    for(std::map<rsPLAYINFO *, OwnerState>::iterator it = g_states.begin(); it != g_states.end(); ++it)
    {
        OwnerState &state = it->second;
        for(int i = 0; i < COMPANION_MAX_ACTIVE; ++i)
        {
            CompanionSlot &slot = state.slots[i];
            if(&slot != requester && slot.lootTarget == item && slot.entity && slot.entity->Flag)
                return true;
        }
    }
    return false;
}

static void ReleaseCompanionLootTarget(CompanionSlot &slot)
{
    slot.lootArea = NULL;
    slot.lootTarget = NULL;
    slot.lootTargetSince = 0;
}

static STG_ITEMS *FindCompanionLootTarget(smCHAR *character, rsPLAYINFO *owner,
    CompanionSlot &slot, const CompanionLootConfig &config)
{
    if(!character || !owner)
        return NULL;
    STG_AREA *area = static_cast<STG_AREA *>(character->lpExt4);
    if(!area)
        return NULL;

    STG_ITEMS *best = NULL;
    long long bestDistance = 0x7FFFFFFFFFFFFFFFLL;
    const int charX = character->pX >> FLOATNS;
    const int charZ = character->pZ >> FLOATNS;
    const int ownerX = owner->Position.x >> FLOATNS;
    const int ownerZ = owner->Position.z >> FLOATNS;
    const long long radius2 = (long long)config.radius * config.radius;
    const long long leash2 = (long long)config.ownerLeash * config.ownerLeash;

    for(int i = 0; i < STG_ITEM_MAX; ++i)
    {
        STG_ITEMS *candidate = &area->StgItems[i];
        if(!IsCompanionLootEligible(*candidate, owner, config))
            continue;
        // Some drops intentionally move dwCreateTime into the future for a
        // short ownership/presentation delay. Never let Companion Loot bypass
        // the same visibility gate used by STG_AREA::SendStgItem().
        if((int)(candidate->dwCreateTime - dwPlayServTime) > 0)
            continue;
        if(config.spawnDelayMs && (DWORD)(dwPlayServTime - candidate->dwCreateTime) < config.spawnDelayMs)
            continue;
        if(candidate->dwLoseTime && (int)(candidate->dwLoseTime - dwPlayServTime) <= 1000)
            continue;
        if(IsCompanionLootClaimed(candidate, &slot))
            continue;

        const long long cdx = (long long)candidate->x - charX;
        const long long cdz = (long long)candidate->z - charZ;
        const long long distance = cdx * cdx + cdz * cdz;
        if(distance > radius2)
            continue;

        const long long odx = (long long)candidate->x - ownerX;
        const long long odz = (long long)candidate->z - ownerZ;
        if(odx * odx + odz * odz > leash2)
            continue;

        if(!best || distance < bestDistance ||
           (distance == bestDistance && candidate->dwExclusiveOwnerCode == owner->dwObjectSerial))
        {
            best = candidate;
            bestDistance = distance;
        }
    }
    return best;
}

static bool TryHandleCompanionLoot(smCHAR *character, rsPLAYINFO *owner,
    CompanionSlot &slot, CompanionMode mode)
{
    const CompanionLootConfig &config = GetCompanionLootConfig();
    if(!config.enabled || !character || !owner)
    {
        ReleaseCompanionLootTarget(slot);
        return false;
    }

    STG_AREA *area = static_cast<STG_AREA *>(character->lpExt4);
    if(!area)
    {
        ReleaseCompanionLootTarget(slot);
        return false;
    }

    // Do not steal time from a live combat decision. Aggressive companions also
    // decline fresh loot while a hostile is in sight, even before target lock.
    if(IsValidHostile(character->lpTargetChar) || IsValidHostile(owner->lpChrTarget) ||
       (mode == COMPANION_MODE_AGGRESSIVE && FindNearestHostile(character)))
    {
        ReleaseCompanionLootTarget(slot);
        return false;
    }

    if(slot.lootTarget)
    {
        if(slot.lootArea != area ||
           !IsCompanionLootEligible(*slot.lootTarget, owner, config) ||
           IsCompanionLootClaimed(slot.lootTarget, &slot) ||
           (slot.lootTargetSince && (DWORD)(dwPlayServTime - slot.lootTargetSince) > config.maxChaseMs))
            ReleaseCompanionLootTarget(slot);
    }

    if(!slot.lootTarget)
    {
        if(slot.nextLootScanTime && (int)(slot.nextLootScanTime - dwPlayServTime) > 0)
            return false;
        slot.nextLootScanTime = dwPlayServTime + config.scanMs;
        slot.lootTarget = FindCompanionLootTarget(character, owner, slot, config);
        if(!slot.lootTarget)
            return false;
        slot.lootArea = area;
        slot.lootTargetSince = dwPlayServTime;
        CompanionDebugLog("LOOT_TARGET name='%s' serial=%lu code=0x%08lX item='%s' x=%d z=%d exclusive=%lu",
            slot.characterName, (unsigned long)character->dwObjectSerial,
            (unsigned long)slot.lootTarget->Item.ItemInfo.CODE,
            slot.lootTarget->Item.ItemInfo.ItemName, slot.lootTarget->x, slot.lootTarget->z,
            (unsigned long)slot.lootTarget->dwExclusiveOwnerCode);
    }

    STG_ITEMS *item = slot.lootTarget;
    const int charX = character->pX >> FLOATNS;
    const int charZ = character->pZ >> FLOATNS;
    const int dx = item->x - charX;
    const int dz = item->z - charZ;
    const int distance2 = dx * dx + dz * dz;
    const int pickup2 = config.pickupDistance * config.pickupDistance;

    if(distance2 <= pickup2)
    {
        if(slot.nextLootPickupTime && (int)(slot.nextLootPickupTime - dwPlayServTime) > 0)
            return true;
        slot.nextLootPickupTime = dwPlayServTime + config.pickupCooldownMs;

        const DWORD code = item->Item.ItemInfo.CODE;
        char itemName[32] = { 0 };
        strncpy_s(itemName, sizeof(itemName), item->Item.ItemInfo.ItemName, _TRUNCATE);
        const int result = rsCompanionPickupStageItem(owner, area, item);
        CompanionDebugLog("LOOT_PICKUP name='%s' serial=%lu code=0x%08lX item='%s' result=%d",
            slot.characterName, (unsigned long)character->dwObjectSerial,
            (unsigned long)code, itemName, result);
        ReleaseCompanionLootTarget(slot);
        return result == TRUE;
    }

    DisengageTarget(character);
    character->SetTargetPosi2(item->x, item->z);
    character->TargetMoveCount = 2;
    character->TragetTraceMode = FALSE;
    if(character->MotionInfo &&
       character->MotionInfo->State != CHRMOTION_STATE_DEAD &&
       character->MotionInfo->State != CHRMOTION_STATE_DAMAGE)
    {
        character->SetMotionFromCode(CHRMOTION_STATE_RUN);
        if(!character->MotionInfo || character->MotionInfo->State != CHRMOTION_STATE_RUN)
            character->SetMotionFromCode(CHRMOTION_STATE_WALK);
    }
    return true;
}

bool IsNameActive(const OwnerState &state, const char *name)
{
    for(int i = 0; i < COMPANION_MAX_ACTIVE; ++i)
    {
        if(state.slots[i].characterName[0] &&
            lstrcmpiA(state.slots[i].characterName, name) == 0)
            return true;
    }
    return false;
}

int FindFreeSlot(OwnerState &state)
{
    for(int i = 0; i < COMPANION_MAX_ACTIVE; ++i)
    {
        if(!state.slots[i].characterName[0])
            return i;
    }
    return -1;
}

template <size_t N>
void CopyText32(char (&dest)[N], const char *source)
{
    dest[0] = 0;
    if(source && source[0])
        strncpy_s(dest, sizeof(dest), source, _TRUNCATE);
}

bool HasChoice(const EquipmentChoice *choices, int count, DWORD code)
{
    for(int i = 0; i < count; ++i)
    {
        if(choices[i].code == code)
            return true;
    }
    return false;
}

void AddChoice(EquipmentChoice *choices, int &count, DWORD code, const char *name)
{
    if(count >= COMPANION_UI_MAX_EQUIP_CHOICES || HasChoice(choices, count, code))
        return;

    choices[count].code = code;
    CopyText32(choices[count].name, name);
    ++count;
}

const char *ChoiceName(const EquipmentChoice *choices, int count, DWORD code)
{
    for(int i = 0; i < count; ++i)
    {
        if(choices[i].code == code)
            return choices[i].name;
    }
    return "";
}

void ReadCompanionLogicalSkillPoints(const TRANS_RECORD_DATA &recordData, BYTE *outPoints, int capacity)
{
    if(!outPoints || capacity <= 0)
        return;

    const int logicalCapacity = std::min(capacity, COMPANION_MAX_SKILL_SLOTS);
    ZeroMemory(outPoints, logicalCapacity * sizeof(BYTE));

    // T1-T4 keep the historical RecordSkill layout: logical slot 16 is byte 0.
    for(int useSlot = 1; useSlot <= 16 && useSlot <= logicalCapacity; ++useSlot)
        outPoints[useSlot - 1] = (BYTE)std::min(10,
            (int)recordData.GameSaveInfo.SkillInfo.bSkillPoint[useSlot & 15]);

    // Rank5 is the first legacy extension and lives in dwTemp2. Future persisted
    // tiers are added only in this bridge; executor/catalog/UI already address 1..64.
    for(int tier5Index = 0; tier5Index < 4; ++tier5Index)
    {
        const int logicalIndex = 16 + tier5Index;
        if(logicalIndex >= logicalCapacity)
            break;
        outPoints[logicalIndex] = (BYTE)std::min(10,
            GetTier5RecordSkillPoint(&recordData.GameSaveInfo, tier5Index));
    }
}

bool ReadEquipmentSnapshot(const TRANS_RECORD_DATA &recordData, CharacterEquipmentSnapshot &snapshot)
{
    snapshot = CharacterEquipmentSnapshot();
    snapshot.valid = true;
    CopyText32(snapshot.characterName, recordData.smCharInfo.szName);
    snapshot.level = recordData.smCharInfo.Level;
    snapshot.jobCode = recordData.smCharInfo.JOB_CODE;
    snapshot.life = recordData.smCharInfo.Life[0];
    snapshot.maxLife = recordData.smCharInfo.Life[1];
    snapshot.mana = recordData.smCharInfo.Mana[0];
    snapshot.maxMana = recordData.smCharInfo.Mana[1];
    ReadCompanionLogicalSkillPoints(recordData, snapshot.skillPoints, COMPANION_MAX_SKILL_SLOTS);

    // Always allow an empty hand from the UI. It is a visual/combat loadout
    // override only and never deletes or moves an inventory record.
    AddChoice(snapshot.right, snapshot.rightCount, 0, "Nenhum");
    AddChoice(snapshot.left, snapshot.leftCount, 0, "Nenhum");

    if(recordData.DataSize <= 0)
        return true;

    const BYTE *cursor = recordData.Data;
    const BYTE *end = recordData.Data + recordData.DataSize;
    int primaryItemCount = recordData.ItemSubStart;
    if(primaryItemCount <= 0 || primaryItemCount > recordData.ItemCount)
        primaryItemCount = recordData.ItemCount;

    for(int item = 0; item < primaryItemCount && cursor + sizeof(int) <= end; ++item)
    {
        const int packedSize = *(const int *)cursor;
        if(packedSize <= (int)sizeof(int) || cursor + packedSize > end)
            break;

        sRECORD_ITEM recordItem;
        ZeroMemory(&recordItem, sizeof(recordItem));
        DecodeCompress((BYTE *)cursor, (BYTE *)&recordItem, sizeof(recordItem));
        cursor += packedSize;

        const DWORD code = recordItem.sItemInfo.CODE;
        if(!code)
            continue;

        const int itemIndex = GetSinItemNumFromCode(code);
        const DWORD itemClass =
            itemIndex >= 0 && itemIndex < MAX_ITEM ? sItem[itemIndex].Class : 0;

        if(itemClass == ITEM_CLASS_WEAPON_ONE || itemClass == ITEM_CLASS_WEAPON_TWO)
            AddChoice(snapshot.right, snapshot.rightCount, code, recordItem.sItemInfo.ItemName);

        // The login inventory accepts an item in a slot when the slot's
        // Position bit intersects the item's Class. Slot 2 is LHAND, so keep
        // every genuine left-hand class (shield/magical-stuff/custom) except a
        // two-handed weapon: that weapon is mirrored into position 2 by
        // SetTwoHandItem and must remain a single right-hand loadout choice.
        // Also preserve a one-handed weapon explicitly serialized in position
        // 2 for dual-wield/custom content even though the stock class mask is
        // RHAND-only.
        const bool leftHandClass =
            (itemClass & INVENTORY_POS_LHAND) != 0 &&
            itemClass != ITEM_CLASS_WEAPON_TWO;
        if(leftHandClass ||
            (itemClass == ITEM_CLASS_WEAPON_ONE && recordItem.ItemPosition == 2))
        {
            AddChoice(snapshot.left, snapshot.leftCount, code, recordItem.sItemInfo.ItemName);
        }

        // This is intentionally the same persisted-slot contract used by the
        // real client inventory: sInven[0] is RHAND, sInven[1] is LHAND, and
        // load/restore assigns sInven[ItemPosition - 1]. The source validator
        // tools/validate_companion_equipment_contract.py guards all three
        // sides of that contract so this does not drift back into guesswork.
        if(recordItem.ItemPosition == 1 && !snapshot.defaultRight)
        {
            snapshot.defaultRight = code;
            CopyText32(snapshot.defaultRightName, recordItem.sItemInfo.ItemName);
        }
        else if(recordItem.ItemPosition == 2 && !snapshot.defaultLeft)
        {
            snapshot.defaultLeft = code;
            CopyText32(snapshot.defaultLeftName, recordItem.sItemInfo.ItemName);
        }
    }

    const int defaultRightIndex = snapshot.defaultRight ? GetSinItemNumFromCode(snapshot.defaultRight) : -1;
    if(snapshot.defaultLeft == snapshot.defaultRight &&
        IsCompanionTwoHandCode(snapshot.defaultRight, defaultRightIndex))
    {
        snapshot.defaultLeft = 0;
        snapshot.defaultLeftName[0] = 0;
    }

    // Keep the saved loadout representable even when the item catalogue could
    // not classify a custom code. This is especially useful while asset/data
    // tables are being migrated independently between client and server.
    if(snapshot.defaultRight && !HasChoice(snapshot.right, snapshot.rightCount, snapshot.defaultRight))
        AddChoice(snapshot.right, snapshot.rightCount, snapshot.defaultRight, snapshot.defaultRightName);
    if(snapshot.defaultLeft && !HasChoice(snapshot.left, snapshot.leftCount, snapshot.defaultLeft))
        AddChoice(snapshot.left, snapshot.leftCount, snapshot.defaultLeft, snapshot.defaultLeftName);

    return true;
}

bool LoadCharacterSnapshot(rsPLAYINFO *owner, const char *characterName, CharacterEquipmentSnapshot &snapshot)
{
    if(!owner || !owner->getAccount() || !owner->getAccount()[0] || !characterName || !characterName[0])
        return false;

    TRANS_RECORD_DATA recordData;
    ZeroMemory(&recordData, sizeof(recordData));
    if(!rsLoadAccountCharacterRecord(owner->getAccount(), characterName, &recordData))
        return false;

    return ReadEquipmentSnapshot(recordData, snapshot);
}

LoadoutOverride *FindLoadoutOverride(OwnerState &state, const char *characterName, bool create)
{
    LoadoutOverride *entry = NULL;
    LoadoutOverride *freeEntry = NULL;
    for(int i = 0; i < protocol::COMPANION_UI_MAX_CHARACTERS; ++i)
    {
        LoadoutOverride &candidate = state.loadouts[i];
        if(candidate.characterName[0] && lstrcmpiA(candidate.characterName, characterName) == 0)
        {
            entry = &candidate;
            break;
        }
        if(!freeEntry && !candidate.characterName[0])
            freeEntry = &candidate;
    }

    if(!entry && create && freeEntry)
    {
        entry = freeEntry;
        CopyText32(entry->characterName, characterName);
    }

    if(entry && !entry->loaded)
    {
        CompanionProfile profile;
        if(state.owner && state.owner->getAccount() &&
            LoadCompanionProfile(state.owner->getAccount(), characterName, &profile))
        {
            entry->active = profile.loadoutActive;
            entry->rightCode = profile.rightItemCode;
            entry->leftCode = profile.leftItemCode;
            entry->mode = static_cast<CompanionMode>(profile.mode);
            entry->tactic = profile.tactic;
            entry->retreatHpPercent = profile.retreatHpPercent;
            entry->preferredRange = profile.preferredRange;
            entry->formationSlot = profile.formationSlot;
            entry->skillLoadoutActive = profile.skillLoadoutActive;
            entry->buffLoadoutActive = profile.buffLoadoutActive;
            memcpy(entry->activeSkillSlots, profile.activeSkillSlots, sizeof(entry->activeSkillSlots));
            memcpy(entry->activeMinResourcePercent, profile.activeMinResourcePercent, sizeof(entry->activeMinResourcePercent));
            memcpy(entry->activeMinimumTargets, profile.activeMinimumTargets, sizeof(entry->activeMinimumTargets));
            memcpy(entry->activeTargetModes, profile.activeTargetModes, sizeof(entry->activeTargetModes));
            memcpy(entry->buffSkillSlots, profile.buffSkillSlots, sizeof(entry->buffSkillSlots));
            memcpy(entry->buffIntervalSeconds, profile.buffIntervalSeconds, sizeof(entry->buffIntervalSeconds));
            memcpy(entry->buffTargetModes, profile.buffTargetModes, sizeof(entry->buffTargetModes));
        }
        if(state.owner && state.owner->getAccount())
            LoadCompanionTactics(state.owner->getAccount(), characterName, &entry->customTactics);
        entry->loaded = true;
    }
    return entry;
}

bool PersistLoadoutOverride(OwnerState &state, const LoadoutOverride &loadout)
{
    if(!state.owner || !state.owner->getAccount() || !state.owner->getAccount()[0] || !loadout.characterName[0])
        return false;

    CompanionProfile profile;
    LoadCompanionProfile(state.owner->getAccount(), loadout.characterName, &profile);
    profile.loadoutActive = loadout.active;
    profile.rightItemCode = loadout.rightCode;
    profile.leftItemCode = loadout.leftCode;
    profile.mode = static_cast<int>(loadout.mode);
    profile.tactic = loadout.tactic;
    profile.retreatHpPercent = loadout.retreatHpPercent;
    profile.preferredRange = loadout.preferredRange;
    profile.formationSlot = loadout.formationSlot;
    profile.skillLoadoutActive = loadout.skillLoadoutActive;
    profile.buffLoadoutActive = loadout.buffLoadoutActive;
    memcpy(profile.activeSkillSlots, loadout.activeSkillSlots, sizeof(profile.activeSkillSlots));
    memcpy(profile.activeMinResourcePercent, loadout.activeMinResourcePercent, sizeof(profile.activeMinResourcePercent));
    memcpy(profile.activeMinimumTargets, loadout.activeMinimumTargets, sizeof(profile.activeMinimumTargets));
    memcpy(profile.activeTargetModes, loadout.activeTargetModes, sizeof(profile.activeTargetModes));
    memcpy(profile.buffSkillSlots, loadout.buffSkillSlots, sizeof(profile.buffSkillSlots));
    memcpy(profile.buffIntervalSeconds, loadout.buffIntervalSeconds, sizeof(profile.buffIntervalSeconds));
    memcpy(profile.buffTargetModes, loadout.buffTargetModes, sizeof(profile.buffTargetModes));
    return SaveCompanionProfile(state.owner->getAccount(), loadout.characterName, profile);
}

bool IsValidHostile(smCHAR *candidate)
{
    return candidate && candidate->Flag &&
        candidate->smCharInfo.Life[0] > 0 &&
        candidate->smCharInfo.State == smCHAR_STATE_ENEMY &&
        candidate->smCharInfo.Brood != smCHAR_MONSTER_USER &&
        !candidate->lpMasterPlayInfo;
}

bool IsCompanionTwoHandCode(DWORD code, int itemIndex)
{
    if(!code)
        return false;

    if(itemIndex >= 0 && itemIndex < MAX_ITEM &&
        sItem[itemIndex].Class == ITEM_CLASS_WEAPON_TWO)
        return true;

    // Captured DATs mirror bow/pike codes into RHAND and LHAND. Custom item
    // catalogues can disagree on Class, so recognize the canonical families too.
    const DWORD family = code & sinITEM_MASK2;
    return family == sinWS1 || family == sinWP1;
}

void FormationOffset(int slot, int &x, int &z)
{
    static const int offsets[COMPANION_MAX_ACTIVE][2] =
    {
        { -70, -70 },
        {  70, -70 },
        { -70,  70 },
        {  70,  70 }
    };
    if(slot < 0 || slot >= COMPANION_MAX_ACTIVE)
        slot = 0;
    x = offsets[slot][0] << FLOATNS;
    z = offsets[slot][1] << FLOATNS;
}

CompanionSlot *FindSlotByName(OwnerState &state, const char *characterName)
{
    for(int i = 0; i < COMPANION_MAX_ACTIVE; ++i)
    {
        if(state.slots[i].characterName[0] &&
            lstrcmpiA(state.slots[i].characterName, characterName) == 0)
            return &state.slots[i];
    }
    return NULL;
}

bool HasIncomingCompanionThreat(smCHAR *companion)
{
    if(!companion || !companion->Flag)
        return false;
    STG_AREA *area = static_cast<STG_AREA *>(companion->lpExt4);
    if(!area)
        return false;
    for(int i = 0; i < area->MonsterLastPoint; ++i)
    {
        smCHAR *candidate = area->lpCharMonster[i];
        if(IsValidHostile(candidate) && candidate->lpTargetChar == companion)
            return true;
    }
    return false;
}

void SendRuntimeStateLocked(OwnerState &state, bool force)
{
    if(!IsOwnerValid(state) || !state.owner->lpsmSock)
        return;

    if(!force && (DWORD)(dwPlayServTime - state.lastRuntimeSendTime) < 1000U)
        return;

    protocol::CompanionRuntimeStatePacket packet;
    ZeroMemory(&packet, sizeof(packet));
    packet.size = sizeof(packet);
    packet.code = protocol::OPCODE_COMPANION_RUNTIME_STATE;
    packet.version = protocol::COMPANION_RUNTIME_PROTOCOL_VERSION;

    for(int i = 0; i < COMPANION_MAX_ACTIVE && packet.entryCount < protocol::COMPANION_RUNTIME_MAX_ACTIVE; ++i)
    {
        CompanionSlot &slot = state.slots[i];
        if(!IsSlotActive(state, slot) || !slot.entity)
            continue;

        protocol::CompanionRuntimeEntry &entry = packet.entries[packet.entryCount++];
        CopyText32(entry.characterName, slot.characterName);
        entry.objectSerial = slot.entity->dwObjectSerial;
        entry.level = slot.entity->smCharInfo.Level;
        entry.jobCode = slot.entity->smCharInfo.JOB_CODE;
        entry.life = slot.entity->smCharInfo.Life[0];
        entry.maxLife = slot.entity->smCharInfo.Life[1];
        entry.mana = slot.entity->smCharInfo.Mana[0];
        entry.maxMana = slot.entity->smCharInfo.Mana[1];
        entry.dead = entry.maxLife > 0 && entry.life <= 0 ? TRUE : FALSE;
        const DWORD now = dwPlayServTime;
        const bool locked = slot.skillRuntime.skillLockUntil &&
            (int)(slot.skillRuntime.skillLockUntil - now) > 0;
        entry.casting = locked ? TRUE : FALSE;
        entry.castingSkillCode = locked ? slot.skillRuntime.lastSkillCode : 0;
        if(locked && slot.skillRuntime.skillLockStartedAt &&
           (int)(slot.skillRuntime.skillLockUntil - slot.skillRuntime.skillLockStartedAt) > 0)
        {
            const DWORD duration = slot.skillRuntime.skillLockUntil - slot.skillRuntime.skillLockStartedAt;
            const DWORD elapsed = now - slot.skillRuntime.skillLockStartedAt;
            entry.castProgressPermille = (int)std::min<unsigned long long>(1000ULL,
                ((unsigned long long)elapsed * 1000ULL) / duration);
        }
        if(entry.castingSkillCode)
        {
            CompanionSkillCatalogEntry catalogEntry;
            if(GetCompanionSkillCatalogEntry(entry.castingSkillCode, &catalogEntry))
                CopyText32(entry.castingSkillName, catalogEntry.name);
        }

        DWORD cooldownUntil = 0;
        if(slot.skillRuntime.nextDamageSkillTime && (int)(slot.skillRuntime.nextDamageSkillTime - now) > 0)
            cooldownUntil = slot.skillRuntime.nextDamageSkillTime;
        if(slot.skillRuntime.nextHealSkillTime && (int)(slot.skillRuntime.nextHealSkillTime - now) > 0 &&
           (!cooldownUntil || (int)(slot.skillRuntime.nextHealSkillTime - cooldownUntil) < 0))
            cooldownUntil = slot.skillRuntime.nextHealSkillTime;
        for(int buffSlot = 0; buffSlot < 4; ++buffSlot)
        {
            const DWORD due = slot.skillRuntime.nextBuffSkillTime[buffSlot];
            if(due && (int)(due - now) > 0 && (!cooldownUntil || (int)(due - cooldownUntil) < 0))
                cooldownUntil = due;
        }
        entry.cooldownMs = cooldownUntil ? std::max(0, (int)(cooldownUntil - now)) : 0;

        if(slot.entity->PlaySlowCount) entry.statusFlags |= protocol::COMPANION_RUNTIME_STATUS_SLOW;
        if(slot.entity->PlayStunCount > 0) entry.statusFlags |= protocol::COMPANION_RUNTIME_STATUS_STUN;
        if(slot.entity->PlayPoison[1] > 0) entry.statusFlags |= protocol::COMPANION_RUNTIME_STATUS_POISON;
        if(slot.entity->PlayDistortion > 0) entry.statusFlags |= protocol::COMPANION_RUNTIME_STATUS_DISTORTION;
        if(HasIncomingCompanionThreat(slot.entity)) entry.statusFlags |= protocol::COMPANION_RUNTIME_STATUS_THREAT;

        const auto addEffect = [&](DWORD code, DWORD expiresAt)
        {
            if(!code || !expiresAt || (int)(expiresAt - now) <= 0 ||
               entry.effectCount >= protocol::COMPANION_RUNTIME_MAX_EFFECTS)
                return;
            protocol::CompanionRuntimeEffect &effect = entry.effects[entry.effectCount++];
            effect.effectCode = code;
            effect.remainingMs = std::max(0, (int)(expiresAt - now));
            effect.positive = TRUE;
        };
        for(int buffSlot = 0; buffSlot < 4; ++buffSlot)
            addEffect(slot.skillRuntime.buffModifiers[buffSlot].skillCode,
                slot.skillRuntime.buffModifiers[buffSlot].expiresAt);
        addEffect(SKILL_FORCE_OF_NATURE, slot.skillRuntime.forceNatureExpiresAt);
        addEffect(SKILL_HOLY_VALOR, slot.skillRuntime.holyValorExpiresAt);
        addEffect(SKILL_TRIUMPH_OF_VALHALLA, slot.skillRuntime.triumphValhallaExpiresAt);
        addEffect(SKILL_HALL_OF_VALHALLA, slot.skillRuntime.hallValhallaExpiresAt);
        addEffect(SKILL_REGENERATION_FIELD, std::max(slot.skillRuntime.regenerationFieldExpiresAt,
            slot.skillRuntime.regenerationFieldPulseExpiresAt));
        addEffect(SKILL_ENCHANT_WEAPON, slot.skillRuntime.enchantWeaponExpiresAt);
        addEffect(slot.skillRuntime.goldenFalconPoint > 0 ? SKILL_GOLDEN_FALCON : SKILL_FALCON,
            slot.skillRuntime.falconExpiresAt);
        addEffect(slot.skillRuntime.persistentEffectSkillCode, slot.skillRuntime.persistentEffectExpiresAt);

        LoadoutOverride *profile = FindLoadoutOverride(state, slot.characterName, true);
        entry.mode = profile ? static_cast<int>(profile->mode) : static_cast<int>(state.mode);
        entry.tactic = profile ? static_cast<int>(profile->tactic) : static_cast<int>(COMPANION_TACTIC_BALANCED);

        if(slot.entity->lpTargetChar && slot.entity->lpTargetChar->Flag)
            CopyText32(entry.targetName, slot.entity->lpTargetChar->smCharInfo.szName);
    }

    state.owner->lpsmSock->Send((char *)&packet, packet.size, TRUE);
    state.lastRuntimeSendTime = dwPlayServTime;
}

static bool IsOneShotVisualMotion(DWORD motionState)
{
    return motionState == CHRMOTION_STATE_ATTACK ||
        motionState == CHRMOTION_STATE_DAMAGE ||
        motionState == CHRMOTION_STATE_SKILL ||
        motionState == CHRMOTION_STATE_DEAD;
}

void UpdateVisualMotionSequence(CompanionSlot &slot)
{
    if(!slot.entity || !slot.entity->Flag)
        return;

    DWORD motionState = CHRMOTION_STATE_STAND;
    if(slot.entity->MotionInfo)
        motionState = (DWORD)slot.entity->MotionInfo->State;

    // The server-side entity borrows a monster pattern and may legitimately
    // lack a RUN clip. In that case SetMotionFromCode falls back to WALK even
    // though account companions are always supposed to run while locomoting.
    // The client owns the real player model, so publish RUN for this internal
    // WALK fallback and let the client select its proper player run animation.
    if(motionState == CHRMOTION_STATE_WALK &&
       slot.entity->smCharInfo.State == smCHAR_STATE_USER &&
       slot.entity->smCharInfo.Brood == smCHAR_MONSTER_USER &&
       slot.entity->lpMasterPlayInfo)
    {
        motionState = CHRMOTION_STATE_RUN;
    }

    const int motionFrame = slot.entity->frame;
    bool advance = false;
    if(slot.visualMotionSequence == 0)
        advance = true;
    else if(slot.visualMotionState != motionState)
        advance = true;
    else if(IsOneShotVisualMotion(motionState) &&
        motionState != CHRMOTION_STATE_DEAD &&
        motionFrame + 160 < slot.visualMotionFrame)
    {
        // A repeated attack/skill can restart without visiting STAND between
        // two 10 Hz visual snapshots. Detect the server-side frame wrap only
        // to advance the event sequence; the frame value itself is never sent.
        advance = true;
    }

    if(advance)
    {
        ++slot.visualMotionSequence;
        if(slot.visualMotionSequence == 0)
            slot.visualMotionSequence = 1;
    }

    slot.visualMotionState = motionState;
    slot.visualMotionFrame = motionFrame;
}

void SendVisualStateLocked(OwnerState &state, bool force)
{
    if(!IsOwnerValid(state) || !state.owner->lpsmSock)
        return;

    if(!force && (DWORD)(dwPlayServTime - state.lastVisualSendTime) < 100U)
        return;

    protocol::CompanionVisualStatePacket packet;
    ZeroMemory(&packet, sizeof(packet));
    packet.size = sizeof(packet);
    packet.code = protocol::OPCODE_COMPANION_VISUAL_STATE;
    packet.version = protocol::COMPANION_VISUAL_PROTOCOL_VERSION;

    for(int i = 0; i < COMPANION_MAX_ACTIVE && packet.entryCount < protocol::COMPANION_VISUAL_MAX_ACTIVE; ++i)
    {
        CompanionSlot &slot = state.slots[i];
        if(!slot.characterName[0] || !slot.entity || !slot.entity->Flag ||
            slot.entity->lpMasterPlayInfo != state.owner)
            continue;

        UpdateVisualMotionSequence(slot);

        protocol::CompanionVisualEntry &entry = packet.entries[packet.entryCount++];
        CopyText32(entry.characterName, slot.characterName);
        entry.objectSerial = slot.entity->dwObjectSerial;
        entry.x = slot.entity->pX;
        entry.y = slot.entity->pY;
        entry.z = slot.entity->pZ;
        entry.angleY = slot.entity->Angle.y;
        entry.motionState = slot.visualMotionState ? slot.visualMotionState : (DWORD)CHRMOTION_STATE_STAND;
        entry.motionSequence = slot.visualMotionSequence ? slot.visualMotionSequence : 1U;
        entry.life = slot.entity->smCharInfo.Life[0];
        entry.maxLife = slot.entity->smCharInfo.Life[1];
        entry.rightItemCode = slot.entity->smCharInfo.dwTemp[smCOMPANION_NET_TEMP_RIGHT_ITEM_CODE];
        entry.leftItemCode = slot.entity->smCharInfo.dwTemp[smCOMPANION_NET_TEMP_LEFT_ITEM_CODE];
        entry.equipmentRevision = slot.entity->smCharInfo.dwTemp[smCOMPANION_NET_TEMP_EQUIP_REVISION];
        if(entry.equipmentRevision == 0)
            entry.equipmentRevision = 1;
        entry.logicalSkillCode = slot.skillRuntime.lastSkillCode;
        entry.skillPlayCode = slot.skillRuntime.lastSkillPlayCode;
        entry.skillPoint = slot.skillRuntime.lastSkillPoint;
        entry.skillTargetSerial = slot.skillRuntime.lastSkillTargetSerial;
        entry.skillTargetX = slot.skillRuntime.lastSkillTargetX;
        entry.skillTargetY = slot.skillRuntime.lastSkillTargetY;
        entry.skillTargetZ = slot.skillRuntime.lastSkillTargetZ;
        entry.skillMotionState = slot.skillRuntime.lastSkillMotionState;
        entry.skillSequence = slot.skillRuntime.skillVisualSequence;
        if(entry.maxLife > 0 && entry.life <= 0)
            entry.flags |= protocol::COMPANION_VISUAL_FLAG_DEAD;
    }

    state.owner->lpsmSock->Send((char *)&packet, packet.size, TRUE);
    state.lastVisualSendTime = dwPlayServTime;
}

void SendFullCompanionInfo(rsPLAYINFO *owner, smCHAR *companion)
{
    if(!owner || !owner->lpsmSock || !companion || !companion->Flag)
        return;

    smTRNAS_PLAYERINFO info;
    ZeroMemory(&info, sizeof(info));
    memcpy(&info.smCharInfo, &companion->smCharInfo, sizeof(smCHAR_INFO));
    info.size = sizeof(info);
    info.code = OpCode::OPCODE_PLAYERINFO;
    info.dwObjectSerial = companion->dwObjectSerial;
    info.x = companion->pX;
    info.y = companion->pY;
    info.z = companion->pZ;
    owner->lpsmSock->Send((char *)&info, info.size, TRUE);
}

void ApplyEquipmentCodes(rsPLAYINFO *owner, smCHAR *companion, DWORD rightCode, DWORD leftCode)
{
    if(!companion)
        return;

    int rightIndex = rightCode ? GetSinItemNumFromCode(rightCode) : -1;
    int leftIndex = leftCode ? GetSinItemNumFromCode(leftCode) : -1;

    if(rightIndex < 0 || rightIndex >= MAX_ITEM)
    {
        rightIndex = -1;
        rightCode = 0;
    }
    if(leftIndex < 0 || leftIndex >= MAX_ITEM)
    {
        leftIndex = -1;
        leftCode = 0;
    }

    if(CompanionDebugEnabled())
    {
        const int rightClass = rightIndex >= 0 ? sItem[rightIndex].Class : -1;
        const int leftClass = leftIndex >= 0 ? sItem[leftIndex].Class : -1;
        CompanionDebugLog("EQUIP_INPUT name='%s' serial=%lu rightCode=0x%08lX rightIndex=%d rightClass=0x%X leftCode=0x%08lX leftIndex=%d leftClass=0x%X",
            companion->smCharInfo.szName, (unsigned long)companion->dwObjectSerial,
            (unsigned long)rightCode, rightIndex, rightClass,
            (unsigned long)leftCode, leftIndex, leftClass);
    }

    if(IsCompanionTwoHandCode(rightCode, rightIndex))
    {
        leftIndex = -1;
        leftCode = 0;
    }
    else if(IsCompanionTwoHandCode(leftCode, leftIndex))
    {
        // Old profiles/DAT snapshots can carry a two-handed weapon only in the
        // left slot. Promote it instead of publishing an invalid LHAND mesh.
        if(!rightCode)
        {
            rightCode = leftCode;
            rightIndex = leftIndex;
        }
        leftIndex = -1;
        leftCode = 0;
    }

    companion->wStickItems[smPLAYBUFF_ITEM_RIGHT] = rightIndex >= 0 ? (short)rightIndex : -1;
    companion->wStickItems[smPLAYBUFF_ITEM_LEFT] = leftIndex >= 0 ? (short)leftIndex : -1;

    // Full PLAYERINFO refreshes can be generated by legacy paths that do not
    // know whether zero hand codes mean "intentionally unequipped" or merely
    // "metadata not populated". Bump a companion-owned revision only when the
    // authoritative loadout actually changes; the client can then reject stale
    // zero snapshots without preventing a real unequip operation.
    DWORD equipRevision = companion->smCharInfo.dwTemp[smCOMPANION_NET_TEMP_EQUIP_REVISION];
    equipRevision = equipRevision ? (equipRevision + 1U) : 1U;
    if(!equipRevision) equipRevision = 1U;
    companion->smCharInfo.dwTemp[smCOMPANION_NET_TEMP_EQUIP_REVISION] = equipRevision;
    companion->smCharInfo.dwTemp[smCOMPANION_NET_TEMP_MAGIC] = smCOMPANION_NET_MAGIC;
    companion->smCharInfo.dwTemp[smCOMPANION_NET_TEMP_RIGHT_ITEM_CODE] = rightCode;
    companion->smCharInfo.dwTemp[smCOMPANION_NET_TEMP_LEFT_ITEM_CODE] = leftCode;

    if(CompanionDebugEnabled())
        CompanionDebugLog("EQUIP_OUTPUT name='%s' serial=%lu rightCode=0x%08lX rightIndex=%d leftCode=0x%08lX leftIndex=%d",
            companion->smCharInfo.szName, (unsigned long)companion->dwObjectSerial,
            (unsigned long)rightCode, rightIndex, (unsigned long)leftCode, leftIndex);

    companion->dwActionItemCode = 0;
    if(rightIndex >= 0 &&
        (sItem[rightIndex].Class == ITEM_CLASS_WEAPON_ONE ||
         sItem[rightIndex].Class == ITEM_CLASS_WEAPON_TWO))
    {
        companion->dwActionItemCode = rightCode;
    }
    else if(leftIndex >= 0 &&
        (sItem[leftIndex].Class == ITEM_CLASS_WEAPON_ONE ||
         sItem[leftIndex].Class == ITEM_CLASS_WEAPON_TWO))
    {
        companion->dwActionItemCode = leftCode;
    }

    // Re-evaluate the current player motion against the newly selected weapon,
    // then seed the normal replication buffer and send one full info snapshot
    // so canonical item CODEs arrive immediately at the client.
    if(companion->MotionInfo &&
        companion->MotionInfo->State != CHRMOTION_STATE_DEAD &&
        companion->MotionInfo->State != CHRMOTION_STATE_DAMAGE)
    {
        companion->SetMotionFromCode(companion->MotionInfo->State);
    }
    companion->SavePlayBuff2();
    SendFullCompanionInfo(owner, companion);
}

DWORD CycleChoice(const EquipmentChoice *choices, int count, DWORD current, int direction)
{
    if(count <= 0)
        return 0;

    int index = 0;
    for(int i = 0; i < count; ++i)
    {
        if(choices[i].code == current)
        {
            index = i;
            break;
        }
    }

    index += direction >= 0 ? 1 : -1;
    if(index >= count) index = 0;
    if(index < 0) index = count - 1;
    return choices[index].code;
}

int ChangeEquipment(rsPLAYINFO *owner, const char *characterName, bool rightHand, int direction)
{
    CharacterEquipmentSnapshot snapshot;
    if(!LoadCharacterSnapshot(owner, characterName, snapshot))
        return -3;

    std::lock_guard<std::mutex> guard(g_mutex);
    OwnerState &state = GetOrCreateState(owner);
    PruneState(state);

    LoadoutOverride *loadout = FindLoadoutOverride(state, characterName, true);
    if(!loadout)
        return -9;

    if(!loadout->active)
    {
        loadout->rightCode = snapshot.defaultRight;
        loadout->leftCode = snapshot.defaultLeft;
        loadout->active = true;
    }

    const LoadoutOverride previous = *loadout;
    if(rightHand)
        loadout->rightCode = CycleChoice(snapshot.right, snapshot.rightCount, loadout->rightCode, direction);
    else
        loadout->leftCode = CycleChoice(snapshot.left, snapshot.leftCount, loadout->leftCode, direction);

    const int rightIndex = loadout->rightCode ? GetSinItemNumFromCode(loadout->rightCode) : -1;
    if(rightIndex >= 0 && rightIndex < MAX_ITEM && sItem[rightIndex].Class == ITEM_CLASS_WEAPON_TWO)
        loadout->leftCode = 0;

    if(!PersistLoadoutOverride(state, *loadout))
    {
        *loadout = previous;
        return -11;
    }

    CompanionSlot *slot = FindSlotByName(state, characterName);
    if(slot && IsSlotActive(state, *slot))
        ApplyEquipmentCodes(owner, slot->entity, loadout->rightCode, loadout->leftCode);
    return TRUE;
}

int ResetEquipmentInternal(rsPLAYINFO *owner, const char *characterName)
{
    CharacterEquipmentSnapshot snapshot;
    if(!LoadCharacterSnapshot(owner, characterName, snapshot))
        return -3;

    std::lock_guard<std::mutex> guard(g_mutex);
    OwnerState &state = GetOrCreateState(owner);
    PruneState(state);
    LoadoutOverride *loadout = FindLoadoutOverride(state, characterName, true);
    if(!loadout)
        return -9;

    const LoadoutOverride previous = *loadout;
    loadout->active = false;
    loadout->rightCode = 0;
    loadout->leftCode = 0;
    if(!PersistLoadoutOverride(state, *loadout))
    {
        *loadout = previous;
        return -11;
    }

    CompanionSlot *slot = FindSlotByName(state, characterName);
    if(slot && IsSlotActive(state, *slot))
        ApplyEquipmentCodes(owner, slot->entity, snapshot.defaultRight, snapshot.defaultLeft);
    return TRUE;
}

int ChangeSkillLoadout(rsPLAYINFO *owner, const char *characterName, bool buffSkill, int encodedSlotDirection)
{
    if(!encodedSlotDirection)
        return -10;
    const int slotValue = encodedSlotDirection < 0 ? -encodedSlotDirection : encodedSlotDirection;
    const int slot = slotValue - 1;
    const int direction = encodedSlotDirection >= 0 ? +1 : -1;
    if(slot < 0 || slot >= 4)
        return -10;

    CharacterEquipmentSnapshot snapshot;
    if(!LoadCharacterSnapshot(owner, characterName, snapshot))
        return -3;

    std::lock_guard<std::mutex> guard(g_mutex);
    OwnerState &state = GetOrCreateState(owner);
    PruneState(state);
    LoadoutOverride *loadout = FindLoadoutOverride(state, characterName, true);
    if(!loadout)
        return -9;

    const LoadoutOverride previous = *loadout;
    BYTE *slots = buffSkill ? loadout->buffSkillSlots : loadout->activeSkillSlots;

    // Slots are independent. When cycling, skip a skill already assigned to
    // another slot instead of silently clearing that other slot.
    BYTE candidate = slots[slot];
    for(int attempt = 0; attempt < COMPANION_MAX_SKILL_SLOTS + 1; ++attempt)
    {
        candidate = (BYTE)CycleCompanionSkillUseSlot(snapshot.jobCode, snapshot.skillPoints,
            candidate, direction, buffSkill);
        if(!candidate)
            break;

        bool duplicate = false;
        for(int other = 0; other < 4; ++other)
        {
            if(other != slot && slots[other] == candidate)
            {
                duplicate = true;
                break;
            }
        }
        if(!duplicate)
            break;
    }
    slots[slot] = candidate;

    if(buffSkill)
    {
        loadout->buffLoadoutActive =
            loadout->buffSkillSlots[0] || loadout->buffSkillSlots[1] ||
            loadout->buffSkillSlots[2] || loadout->buffSkillSlots[3];
        if(slots[slot] && slots[slot] != previous.buffSkillSlots[slot])
        {
            CompanionSkillDescriptor descriptor;
            int interval = 60;
            if(GetCompanionSkillDescriptor(snapshot.jobCode, snapshot.skillPoints,
                slots[slot], &descriptor))
            {
                const int duration = GetCompanionSupportDefaultIntervalSeconds(
                    descriptor.skillCode, descriptor.point);
                if(duration > 0)
                    interval = duration;
            }
            if(interval < 5) interval = 5;
            if(interval > 3600) interval = 3600;
            loadout->buffIntervalSeconds[slot] = (WORD)interval;
        }
        else if(slots[slot] && loadout->buffIntervalSeconds[slot] < 5)
        {
            loadout->buffIntervalSeconds[slot] = 60;
        }
        if(!slots[slot])
            loadout->buffIntervalSeconds[slot] = 0;
    }
    else
    {
        if(!slots[slot])
        {
            loadout->activeMinResourcePercent[slot] = 0;
            loadout->activeMinimumTargets[slot] = 1;
            loadout->activeTargetModes[slot] = 0;
        }
        else if(slots[slot] != previous.activeSkillSlots[slot])
        {
            loadout->activeMinResourcePercent[slot] = 0;
            loadout->activeMinimumTargets[slot] = 1;
            loadout->activeTargetModes[slot] = 0;
        }
        loadout->skillLoadoutActive =
            loadout->activeSkillSlots[0] || loadout->activeSkillSlots[1] ||
            loadout->activeSkillSlots[2] || loadout->activeSkillSlots[3];
    }

    if(!PersistLoadoutOverride(state, *loadout))
    {
        *loadout = previous;
        return -11;
    }

    CompanionSlot *activeSlot = FindSlotByName(state, characterName);
    if(buffSkill && activeSlot && IsSlotActive(state, *activeSlot) &&
       previous.buffSkillSlots[slot] != loadout->buffSkillSlots[slot])
    {
        // Editing one buff slot must not tear down unrelated active buffs.
        ResetCompanionBuffModifier(activeSlot->entity, activeSlot->skillRuntime, slot);
    }

    CompanionDebugLog(
        "LOADOUT_EDIT name='%s' kind=%s slot=%d old=%u new=%u slots=[%u,%u,%u,%u] active=%d",
        characterName, buffSkill ? "buff" : "active", slot + 1,
        (unsigned)(buffSkill ? previous.buffSkillSlots[slot] : previous.activeSkillSlots[slot]),
        (unsigned)slots[slot],
        (unsigned)slots[0], (unsigned)slots[1], (unsigned)slots[2], (unsigned)slots[3],
        buffSkill ? (loadout->buffLoadoutActive ? 1 : 0) : (loadout->skillLoadoutActive ? 1 : 0));
    return TRUE;
}

int AdjustBuffInterval(rsPLAYINFO *owner, const char *characterName, int encodedSlotDirection)
{
    if(!encodedSlotDirection)
        return -10;
    const int slotValue = encodedSlotDirection < 0 ? -encodedSlotDirection : encodedSlotDirection;
    const int slot = slotValue - 1;
    const int direction = encodedSlotDirection >= 0 ? +1 : -1;
    if(slot < 0 || slot >= 4)
        return -10;

    std::lock_guard<std::mutex> guard(g_mutex);
    OwnerState &state = GetOrCreateState(owner);
    PruneState(state);
    LoadoutOverride *loadout = FindLoadoutOverride(state, characterName, true);
    if(!loadout)
        return -9;
    if(!loadout->buffSkillSlots[slot])
        return -10;

    const LoadoutOverride previous = *loadout;
    int interval = loadout->buffIntervalSeconds[slot];
    if(interval < 5) interval = 60;
    interval += direction * 15;
    if(interval < 5) interval = 5;
    if(interval > 3600) interval = 3600;
    loadout->buffIntervalSeconds[slot] = (WORD)interval;
    loadout->buffLoadoutActive =
        loadout->buffSkillSlots[0] || loadout->buffSkillSlots[1] ||
        loadout->buffSkillSlots[2] || loadout->buffSkillSlots[3];
    if(!PersistLoadoutOverride(state, *loadout))
    {
        *loadout = previous;
        return -11;
    }

    CompanionSlot *activeSlot = FindSlotByName(state, characterName);
    if(activeSlot && IsSlotActive(state, *activeSlot))
        activeSlot->skillRuntime.nextBuffSkillTime[slot] =
            dwPlayServTime + (DWORD)interval * 1000U;

    CompanionDebugLog(
        "LOADOUT_INTERVAL name='%s' slot=%d old=%u new=%u",
        characterName, slot + 1,
        (unsigned)previous.buffIntervalSeconds[slot],
        (unsigned)loadout->buffIntervalSeconds[slot]);
    return TRUE;
}

int CycleActiveSkillPolicy(rsPLAYINFO *owner, const char *characterName, int slotValue, int policyKind)
{
    const int slot = slotValue - 1;
    if(!owner || !characterName || !characterName[0] || slot < 0 || slot >= 4)
        return -10;
    std::lock_guard<std::mutex> guard(g_mutex);
    OwnerState &state = GetOrCreateState(owner);
    PruneState(state);
    LoadoutOverride *loadout = FindLoadoutOverride(state, characterName, true);
    if(!loadout || !loadout->activeSkillSlots[slot])
        return -10;
    const LoadoutOverride previous = *loadout;
    if(policyKind == 0)
        loadout->activeMinResourcePercent[slot] = (BYTE)((loadout->activeMinResourcePercent[slot] + 10) % 100);
    else if(policyKind == 1)
        loadout->activeMinimumTargets[slot] = (BYTE)((loadout->activeMinimumTargets[slot] % 5) + 1);
    else if(policyKind == 2)
        loadout->activeTargetModes[slot] = (BYTE)((loadout->activeTargetModes[slot] + 1) % 3);
    else
        return -10;
    if(!PersistLoadoutOverride(state, *loadout))
    {
        *loadout = previous;
        return -11;
    }
    CompanionDebugLog("LOADOUT_POLICY name='%s' slot=%d reserve=%u minTargets=%u targetMode=%u",
        characterName, slot + 1, (unsigned)loadout->activeMinResourcePercent[slot],
        (unsigned)loadout->activeMinimumTargets[slot], (unsigned)loadout->activeTargetModes[slot]);
    return TRUE;
}

int ResetSkillLoadout(rsPLAYINFO *owner, const char *characterName, bool buffSkill)
{
    std::lock_guard<std::mutex> guard(g_mutex);
    OwnerState &state = GetOrCreateState(owner);
    PruneState(state);
    LoadoutOverride *loadout = FindLoadoutOverride(state, characterName, true);
    if(!loadout)
        return -9;

    const LoadoutOverride previous = *loadout;
    if(buffSkill)
    {
        loadout->buffLoadoutActive = false;
        ZeroMemory(loadout->buffSkillSlots, sizeof(loadout->buffSkillSlots));
        ZeroMemory(loadout->buffIntervalSeconds, sizeof(loadout->buffIntervalSeconds));
        ZeroMemory(loadout->buffTargetModes, sizeof(loadout->buffTargetModes));
    }
    else
    {
        loadout->skillLoadoutActive = false;
        ZeroMemory(loadout->activeSkillSlots, sizeof(loadout->activeSkillSlots));
        ZeroMemory(loadout->activeMinResourcePercent, sizeof(loadout->activeMinResourcePercent));
        ZeroMemory(loadout->activeTargetModes, sizeof(loadout->activeTargetModes));
        for(int i = 0; i < 4; ++i) loadout->activeMinimumTargets[i] = 1;
    }

    if(!PersistLoadoutOverride(state, *loadout))
    {
        *loadout = previous;
        return -11;
    }

    CompanionSlot *activeSlot = FindSlotByName(state, characterName);
    if(activeSlot && IsSlotActive(state, *activeSlot))
    {
        if(buffSkill)
            ResetAllCompanionBuffModifiers(activeSlot->entity, activeSlot->skillRuntime);
        else
            activeSlot->skillRuntime.activeSkillCursor = 0;
    }
    CompanionDebugLog("LOADOUT_RESET name='%s' kind=%s",
        characterName, buffSkill ? "buff" : "active");
    return TRUE;
}

INT64 ComputeBaseExperience(INT64 rawExp, int monsterLevel, int companionLevel)
{
    int levelDifference = monsterLevel - companionLevel;
    if(levelDifference < 0)
        levelDifference = -levelDifference;

    INT64 result = 0;
    if(levelDifference <= 10)
        result = rawExp;
    else if(levelDifference <= 20)
        result = (rawExp * 90) / 100;
    else if(levelDifference <= 30)
        result = (rawExp * 60) / 100;
    else
        result = (rawExp * 25) / 100;

    if(result < 0)
        result = 0;

    char key[32];
    wsprintfA(key, "Nv[%d]", companionLevel);
    File expIni("Configs//ExpManager.ini");
    const int levelRate = expIni.readInt("ExpConfig", key);
    result *= levelRate;

    return result;
}

void KillEntity(CompanionSlot &slot)
{
    if(slot.entity)
        CompanionDebugLog("KILL_ENTITY_CALL name='%s' serial=%lu flag=%d life=%d motion=%d frame=%d",
            slot.characterName, (unsigned long)slot.entity->dwObjectSerial, slot.entity->Flag,
            slot.entity->smCharInfo.Life[0],
            slot.entity->MotionInfo ? (int)slot.entity->MotionInfo->State : -1, slot.entity->frame);

    if(slot.entity && slot.entity->Flag && slot.entity->smCharInfo.Life[0] > 0)
    {
        slot.entity->smCharInfo.Life[0] = 0;
        const int changed = slot.entity->SetMotionFromCode(CHRMOTION_STATE_DEAD);
        CompanionDebugLog("KILL_ENTITY_DEAD name='%s' serial=%lu setMotion=%d motion=%d frame=%d",
            slot.characterName, (unsigned long)slot.entity->dwObjectSerial, changed,
            slot.entity->MotionInfo ? (int)slot.entity->MotionInfo->State : -1, slot.entity->frame);
    }
    if(slot.entity)
        slot.entity->lpMasterPlayInfo = NULL;
    slot.entity = NULL;
}

} // namespace

int Summon(rsPLAYINFO *owner, const char *characterName)
{
    if(!owner || !owner->lpsmSock || !owner->getAccount() ||
        !owner->getAccount()[0] || !characterName || !characterName[0])
        return FALSE;

    if(owner->getName() && lstrcmpiA(owner->getName(), characterName) == 0)
        return -2;

    TRANS_RECORD_DATA recordData;
    ZeroMemory(&recordData, sizeof(recordData));

    if(!rsLoadAccountCharacterRecord(owner->getAccount(), characterName, &recordData))
        return -3;

    rsPLAYINFO *onlineCharacter = FindUserFromName((char *)characterName);
    if(onlineCharacter && onlineCharacter != owner && onlineCharacter->lpsmSock)
        return -4;

    std::lock_guard<std::mutex> guard(g_mutex);
    OwnerState &state = GetOrCreateState(owner);
    PruneState(state);

    if(IsNameActive(state, characterName))
        return -5;

    const int freeSlot = FindFreeSlot(state);
    if(freeSlot < 0)
        return -6;

    smCHAR *entity = OpenCharacterCompanionFromRecord(owner, &recordData);
    if(!entity)
        return -7;

    CompanionSlot &slot = state.slots[freeSlot];
    strcpy_s(slot.characterName, sizeof(slot.characterName), characterName);
    slot.entity = entity;
    slot.cachedLevel = recordData.smCharInfo.Level;
    slot.changeJob = recordData.smCharInfo.ChangeJob;
    slot.cachedExp = GetExp64(&recordData.smCharInfo);
    slot.pendingExp = 0;
    slot.lastExpFlushTime = dwPlayServTime;
    slot.deadSince = 0;
    slot.lastProgressCheckTime = 0;
    slot.lastProgressX = 0;
    slot.lastProgressZ = 0;
    slot.stuckTicks = 0;
    slot.nextIdentityCheckTime = 0;
    slot.lootArea = NULL;
    slot.lootTarget = NULL;
    slot.lootTargetSince = 0;
    slot.nextLootScanTime = 0;
    slot.nextLootPickupTime = 0;
    slot.visualMotionState = 0;
    slot.visualMotionSequence = 0;
    slot.visualMotionFrame = 0;
    slot.skillRuntime.Reset();
    BYTE logicalSkillPoints[COMPANION_MAX_SKILL_SLOTS] = {};
    ReadCompanionLogicalSkillPoints(recordData, logicalSkillPoints, COMPANION_MAX_SKILL_SLOTS);
    LoadCompanionLogicalSkillPoints(slot.skillRuntime, logicalSkillPoints, COMPANION_MAX_SKILL_SLOTS);
    ApplyCompanionPassiveSkills(entity, slot.skillRuntime);
    if(CompanionDebugEnabled())
    {
        CompanionDebugLog(
            "SKILL_LOAD name='%s' serial=%lu job=%lu level=%d mana=%d/%d stamina=%d/%d points=[%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u]",
            slot.characterName, (unsigned long)entity->dwObjectSerial,
            (unsigned long)entity->smCharInfo.JOB_CODE, entity->smCharInfo.Level,
            entity->smCharInfo.Mana[0], entity->smCharInfo.Mana[1],
            entity->smCharInfo.Stamina[0], entity->smCharInfo.Stamina[1],
            (unsigned)slot.skillRuntime.skillPoints[0],
            (unsigned)slot.skillRuntime.skillPoints[1],
            (unsigned)slot.skillRuntime.skillPoints[2],
            (unsigned)slot.skillRuntime.skillPoints[3],
            (unsigned)slot.skillRuntime.skillPoints[4],
            (unsigned)slot.skillRuntime.skillPoints[5],
            (unsigned)slot.skillRuntime.skillPoints[6],
            (unsigned)slot.skillRuntime.skillPoints[7],
            (unsigned)slot.skillRuntime.skillPoints[8],
            (unsigned)slot.skillRuntime.skillPoints[9],
            (unsigned)slot.skillRuntime.skillPoints[10],
            (unsigned)slot.skillRuntime.skillPoints[11],
            (unsigned)slot.skillRuntime.skillPoints[12],
            (unsigned)slot.skillRuntime.skillPoints[13],
            (unsigned)slot.skillRuntime.skillPoints[14],
            (unsigned)slot.skillRuntime.skillPoints[15]);
    }

    // Session-only companion loadouts survive dismiss/resummon but never alter
    // the offline character record. Reapply the override after the DAT-backed
    // spawn has initialized its canonical appearance/equipment.
    LoadoutOverride *loadout = FindLoadoutOverride(state, characterName, true);
    if(loadout)
    {
        if(loadout->formationSlot < 0 || loadout->formationSlot >= COMPANION_MAX_ACTIVE)
        {
            const int previousFormationSlot = loadout->formationSlot;
            loadout->formationSlot = freeSlot;
            if(!PersistLoadoutOverride(state, *loadout))
                loadout->formationSlot = previousFormationSlot;
        }
        if(loadout->active)
            ApplyEquipmentCodes(owner, entity, loadout->rightCode, loadout->leftCode);
        else
            SendFullCompanionInfo(owner, entity);
    }
    else
    {
        SendFullCompanionInfo(owner, entity);
    }

    // OpenCharacterCompanionFromRecord bootstraps visibility before this slot
    // exists. The post-slot full snapshot above intentionally repeats PLAYERINFO
    // once so the client gets a stable body/head snapshot after the remote smCHAR
    // has been allocated; this also gives a failed secondary-head load a retry.
    SendRuntimeStateLocked(state, true);
    SendVisualStateLocked(state, true);
    return TRUE;
}

int Select(rsPLAYINFO *owner, const char *characterName)
{
    if(!owner || !characterName || !characterName[0])
        return FALSE;

    if(owner->getName() && lstrcmpiA(owner->getName(), characterName) == 0)
        return -2;

    if(!rsCheckAccountChar(owner->getAccount(), (char *)characterName))
        return -3;

    std::lock_guard<std::mutex> guard(g_mutex);
    OwnerState &state = GetOrCreateState(owner);
    strcpy_s(state.selectedName, sizeof(state.selectedName), characterName);
    return TRUE;
}

int SummonSelected(rsPLAYINFO *owner)
{
    if(!owner)
        return FALSE;

    char selected[32] = { 0 };
    {
        std::lock_guard<std::mutex> guard(g_mutex);
        OwnerState &state = GetOrCreateState(owner);
        PruneState(state);
        if(state.selectedName[0])
            strcpy_s(selected, sizeof(selected), state.selectedName);
    }

    if(selected[0])
        return Summon(owner, selected);

    char names[sPLAY_CHAR_MAX][32];
    ZeroMemory(names, sizeof(names));
    const int count = rsGetAccountCharacterNames(owner->getAccount(), names, sPLAY_CHAR_MAX);
    for(int i = 0; i < count; ++i)
    {
        if(!names[i][0])
            continue;
        if(owner->getName() && lstrcmpiA(owner->getName(), names[i]) == 0)
            continue;

        const int result = Summon(owner, names[i]);
        if(result == TRUE)
        {
            Select(owner, names[i]);
            return TRUE;
        }
        if(result != -5)
            return result;
    }

    return -8;
}

int Dismiss(rsPLAYINFO *owner, const char *characterName)
{
    if(!owner || !characterName || !characterName[0])
        return FALSE;

    std::lock_guard<std::mutex> guard(g_mutex);
    OwnerState *state = FindState(owner);
    if(!state)
        return -1;

    for(int i = 0; i < COMPANION_MAX_ACTIVE; ++i)
    {
        CompanionSlot &slot = state->slots[i];
        if(slot.characterName[0] && lstrcmpiA(slot.characterName, characterName) == 0)
        {
            const bool flushed = FlushSlotExperienceRetry(*state, slot, 3);
            KillEntity(slot);
            if(flushed)
                ClearSlot(slot);
            SendRuntimeStateLocked(*state, true);
            SendVisualStateLocked(*state, true);
            // Keep the slot reserved with pending EXP if persistence failed.
            // A later state touch/dismiss can safely retry instead of losing it.
            return flushed ? TRUE : -3;
        }
    }
    return -2;
}

int DismissAll(rsPLAYINFO *owner)
{
    if(!owner)
        return FALSE;

    std::lock_guard<std::mutex> guard(g_mutex);
    OwnerState *state = FindState(owner);
    if(!state)
        return TRUE;

    bool allFlushed = true;
    for(int i = 0; i < COMPANION_MAX_ACTIVE; ++i)
    {
        CompanionSlot &slot = state->slots[i];
        const bool flushed = FlushSlotExperienceRetry(*state, slot, 3);
        KillEntity(slot);
        if(flushed)
            ClearSlot(slot);
        else
            allFlushed = false;
    }

    SendRuntimeStateLocked(*state, true);
    SendVisualStateLocked(*state, true);
    return allFlushed ? TRUE : -3;
}

void OnOwnerDisconnect(rsPLAYINFO *owner)
{
    if(!owner)
        return;

    std::lock_guard<std::mutex> guard(g_mutex);
    std::map<rsPLAYINFO *, OwnerState>::iterator it = g_states.find(owner);
    if(it == g_states.end())
        return;

    for(int i = 0; i < COMPANION_MAX_ACTIVE; ++i)
    {
        CompanionSlot &slot = it->second.slots[i];
        // Match the legacy save path's retry-minded behaviour on disconnect.
        FlushSlotExperienceRetry(it->second, slot, 6);
        KillEntity(slot);
        ClearSlot(slot);
    }

    g_states.erase(it);
}

int GetActiveCount(rsPLAYINFO *owner)
{
    if(!owner)
        return 0;

    std::lock_guard<std::mutex> guard(g_mutex);
    OwnerState *state = FindState(owner);
    if(!state)
        return 0;

    int count = 0;
    for(int i = 0; i < COMPANION_MAX_ACTIVE; ++i)
    {
        if(IsSlotActive(*state, state->slots[i]))
            ++count;
    }
    return count;
}

int GetPartyActiveCount(rsPLAYINFO *partyMaster, int area)
{
    if(!partyMaster)
        return 0;

    int count = 0;
    for(int i = 0; i < PARTY_PLAYER_MAX; ++i)
    {
        rsPLAYINFO *member = partyMaster->lpPartyPlayers[i];
        if(member && member->lpsmSock && member->Position.Area == area)
            count += GetActiveCount(member);
    }
    return count;
}

bool IsManagedCompanion(smCHAR *character)
{
    if(!character)
        return false;

    std::lock_guard<std::mutex> guard(g_mutex);
    for(std::map<rsPLAYINFO *, OwnerState>::iterator it = g_states.begin();
        it != g_states.end(); ++it)
    {
        OwnerState &state = it->second;
        if(!IsOwnerValid(state))
            continue;
        for(int i = 0; i < COMPANION_MAX_ACTIVE; ++i)
        {
            if(IsSlotActive(state, state.slots[i]) && state.slots[i].entity == character)
                return true;
        }
    }
    return false;
}

int GetPassiveEvadePercent(smCHAR *character)
{
    if(!character || !character->lpMasterPlayInfo)
        return 0;

    std::lock_guard<std::mutex> guard(g_mutex);
    for(std::map<rsPLAYINFO *, OwnerState>::iterator it = g_states.begin(); it != g_states.end(); ++it)
    {
        OwnerState &state = it->second;
        if(!IsOwnerValid(state))
            continue;
        for(int i = 0; i < COMPANION_MAX_ACTIVE; ++i)
        {
            CompanionSlot &slot = state.slots[i];
            if(IsSlotActive(state, slot) && slot.entity == character)
            {
                int evade = (int)slot.skillRuntime.passiveEvadePercent;
                for(int b = 0; b < 4; ++b)
                    evade += slot.skillRuntime.buffModifiers[b].evadePercent;
                return std::max(0, std::min(95, evade));
            }
        }
    }
    return 0;
}


int GetReactiveDamage(smCHAR *character, int receivedDamage)
{
    if(!character || !character->lpMasterPlayInfo || receivedDamage <= 0)
        return 0;

    std::lock_guard<std::mutex> guard(g_mutex);
    for(std::map<rsPLAYINFO *, OwnerState>::iterator it = g_states.begin(); it != g_states.end(); ++it)
    {
        OwnerState &state = it->second;
        if(!IsOwnerValid(state))
            continue;
        for(int i = 0; i < COMPANION_MAX_ACTIVE; ++i)
        {
            CompanionSlot &slot = state.slots[i];
            if(!IsSlotActive(state, slot) || slot.entity != character)
                continue;
            int reflected = 0;
            for(int b = 0; b < 4; ++b)
            {
                const CompanionBuffModifier &modifier = slot.skillRuntime.buffModifiers[b];
                reflected += modifier.reactiveDamageFlat;
                if(modifier.reactiveDamagePercent > 0)
                    reflected += (receivedDamage * modifier.reactiveDamagePercent) / 100;
            }
            return std::max(0, reflected);
        }
    }
    return 0;
}

bool CanMonsterTargetCompanion(smCHAR *monster, smCHAR *target)
{
    if(!monster || !target)
        return true;

    std::lock_guard<std::mutex> guard(g_mutex);
    for(std::map<rsPLAYINFO *, OwnerState>::iterator it = g_states.begin();
        it != g_states.end(); ++it)
    {
        OwnerState &state = it->second;
        if(!IsOwnerValid(state))
            continue;
        for(int i = 0; i < COMPANION_MAX_ACTIVE; ++i)
        {
            CompanionSlot &slot = state.slots[i];
            if(IsSlotActive(state, slot) && slot.entity == target)
                return CanMonsterSeeCompanionWithVanish(target, monster, slot.skillRuntime, dwPlayServTime);
        }
    }
    return true;
}

void ApplyIncomingDamage(smCHAR *target, smCHAR *attacker, int *damage)
{
    if(!target || !damage || *damage <= 0)
        return;

    std::lock_guard<std::mutex> guard(g_mutex);
    for(std::map<rsPLAYINFO *, OwnerState>::iterator it = g_states.begin();
        it != g_states.end(); ++it)
    {
        OwnerState &state = it->second;
        if(!IsOwnerValid(state))
            continue;
        for(int i = 0; i < COMPANION_MAX_ACTIVE; ++i)
        {
            CompanionSlot &slot = state.slots[i];
            if(IsSlotActive(state, slot) && slot.entity == target)
            {
                ApplyCompanionIncomingDamageModifiers(target, attacker, state.owner,
                    slot.skillRuntime, damage, dwPlayServTime);
                return;
            }
        }
    }
}

void ApplyOutgoingBasicDamage(smCHAR *attacker, smCHAR *target, int *damage)
{
    if(!attacker || !target || !damage || *damage <= 0)
        return;

    std::lock_guard<std::mutex> guard(g_mutex);
    for(std::map<rsPLAYINFO *, OwnerState>::iterator it = g_states.begin();
        it != g_states.end(); ++it)
    {
        OwnerState &state = it->second;
        if(!IsOwnerValid(state))
            continue;
        for(int i = 0; i < COMPANION_MAX_ACTIVE; ++i)
        {
            CompanionSlot &slot = state.slots[i];
            if(IsSlotActive(state, slot) && slot.entity == attacker)
            {
                const DWORD rightItemCode = attacker->smCharInfo.dwTemp[smCOMPANION_NET_TEMP_RIGHT_ITEM_CODE];
                ApplyCompanionOutgoingBasicDamageModifiers(attacker, target, rightItemCode,
                    slot.skillRuntime, damage, dwPlayServTime);
                return;
            }
        }
    }
}

void ApplyOutgoingBasicPostAbsorption(smCHAR *attacker, smCHAR *target, int *damage)
{
    if(!attacker || !target || !damage || *damage <= 0)
        return;

    std::lock_guard<std::mutex> guard(g_mutex);
    for(std::map<rsPLAYINFO *, OwnerState>::iterator it = g_states.begin();
        it != g_states.end(); ++it)
    {
        OwnerState &state = it->second;
        if(!IsOwnerValid(state))
            continue;
        for(int i = 0; i < COMPANION_MAX_ACTIVE; ++i)
        {
            CompanionSlot &slot = state.slots[i];
            if(IsSlotActive(state, slot) && slot.entity == attacker)
            {
                ApplyCompanionOutgoingBasicPostAbsorptionModifiers(attacker, target,
                    slot.skillRuntime, damage, dwPlayServTime);
                return;
            }
        }
    }
}

void PublishVisualState(smCHAR *character)
{
    if(!character || !character->lpMasterPlayInfo)
        return;

    std::lock_guard<std::mutex> guard(g_mutex);
    OwnerState *state = FindState(character->lpMasterPlayInfo);
    if(!state)
        return;

    for(int i = 0; i < COMPANION_MAX_ACTIVE; ++i)
    {
        CompanionSlot &slot = state->slots[i];
        if(slot.characterName[0] && slot.entity == character && slot.entity->Flag &&
            slot.entity->lpMasterPlayInfo == state->owner)
        {
            UpdateVisualMotionSequence(slot);
            break;
        }
    }

    SendVisualStateLocked(*state, false);
}

void ApplyAI(smCHAR *character)
{
    if(!character || !character->lpMasterPlayInfo)
        return;

    std::lock_guard<std::mutex> guard(g_mutex);
    OwnerState *state = FindState(character->lpMasterPlayInfo);
    if(!state)
        return;

    CompanionSlot *managedSlot = NULL;
    for(int i = 0; i < COMPANION_MAX_ACTIVE; ++i)
    {
        if(IsSlotActive(*state, state->slots[i]) && state->slots[i].entity == character)
        {
            managedSlot = &state->slots[i];
            break;
        }
    }
    if(!managedSlot)
        return;

    rsPLAYINFO *owner = state->owner;

    // The summon path already rejects an alt that is online as a real player.
    // Re-check periodically so a later login cannot leave the same character
    // alive both as rsPLAYINFO and as an account companion.  Retire the
    // companion before running any more AI/combat and preserve pending EXP if
    // the persistence flush transiently fails.
    if(!managedSlot->nextIdentityCheckTime ||
       (int)(dwPlayServTime - managedSlot->nextIdentityCheckTime) >= 0)
    {
        managedSlot->nextIdentityCheckTime = dwPlayServTime + 2000U;
        rsPLAYINFO *onlineCharacter = FindUserFromName(managedSlot->characterName);
        if(onlineCharacter && onlineCharacter != owner && onlineCharacter->lpsmSock)
        {
            char retiringName[32] = { 0 };
            CopyText32(retiringName, managedSlot->characterName);
            const DWORD retiringSerial = character->dwObjectSerial;
            const bool flushed = FlushSlotExperience(*state, *managedSlot);
            KillEntity(*managedSlot);
            if(flushed)
                ClearSlot(*managedSlot);
            CompanionDebugLog(
                "IDENTITY_CONFLICT_DISMISS name='%s' serial=%lu onlineSerial=%lu expFlushed=%d",
                retiringName, (unsigned long)retiringSerial,
                (unsigned long)onlineCharacter->dwObjectSerial, flushed ? 1 : 0);
            SendRuntimeStateLocked(*state, true);
            SendVisualStateLocked(*state, true);
            return;
        }
    }

    LoadoutOverride *profile = FindLoadoutOverride(*state, managedSlot->characterName, true);
    const CompanionMode mode = profile ? profile->mode : state->mode;
    const CompanionTacticPreset tactic = profile ? profile->tactic : COMPANION_TACTIC_BALANCED;
    const int retreatHpPercent = profile ? profile->retreatHpPercent : 0;
    const int preferredRange = profile ? profile->preferredRange : 0;

    // Damage code rejects targets whose authoritative motion is DEAD. A living
    // account companion must never remain in that state after a stale transition
    // or it becomes effectively untargetable even while HP is still positive.
    if(character->smCharInfo.Life[0] > 0 && character->MotionInfo &&
        character->MotionInfo->State == CHRMOTION_STATE_DEAD)
    {
        if(CompanionDebugEnabled())
            CompanionDebugLog("ALIVE_DEAD_MOTION_RECOVER name='%s' serial=%lu life=%d/%d frame=%d",
                managedSlot->characterName, (unsigned long)character->dwObjectSerial,
                character->smCharInfo.Life[0], character->smCharInfo.Life[1], character->frame);
        character->SetMotionFromCode(CHRMOTION_STATE_STAND);
    }

    // Compact 1 Hz HUD telemetry is built only from already-live companion
    // state; it never reloads the character DAT or item catalogue.
    SendRuntimeStateLocked(*state, false);

    // Owner-dead policy: companions hold/follow the owner's corpse but do not
    // keep fighting or consuming resources while their controlling character
    // is dead.  Cancel delayed offensive events so they cannot land after a
    // later owner revive; persistent timed buffs/familiars keep their absolute
    // expiry and simply resume if still valid.
    if(owner->smCharInfo.Life[0] <= 0)
    {
        managedSlot->skillRuntime.multiHitSkillCode = 0;
        managedSlot->skillRuntime.multiHitTargetSerial = 0;
        managedSlot->skillRuntime.multiHitRemaining = 0;
        managedSlot->skillRuntime.multiHitNextTime = 0;
        managedSlot->skillRuntime.chargedStrikeTargetSerial = 0;
        managedSlot->skillRuntime.chargedStrikeExecuteAt = 0;
        character->lpTargetChar = NULL;
        character->lpTargetPlayInfo = NULL;
        character->TargetMoveCount = 0;
        character->AutoMoveStep = 0;
        if(character->MotionInfo &&
           (character->MotionInfo->State == CHRMOTION_STATE_ATTACK ||
            character->MotionInfo->State == CHRMOTION_STATE_SKILL))
            character->SetMotionFromCode(CHRMOTION_STATE_STAND);
        return;
    }

    const DWORD rightItemCode = character->smCharInfo.dwTemp[smCOMPANION_NET_TEMP_RIGHT_ITEM_CODE];
    const DWORD leftItemCode = character->smCharInfo.dwTemp[smCOMPANION_NET_TEMP_LEFT_ITEM_CODE];

    // Match the legacy player resource loop before emergency consumables.
    // This uses the character's authoritative Life/Mana/Stamina_Regen stats
    // and fractional accumulators so low per-second values are not lost.
    if(UpdateCompanionResourceRegeneration(character, managedSlot->skillRuntime, dwPlayServTime) &&
       CompanionDebugEnabled() &&
       CompanionDebugTick(character->dwObjectSerial + 59U, 5000U))
    {
        CompanionDebugLog(
            "RESOURCE_REGEN name='%s' serial=%lu life=%d/%d mana=%d/%d stamina=%d/%d",
            managedSlot->characterName, (unsigned long)character->dwObjectSerial,
            character->smCharInfo.Life[0], character->smCharInfo.Life[1],
            character->smCharInfo.Mana[0], character->smCharInfo.Mana[1],
            character->smCharInfo.Stamina[0], character->smCharInfo.Stamina[1]);
    }

    if(CompanionDebugEnabled() && CompanionDebugTick(character->dwObjectSerial + 67U, 5000U))
        LogCompanionPotionState(managedSlot->characterName, character, managedSlot->skillRuntime, dwPlayServTime);

    // Consumables belong to the offline alt itself. The persistence helper
    // validates the restore payload first, then decrements and persists the DAT
    // stack before applying HP/MP/Stamina so a failed write cannot duplicate it.
    if(owner->getAccount() && owner->getAccount()[0])
    {
        if(TryUseCompanionConsumable(owner->getAccount(), managedSlot->characterName,
            character, managedSlot->skillRuntime, dwPlayServTime))
        {
            CompanionDebugLog("ITEM_USE name='%s' serial=%lu life=%d/%d mana=%d/%d stamina=%d/%d",
                managedSlot->characterName, (unsigned long)character->dwObjectSerial,
                character->smCharInfo.Life[0], character->smCharInfo.Life[1],
                character->smCharInfo.Mana[0], character->smCharInfo.Mana[1],
                character->smCharInfo.Stamina[0], character->smCharInfo.Stamina[1]);
            SendRuntimeStateLocked(*state, true);
        }
    }

    // Buff modifiers are server-authoritative stats. Expire them even while the
    // companion is Passive or temporarily skill-locked.
    UpdateCompanionBuffModifiers(character, managedSlot->skillRuntime, dwPlayServTime);

    // V9: scheduled hits belong to the original cast and must advance even
    // while the short action lock suppresses the basic-attack state machine.
    if(UpdateCompanionMultiHit(character, owner, managedSlot->skillRuntime, dwPlayServTime))
    {
        SendRuntimeStateLocked(*state, true);
        if(CompanionDebugEnabled())
            CompanionDebugLog("SKILL_HIT name='%s' serial=%lu code=0x%08lX remaining=%d target=%lu",
                managedSlot->characterName, (unsigned long)character->dwObjectSerial,
                (unsigned long)managedSlot->skillRuntime.multiHitSkillCode,
                managedSlot->skillRuntime.multiHitRemaining,
                (unsigned long)managedSlot->skillRuntime.multiHitTargetSerial);
    }

    if(UpdateCompanionChargedStrike(character, owner, managedSlot->skillRuntime, dwPlayServTime))
    {
        SendRuntimeStateLocked(*state, true);
        if(CompanionDebugEnabled())
            CompanionDebugLog("SKILL_CHARGED_HIT name='%s' serial=%lu target=%lu",
                managedSlot->characterName, (unsigned long)character->dwObjectSerial,
                character->lpTargetChar ? (unsigned long)character->lpTargetChar->dwObjectSerial : 0UL);
    }

    if(UpdateCompanionPersistentEffect(character, owner, managedSlot->skillRuntime, dwPlayServTime))
    {
        SendRuntimeStateLocked(*state, true);
        if(CompanionDebugEnabled())
            CompanionDebugLog("SKILL_FAMILIAR_HIT name='%s' serial=%lu code=0x%08lX target=%lu",
                managedSlot->characterName, (unsigned long)character->dwObjectSerial,
                (unsigned long)managedSlot->skillRuntime.persistentEffectSkillCode,
                character->lpTargetChar ? (unsigned long)character->lpTargetChar->dwObjectSerial : 0UL);
    }

    // A server-authoritative cast owns this short window. Do not let the
    // monster/basic-attack state machine add an extra hit at FrameCounter 40.
    if(IsCompanionSkillLocked(managedSlot->skillRuntime, dwPlayServTime))
    {
        character->lpTargetChar = NULL;
        character->lpTargetPlayInfo = NULL;
        character->TargetMoveCount = 0;
        character->AutoMoveStep = 0;
        return;
    }

    // Follow may still provide support, but Passive never consumes MP/casts.
    if(mode != COMPANION_MODE_PASSIVE)
    {
        smCHAR *party[COMPANION_MAX_ACTIVE] = {};
        CompanionSkillRuntime *partyRuntimes[COMPANION_MAX_ACTIVE] = {};
        int partyCount = 0;
        for(int i = 0; i < COMPANION_MAX_ACTIVE; ++i)
        {
            CompanionSlot &partySlot = state->slots[i];
            // Keep owned corpses in the support view so Resurrection can select
            // them during the configured corpse window. Other support handlers
            // already reject Life <= 0 recipients explicitly.
            if(partySlot.entity && partySlot.entity->Flag &&
               partySlot.entity->lpMasterPlayInfo == state->owner)
            {
                party[partyCount] = partySlot.entity;
                partyRuntimes[partyCount] = &partySlot.skillRuntime;
                ++partyCount;
            }
        }

        UpdateCompanionRegenerationFieldAura(character, party, partyRuntimes, partyCount,
            managedSlot->skillRuntime, dwPlayServTime);

        CompanionSkillAttempt buffAttempt;
        if(profile && TryExecuteCompanionSupportLoadout(character, owner, party, partyRuntimes, partyCount,
            rightItemCode, leftItemCode, profile->buffSkillSlots, profile->buffIntervalSeconds,
            profile->buffLoadoutActive, managedSlot->skillRuntime, dwPlayServTime, &buffAttempt))
        {
            CompanionDebugLog("SKILL_CAST name='%s' serial=%lu code=0x%08lX kind=support point=%d mana=%d/%d stamina=%d/%d",
                managedSlot->characterName, (unsigned long)character->dwObjectSerial,
                (unsigned long)managedSlot->skillRuntime.lastSkillCode, buffAttempt.point,
                character->smCharInfo.Mana[0], character->smCharInfo.Mana[1],
                character->smCharInfo.Stamina[0], character->smCharInfo.Stamina[1]);
            SendRuntimeStateLocked(*state, true);
            SendVisualStateLocked(*state, true);
            return;
        }
        else if(profile && profile->buffLoadoutActive &&
                buffAttempt.reason != COMPANION_SKILL_INVALID &&
                buffAttempt.reason != COMPANION_SKILL_COOLDOWN &&
                buffAttempt.reason != COMPANION_SKILL_LOCKED &&
                buffAttempt.reason != COMPANION_SKILL_NOT_NEEDED &&
                CompanionDebugEnabled() &&
                ShouldLogCompanionSkillReject(managedSlot->skillRuntime, 0, buffAttempt, dwPlayServTime, 5000U))
        {
            CompanionDebugLog(
                "SKILL_CHECK name='%s' serial=%lu kind=support code=0x%08lX point=%d reason=%s level=%d reqLevel=%d RH=0x%08lX LH=0x%08lX mana=%d/%d reqMana=%d reserveMana=%d stamina=%d/%d reqStamina=%d reserveStamina=%d",
                managedSlot->characterName, (unsigned long)character->dwObjectSerial,
                (unsigned long)buffAttempt.skillCode, buffAttempt.point,
                CompanionSkillRejectReasonName(buffAttempt.reason),
                character->smCharInfo.Level, buffAttempt.requiredLevel,
                (unsigned long)rightItemCode, (unsigned long)leftItemCode,
                character->smCharInfo.Mana[0], character->smCharInfo.Mana[1], buffAttempt.manaCost, buffAttempt.manaReserve,
                character->smCharInfo.Stamina[0], character->smCharInfo.Stamina[1], buffAttempt.staminaCost, buffAttempt.staminaReserve);
        }

        CompanionSkillAttempt healAttempt;
        if(TryExecuteCompanionHeal(character, owner, party, partyCount,
            rightItemCode, leftItemCode, managedSlot->skillRuntime, dwPlayServTime, &healAttempt))
        {
            CompanionDebugLog("SKILL_CAST name='%s' serial=%lu code=0x%08lX kind=heal point=%d mana=%d/%d stamina=%d/%d",
                managedSlot->characterName, (unsigned long)character->dwObjectSerial,
                (unsigned long)managedSlot->skillRuntime.lastSkillCode, healAttempt.point,
                character->smCharInfo.Mana[0], character->smCharInfo.Mana[1],
                character->smCharInfo.Stamina[0], character->smCharInfo.Stamina[1]);
            SendRuntimeStateLocked(*state, true);
            SendVisualStateLocked(*state, true);
            return;
        }
        else if(character->smCharInfo.JOB_CODE == JOBCODE_PRIESTESS &&
                healAttempt.reason != COMPANION_SKILL_NO_TARGET &&
                healAttempt.reason != COMPANION_SKILL_COOLDOWN &&
                healAttempt.reason != COMPANION_SKILL_LOCKED &&
                CompanionDebugEnabled() &&
                ShouldLogCompanionSkillReject(managedSlot->skillRuntime, 1, healAttempt, dwPlayServTime, 5000U))
        {
            CompanionDebugLog(
                "SKILL_CHECK name='%s' serial=%lu kind=heal code=0x%08lX point=%d reason=%s level=%d reqLevel=%d mana=%d/%d reqMana=%d reserveMana=%d stamina=%d/%d reqStamina=%d reserveStamina=%d",
                managedSlot->characterName, (unsigned long)character->dwObjectSerial,
                (unsigned long)healAttempt.skillCode, healAttempt.point,
                CompanionSkillRejectReasonName(healAttempt.reason),
                character->smCharInfo.Level, healAttempt.requiredLevel,
                character->smCharInfo.Mana[0], character->smCharInfo.Mana[1], healAttempt.manaCost, healAttempt.manaReserve,
                character->smCharInfo.Stamina[0], character->smCharInfo.Stamina[1], healAttempt.staminaCost, healAttempt.staminaReserve);
        }

        // Healing already chooses the lowest-health owner/companion inside
        // 320 units. If Healing is learned but every injured recipient is just
        // outside that range, spend this AI tick closing the distance instead
        // of immediately returning to offensive target selection. Resource or
        // cooldown rejects do not trigger repositioning.
        if(character->smCharInfo.JOB_CODE == JOBCODE_PRIESTESS &&
           GetCompanionSkillPoint(managedSlot->skillRuntime, 1) > 0 &&
           healAttempt.reason == COMPANION_SKILL_NO_TARGET)
        {
            DWORD healMoveTarget = 0;
            if(MoveTowardInjuredAlly(character, owner, party, partyCount, &healMoveTarget))
            {
                if(CompanionDebugEnabled() && CompanionDebugTick(character->dwObjectSerial + 83U, 1000U))
                    CompanionDebugLog("AI_HEAL_POSITION name='%s' serial=%lu target=%lu",
                        managedSlot->characterName, (unsigned long)character->dwObjectSerial,
                        (unsigned long)healMoveTarget);
                return;
            }
        }
    }

    // Retreat is explicit/opt-in only. A zero threshold disables it entirely. A permanent low-HP
    // early-return made companions stop fighting forever after crossing the
    // configured threshold. Keep retreat meaningful, but make it a one-shot
    // short disengage until HP recovers above a small hysteresis window.
    if(retreatHpPercent > 0 && character->smCharInfo.Life[1] > 0)
    {
        const int lifePercent = (character->smCharInfo.Life[0] * 100) / character->smCharInfo.Life[1];
        if(lifePercent > retreatHpPercent + 10)
        {
            managedSlot->retreatLatched = false;
            managedSlot->retreatUntil = 0;
        }
        else if(lifePercent <= retreatHpPercent && !managedSlot->retreatLatched)
        {
            managedSlot->retreatLatched = true;
            managedSlot->retreatUntil = dwPlayServTime + 1500U;
        }

        if(managedSlot->retreatLatched &&
            static_cast<int>(managedSlot->retreatUntil - dwPlayServTime) > 0)
        {
            DisengageTarget(character);
            int offsetX = 0;
            int offsetZ = 0;
            FormationOffset(profile ? profile->formationSlot : 0, offsetX, offsetZ);
            character->SetTargetPosi2(
                (owner->Position.x + offsetX) >> FLOATNS,
                (owner->Position.z + offsetZ) >> FLOATNS);
            character->TargetMoveCount = 2;
            character->TragetTraceMode = FALSE;
            if(character->MotionInfo &&
                character->MotionInfo->State != CHRMOTION_STATE_DEAD &&
                character->MotionInfo->State != CHRMOTION_STATE_DAMAGE)
            {
                character->SetMotionFromCode(CHRMOTION_STATE_RUN);
                if(!character->MotionInfo || character->MotionInfo->State != CHRMOTION_STATE_RUN)
                    character->SetMotionFromCode(CHRMOTION_STATE_WALK);
            }
            return;
        }
    }

    // V43 Loot runs only after emergency/support/retreat decisions. Aggressive
    // companions additionally refuse loot while a hostile is visible, so pickup
    // cannot pre-empt normal combat target selection.
    if(TryHandleCompanionLoot(character, owner, *managedSlot, mode))
        return;

    if(mode == COMPANION_MODE_PASSIVE || mode == COMPANION_MODE_FOLLOW)
    {
        if(CompanionDebugTick(character->dwObjectSerial, 1000U))
            CompanionDebugLog("AI name='%s' serial=%lu mode=%d tactic=%d decision=MODE_DISENGAGE life=%d/%d motion=%d target=%lu",
                managedSlot->characterName, (unsigned long)character->dwObjectSerial,
                (int)mode, (int)tactic, character->smCharInfo.Life[0], character->smCharInfo.Life[1],
                character->MotionInfo ? (int)character->MotionInfo->State : -1,
                character->lpTargetChar ? (unsigned long)character->lpTargetChar->dwObjectSerial : 0UL);
        DisengageTarget(character);
        return;
    }

    smCHAR *ownerTarget = IsValidHostile(owner->lpChrTarget) ? owner->lpChrTarget : NULL;
    smCHAR *currentTarget = IsValidHostile(character->lpTargetChar) ? character->lpTargetChar : NULL;
    smCHAR *target = NULL;

    int matchedCustomRule = -1;
    const int customAction = profile ?
        EvaluateCustomTactic(profile->customTactics, character, owner, *state, ownerTarget, &matchedCustomRule) : -1;

    if(customAction == protocol::COMPANION_TACTIC_ACTION_RETREAT)
    {
        DisengageTarget(character);
        int offsetX = 0;
        int offsetZ = 0;
        FormationOffset(profile ? profile->formationSlot : 0, offsetX, offsetZ);
        character->SetTargetPosi2(
            (owner->Position.x + offsetX) >> FLOATNS,
            (owner->Position.z + offsetZ) >> FLOATNS);
        character->TargetMoveCount = 2;
        character->TragetTraceMode = FALSE;
        if(character->MotionInfo &&
           character->MotionInfo->State != CHRMOTION_STATE_DEAD &&
           character->MotionInfo->State != CHRMOTION_STATE_DAMAGE)
        {
            character->SetMotionFromCode(CHRMOTION_STATE_RUN);
            if(!character->MotionInfo || character->MotionInfo->State != CHRMOTION_STATE_RUN)
                character->SetMotionFromCode(CHRMOTION_STATE_WALK);
        }
        if(CompanionDebugEnabled() && CompanionDebugTick(character->dwObjectSerial + 101U, 1000U))
            CompanionDebugLog("TACTIC_RULE name='%s' rule=%d action=RETREAT", managedSlot->characterName, matchedCustomRule);
        return;
    }

    if(customAction >= 0)
    {
        switch(customAction)
        {
            case protocol::COMPANION_TACTIC_ACTION_ATTACK_OWNER_TARGET:
                target = ownerTarget ? ownerTarget : (currentTarget ? currentTarget :
                    FindBestHostile(character, owner, *state, currentTarget, ownerTarget));
                break;

            case protocol::COMPANION_TACTIC_ACTION_DEFEND_PARTY:
                target = FindNearestThreatToParty(character, owner, *state);
                if(!target && ownerTarget) target = ownerTarget;
                if(!target && currentTarget) target = currentTarget;
                if(!target) target = FindBestHostile(character, owner, *state, currentTarget, ownerTarget);
                break;

            case protocol::COMPANION_TACTIC_ACTION_ATTACK_NEAREST:
                target = FindNearestHostile(character);
                break;

            case protocol::COMPANION_TACTIC_ACTION_HOLD_POSITION:
            {
                smCHAR *candidate = currentTarget ? currentTarget : ownerTarget;
                if(!candidate)
                    candidate = FindNearestHostile(character);
                if(candidate)
                {
                    int offsetX = 0;
                    int offsetZ = 0;
                    FormationOffset(profile ? profile->formationSlot : 0, offsetX, offsetZ);
                    const int dx = (candidate->pX - (owner->Position.x + offsetX)) >> FLOATNS;
                    const int dz = (candidate->pZ - (owner->Position.z + offsetZ)) >> FLOATNS;
                    if(dx * dx + dz * dz <= (260 * 260))
                        target = candidate;
                }
                break;
            }

            default:
                break;
        }
    }
    else
    {
        switch(tactic)
        {
            case COMPANION_TACTIC_OWNER_FOCUS:
                // Owner target has priority, but the preset must remain useful while
                // the owner has nothing selected.
                if(ownerTarget)
                    target = ownerTarget;
                else if(currentTarget)
                    target = currentTarget;
                else
                    target = FindBestHostile(character, owner, *state, currentTarget, ownerTarget);
                break;

            case COMPANION_TACTIC_DEFENSIVE:
            {
                // Prefer an enemy that is actually threatening the owner/party,
                // not merely the geometrically nearest monster. Retain a current
                // retaliating target first to avoid target thrash.
                if(currentTarget &&
                    (currentTarget->lpTargetChar == character || currentTarget->lpTargetPlayInfo == owner))
                    target = currentTarget;
                else
                    target = FindNearestThreatToParty(character, owner, *state);
                if(!target && ownerTarget)
                    target = ownerTarget;
                if(!target && currentTarget)
                    target = currentTarget;
                if(!target)
                    target = FindBestHostile(character, owner, *state, currentTarget, ownerTarget);
                break;
            }

            case COMPANION_TACTIC_HOLD_POSITION:
            {
                // Acquire enemies normally, but only fight them while they remain
                // close to this companion's formation anchor.
                smCHAR *candidate = currentTarget ? currentTarget : ownerTarget;
                if(!candidate)
                    candidate = FindNearestHostile(character);

                if(candidate)
                {
                    int offsetX = 0;
                    int offsetZ = 0;
                    FormationOffset(profile ? profile->formationSlot : 0, offsetX, offsetZ);
                    const int dx = (candidate->pX - (owner->Position.x + offsetX)) >> FLOATNS;
                    const int dz = (candidate->pZ - (owner->Position.z + offsetZ)) >> FLOATNS;
                    if(dx * dx + dz * dz <= (260 * 260))
                        target = candidate;
                }
                break;
            }

            case COMPANION_TACTIC_BALANCED:
            default:
                if(ownerTarget)
                    target = ownerTarget;
                else
                    target = FindBestHostile(character, owner, *state, currentTarget, ownerTarget);
                break;
        }
    }

    if(CompanionDebugTick(character->dwObjectSerial, 1000U))
    {
        int dist2 = -1;
        if(target)
        {
            const int dx = (target->pX - character->pX) >> FLOATNS;
            const int dz = (target->pZ - character->pZ) >> FLOATNS;
            dist2 = dx * dx + dz * dz;
        }
        CompanionDebugLog("AI name='%s' serial=%lu mode=%d tactic=%d custom=%d rule=%d action=%d life=%d/%d motion=%d ownerTarget=%lu currentTarget=%lu chosen=%lu dist2=%d preferredRange=%d",
            managedSlot->characterName, (unsigned long)character->dwObjectSerial,
            (int)mode, (int)tactic, profile && profile->customTactics.enabled ? 1 : 0, matchedCustomRule, customAction,
            character->smCharInfo.Life[0], character->smCharInfo.Life[1],
            character->MotionInfo ? (int)character->MotionInfo->State : -1,
            ownerTarget ? (unsigned long)ownerTarget->dwObjectSerial : 0UL,
            currentTarget ? (unsigned long)currentTarget->dwObjectSerial : 0UL,
            target ? (unsigned long)target->dwObjectSerial : 0UL, dist2, preferredRange);
    }

    if(target)
    {
        CompanionSkillAttempt damageAttempt;
        const BYTE *activeSkillSlots = profile ? profile->activeSkillSlots : NULL;
        const bool skillLoadoutActive = profile && profile->skillLoadoutActive;
        if(TryExecuteCompanionDamageLoadout(character, owner, target, rightItemCode, leftItemCode,
            activeSkillSlots,
            profile ? profile->activeMinResourcePercent : NULL,
            profile ? profile->activeMinimumTargets : NULL,
            profile ? profile->activeTargetModes : NULL,
            skillLoadoutActive, managedSlot->skillRuntime, dwPlayServTime, &damageAttempt))
        {
            CompanionDebugLog("SKILL_CAST name='%s' serial=%lu code=0x%08lX kind=damage point=%d target=%lu mana=%d/%d stamina=%d/%d targetLife=%d/%d",
                managedSlot->characterName, (unsigned long)character->dwObjectSerial,
                (unsigned long)managedSlot->skillRuntime.lastSkillCode, damageAttempt.point,
                (unsigned long)target->dwObjectSerial,
                character->smCharInfo.Mana[0], character->smCharInfo.Mana[1],
                character->smCharInfo.Stamina[0], character->smCharInfo.Stamina[1],
                target->smCharInfo.Life[0], target->smCharInfo.Life[1]);
            SendRuntimeStateLocked(*state, true);
            SendVisualStateLocked(*state, true);
            return;
        }
        else if(damageAttempt.reason != COMPANION_SKILL_COOLDOWN &&
                damageAttempt.reason != COMPANION_SKILL_LOCKED &&
                CompanionDebugEnabled() &&
                ShouldLogCompanionSkillReject(managedSlot->skillRuntime, 2, damageAttempt, dwPlayServTime, 5000U))
        {
            CompanionDebugLog(
                "SKILL_CHECK name='%s' serial=%lu kind=damage code=0x%08lX point=%d reason=%s job=%lu level=%d reqLevel=%d RH=0x%08lX LH=0x%08lX mana=%d/%d reqMana=%d reserveMana=%d stamina=%d/%d reqStamina=%d reserveStamina=%d range=%d dist2=%d target=%lu",
                managedSlot->characterName, (unsigned long)character->dwObjectSerial,
                (unsigned long)damageAttempt.skillCode, damageAttempt.point,
                CompanionSkillRejectReasonName(damageAttempt.reason),
                (unsigned long)character->smCharInfo.JOB_CODE, character->smCharInfo.Level,
                damageAttempt.requiredLevel,
                (unsigned long)rightItemCode, (unsigned long)leftItemCode,
                character->smCharInfo.Mana[0], character->smCharInfo.Mana[1], damageAttempt.manaCost, damageAttempt.manaReserve,
                character->smCharInfo.Stamina[0], character->smCharInfo.Stamina[1], damageAttempt.staminaCost, damageAttempt.staminaReserve,
                damageAttempt.range, damageAttempt.distanceSquared,
                target ? (unsigned long)target->dwObjectSerial : 0UL);
        }
        // Lightweight stuck/path recovery: if an out-of-range companion has
        // moved less than eight world units across two one-second probes, reset
        // the legacy move state before reissuing the combat trace. This avoids
        // leaving AutoMoveStep/TargetMoveCount wedged indefinitely without
        // introducing a second pathfinder.
        if((DWORD)(dwPlayServTime - managedSlot->lastProgressCheckTime) >= 1000U)
        {
            const int movedX = (character->pX - managedSlot->lastProgressX) >> FLOATNS;
            const int movedZ = (character->pZ - managedSlot->lastProgressZ) >> FLOATNS;
            const int targetDx = (target->pX - character->pX) >> FLOATNS;
            const int targetDz = (target->pZ - character->pZ) >> FLOATNS;
            int combatRange = preferredRange > 0 ? preferredRange : character->smCharInfo.Shooting_Range + 32;
            combatRange = std::max(80, std::min(600, combatRange));
            if(targetDx * targetDx + targetDz * targetDz > combatRange * combatRange &&
               managedSlot->lastProgressCheckTime && movedX * movedX + movedZ * movedZ < (8 * 8))
                ++managedSlot->stuckTicks;
            else
                managedSlot->stuckTicks = 0;

            managedSlot->lastProgressCheckTime = dwPlayServTime;
            managedSlot->lastProgressX = character->pX;
            managedSlot->lastProgressZ = character->pZ;
            if(managedSlot->stuckTicks >= 2)
            {
                character->TargetMoveCount = 0;
                character->AutoMoveStep = 0;
                character->TragetTraceMode = 0;
                character->SetNextTarget(0, 0, 0);
                managedSlot->stuckTicks = 0;
                CompanionDebugLog("AI_STUCK_RECOVER name='%s' serial=%lu target=%lu",
                    managedSlot->characterName, (unsigned long)character->dwObjectSerial,
                    (unsigned long)target->dwObjectSerial);
            }
        }

        int formationSlot = profile ? profile->formationSlot : -1;
        if(formationSlot < 0 || formationSlot >= COMPANION_MAX_ACTIVE)
            formationSlot = (int)(managedSlot - &state->slots[0]);
        const bool combatSpacing = HasOtherCompanionTargeting(*state, character, target);
        EngageTarget(character, target, preferredRange, formationSlot, combatSpacing);
    }
    else
    {
        managedSlot->stuckTicks = 0;
        managedSlot->lastProgressCheckTime = 0;
        DisengageTarget(character);
    }
}

bool GetFollowPosition(smCHAR *character, int *x, int *z)
{
    if(!character || !character->lpMasterPlayInfo || !x || !z)
        return false;

    std::lock_guard<std::mutex> guard(g_mutex);
    OwnerState *state = FindState(character->lpMasterPlayInfo);
    if(!state)
        return false;

    for(int i = 0; i < COMPANION_MAX_ACTIVE; ++i)
    {
        CompanionSlot &slot = state->slots[i];
        if(!IsSlotActive(*state, slot) || slot.entity != character)
            continue;

        LoadoutOverride *profile = FindLoadoutOverride(*state, slot.characterName, true);

        // Character::Main applies legacy master-follow after ApplyAI(). That
        // path clears lpTargetChar once a summon is ~300 units from its owner
        // anchor. With formation offsets, only the companion on the target side
        // could keep an aggressive chase while the others were pulled back.
        // Suppress only the soft follow correction while an aggressive companion
        // has a valid target; the hard leash still wins at 475+ units.
        const CompanionMode mode = profile ? profile->mode : state->mode;
        const CompanionTacticPreset tactic = profile ? profile->tactic : COMPANION_TACTIC_BALANCED;
        if(mode == COMPANION_MODE_AGGRESSIVE &&
            tactic != COMPANION_TACTIC_HOLD_POSITION &&
            IsValidHostile(character->lpTargetChar))
        {
            const int ownerDx = (character->pX - state->owner->Position.x) >> FLOATNS;
            const int ownerDz = (character->pZ - state->owner->Position.z) >> FLOATNS;
            if(ownerDx * ownerDx + ownerDz * ownerDz < (475 * 475))
            {
                *x = character->pX;
                *z = character->pZ;
                return true;
            }
        }

        int offsetX = 0;
        int offsetZ = 0;
        FormationOffset(profile ? profile->formationSlot : i, offsetX, offsetZ);
        *x = state->owner->Position.x + offsetX;
        *z = state->owner->Position.z + offsetZ;
        return true;
    }
    return false;
}

INT64 AwardExperience(rsPLAYINFO *owner, smCHAR *killedMonster, float partyShareRate)
{
    if(!owner || !killedMonster || killedMonster->smMonsterInfo.GetExp <= 0 || partyShareRate <= 0.0f)
        return 0;

    std::lock_guard<std::mutex> guard(g_mutex);
    OwnerState *state = FindState(owner);
    if(!state)
        return 0;

    INT64 totalAwarded = 0;
    const DWORD flushInterval = (DWORD)GetExpFlushSeconds() * 1000U;

    for(int i = 0; i < COMPANION_MAX_ACTIVE; ++i)
    {
        CompanionSlot &slot = state->slots[i];
        if(!IsSlotActive(*state, slot))
            continue;

        INT64 exp = ComputeBaseExperience(
            killedMonster->smMonsterInfo.GetExp,
            killedMonster->smCharInfo.Level,
            slot.cachedLevel);
        exp = static_cast<INT64>(exp * partyShareRate);
        if(exp <= 0)
            continue;

        INT64 currentVirtualExp = slot.cachedExp + slot.pendingExp;
        INT64 nextVirtualExp = currentVirtualExp;
        if(currentVirtualExp < ExpLevelTable[CHAR_LEVEL_MAX])
        {
            if(exp > ExpLevelTable[CHAR_LEVEL_MAX] - currentVirtualExp)
                nextVirtualExp = ExpLevelTable[CHAR_LEVEL_MAX];
            else
                nextVirtualExp = currentVirtualExp + exp;
        }

        // Match the normal player AddExp gate: the award that would cross the
        // legacy rank threshold is discarded until ChangeJob is high enough.
        if(nextVirtualExp > ExpLevelTable[121] && slot.changeJob < 3)
            continue;

        const INT64 acceptedExp = nextVirtualExp - currentVirtualExp;
        if(acceptedExp <= 0)
            continue;

        slot.pendingExp += acceptedExp;
        totalAwarded += acceptedExp;

        while(slot.cachedLevel < CHAR_LEVEL_MAX &&
            nextVirtualExp >= ExpLevelTable[slot.cachedLevel])
        {
            ++slot.cachedLevel;
        }
        if(slot.entity && slot.entity->Flag)
            slot.entity->smCharInfo.Level = slot.cachedLevel;

        if((DWORD)(dwPlayServTime - slot.lastExpFlushTime) >= flushInterval)
            FlushSlotExperience(*state, slot);
    }

    return totalAwarded;
}

DWORD GetConfiguredCrystalItemCode()
{
    // Decimal item code. 0 disables item interception while keeping commands.
    return (DWORD)SummitCompanionConfigInt("CrystalItemCode", 0);
}

int HandleCrystalUse(rsPLAYINFO *owner, DWORD itemCode)
{
    const DWORD configuredCode = GetConfiguredCrystalItemCode();
    if(!configuredCode || configuredCode != itemCode)
        return 0;

    const int result = SummonSelected(owner);
    return result == TRUE ? TRUE : -1;
}

namespace
{
int UpdateCompanionBehavior(
    rsPLAYINFO *owner,
    const char *characterName,
    int setting,
    int value)
{
    if(!owner || !characterName || !characterName[0])
        return -8;

    std::lock_guard<std::mutex> guard(g_mutex);
    OwnerState &state = GetOrCreateState(owner);
    LoadoutOverride *profile = FindLoadoutOverride(state, characterName, true);
    if(!profile)
        return -9;

    const LoadoutOverride previous = *profile;
    switch(setting)
    {
        case 0: // mode
            if(value < static_cast<int>(COMPANION_MODE_AGGRESSIVE) ||
                value > static_cast<int>(COMPANION_MODE_PASSIVE))
                return -10;
            profile->mode = static_cast<CompanionMode>(value);
            break;

        case 1: // tactic preset
            if(value < static_cast<int>(COMPANION_TACTIC_BALANCED) ||
                value > static_cast<int>(COMPANION_TACTIC_HOLD_POSITION))
                return -10;
            profile->tactic = static_cast<CompanionTacticPreset>(value);
            break;

        case 2: // retreat HP percent
            if(value < 0 || value > 90)
                return -10;
            profile->retreatHpPercent = value;
            break;

        case 3: // preferred combat range; 0 = weapon/default range
            if(value < 0 || value > 600)
                return -10;
            profile->preferredRange = value;
            break;

        default:
            return -10;
    }

    if(!PersistLoadoutOverride(state, *profile))
    {
        *profile = previous;
        CompanionDebugLog("BEHAVIOR_FAIL name='%s' setting=%d value=%d result=-11", characterName, setting, value);
        return -11;
    }
    CompanionDebugLog("BEHAVIOR name='%s' setting=%d value=%d mode=%d tactic=%d retreat=%d range=%d",
        characterName, setting, value, (int)profile->mode, (int)profile->tactic,
        profile->retreatHpPercent, profile->preferredRange);
    return TRUE;
}
int UpdateCustomTactics(rsPLAYINFO *owner, const char *characterName, int operation, int value)
{
    if(!owner || !owner->getAccount() || !owner->getAccount()[0] ||
       !characterName || !characterName[0])
        return -8;

    std::lock_guard<std::mutex> guard(g_mutex);
    OwnerState &state = GetOrCreateState(owner);
    LoadoutOverride *profile = FindLoadoutOverride(state, characterName, true);
    if(!profile)
        return -9;

    CompanionTacticsConfig previous = profile->customTactics;
    int slot = value < 0 ? -value - 1 : value - 1;

    switch(operation)
    {
        case 0: // enable/disable custom engine
            profile->customTactics.enabled = !profile->customTactics.enabled;
            break;

        case 1: // toggle rule enabled
            if(slot < 0 || slot >= protocol::COMPANION_UI_TACTIC_RULES) return -10;
            profile->customTactics.rules[slot].enabled = !profile->customTactics.rules[slot].enabled;
            break;

        case 2: // cycle condition
            if(slot < 0 || slot >= protocol::COMPANION_UI_TACTIC_RULES) return -10;
            profile->customTactics.rules[slot].condition =
                (profile->customTactics.rules[slot].condition + 1) %
                (protocol::COMPANION_TACTIC_CONDITION_PARTY_THREATENED + 1);
            switch(profile->customTactics.rules[slot].condition)
            {
                case protocol::COMPANION_TACTIC_CONDITION_SELF_HP_BELOW:
                case protocol::COMPANION_TACTIC_CONDITION_OWNER_HP_BELOW:
                    profile->customTactics.rules[slot].threshold = 25;
                    break;
                case protocol::COMPANION_TACTIC_CONDITION_ENEMIES_NEARBY_AT_LEAST:
                    profile->customTactics.rules[slot].threshold = 3;
                    break;
                default:
                    profile->customTactics.rules[slot].threshold = 0;
                    break;
            }
            break;

        case 3: // cycle action
            if(slot < 0 || slot >= protocol::COMPANION_UI_TACTIC_RULES) return -10;
            profile->customTactics.rules[slot].action =
                (profile->customTactics.rules[slot].action + 1) %
                (protocol::COMPANION_TACTIC_ACTION_HOLD_POSITION + 1);
            break;

        case 4: // cycle threshold
            if(slot < 0 || slot >= protocol::COMPANION_UI_TACTIC_RULES) return -10;
            switch(profile->customTactics.rules[slot].condition)
            {
                case protocol::COMPANION_TACTIC_CONDITION_SELF_HP_BELOW:
                case protocol::COMPANION_TACTIC_CONDITION_OWNER_HP_BELOW:
                {
                    int next = profile->customTactics.rules[slot].threshold + 5;
                    if(next > 95) next = 5;
                    profile->customTactics.rules[slot].threshold = next;
                    break;
                }
                case protocol::COMPANION_TACTIC_CONDITION_ENEMIES_NEARBY_AT_LEAST:
                {
                    int next = profile->customTactics.rules[slot].threshold + 1;
                    if(next > 8) next = 1;
                    profile->customTactics.rules[slot].threshold = next;
                    break;
                }
                default:
                    profile->customTactics.rules[slot].threshold = 0;
                    break;
            }
            break;

        case 5: // reorder; negative value moves that slot up, positive moves down
        {
            if(slot < 0 || slot >= protocol::COMPANION_UI_TACTIC_RULES) return -10;
            const int other = value < 0 ? slot - 1 : slot + 1;
            if(other < 0 || other >= protocol::COMPANION_UI_TACTIC_RULES)
                return TRUE;
            const protocol::CompanionUITacticRule temp = profile->customTactics.rules[slot];
            profile->customTactics.rules[slot] = profile->customTactics.rules[other];
            profile->customTactics.rules[other] = temp;
            break;
        }

        default:
            return -10;
    }

    if(!SaveCompanionTactics(owner->getAccount(), characterName, profile->customTactics))
    {
        profile->customTactics = previous;
        CompanionDebugLog("TACTIC_EDIT_FAIL name='%s' operation=%d value=%d", characterName, operation, value);
        return -11;
    }

    CompanionDebugLog("TACTIC_EDIT name='%s' operation=%d value=%d enabled=%d",
        characterName, operation, value, profile->customTactics.enabled ? 1 : 0);
    return TRUE;
}

} // namespace

int SetMode(rsPLAYINFO *owner, CompanionMode mode)
{
    if(!owner || mode < COMPANION_MODE_AGGRESSIVE || mode > COMPANION_MODE_PASSIVE)
        return FALSE;

    char selected[32] = { 0 };
    {
        std::lock_guard<std::mutex> guard(g_mutex);
        OwnerState &state = GetOrCreateState(owner);
        state.mode = mode;
        if(state.selectedName[0])
            CopyText32(selected, state.selectedName);
    }

    // Legacy /companion mode keeps working: when a character is selected, the
    // command now changes that companion's persistent profile. With no
    // selection it only updates the owner's session default.
    if(selected[0])
        return UpdateCompanionBehavior(owner, selected, 0, static_cast<int>(mode));
    return TRUE;
}

CompanionMode GetMode(rsPLAYINFO *owner)
{
    if(!owner)
        return COMPANION_MODE_AGGRESSIVE;

    std::lock_guard<std::mutex> guard(g_mutex);
    OwnerState *state = FindState(owner);
    if(!state)
        return COMPANION_MODE_AGGRESSIVE;

    if(state->selectedName[0])
    {
        LoadoutOverride *profile = FindLoadoutOverride(*state, state->selectedName, true);
        if(profile)
            return profile->mode;
    }
    return state->mode;
}

const char *GetSelectedName(rsPLAYINFO *owner)
{
    static thread_local char result[32];
    result[0] = 0;

    if(!owner)
        return result;

    std::lock_guard<std::mutex> guard(g_mutex);
    OwnerState *state = FindState(owner);
    if(state && state->selectedName[0])
        strcpy_s(result, sizeof(result), state->selectedName);
    return result;
}

void SendUIState(rsPLAYINFO *owner, int lastResult)
{
    if(!owner || !owner->lpsmSock || !owner->getAccount() || !owner->getAccount()[0])
        return;

    char names[sPLAY_CHAR_MAX][32];
    ZeroMemory(names, sizeof(names));
    const int rawCount = rsGetAccountCharacterNames(owner->getAccount(), names, sPLAY_CHAR_MAX);
    const int count = std::max(0, std::min(rawCount, protocol::COMPANION_UI_MAX_CHARACTERS));

    CharacterEquipmentSnapshot snapshots[protocol::COMPANION_UI_MAX_CHARACTERS];
    for(int i = 0; i < count; ++i)
    {
        if(names[i][0])
        {
            if(!LoadCharacterSnapshot(owner, names[i], snapshots[i]))
            {
                snapshots[i].valid = false;
                CopyText32(snapshots[i].characterName, names[i]);
            }
        }
    }

    protocol::CompanionUIStatePacket packet;
    ZeroMemory(&packet, sizeof(packet));
    packet.size = sizeof(packet);
    packet.code = protocol::OPCODE_COMPANION_UI_STATE;
    packet.version = protocol::COMPANION_UI_PROTOCOL_VERSION;
    packet.maxActive = COMPANION_MAX_ACTIVE;
    packet.entryCount = count;
    packet.lastResult = lastResult;

    {
        std::lock_guard<std::mutex> guard(g_mutex);
        OwnerState &state = GetOrCreateState(owner);
        PruneState(state);
        packet.mode = (int)state.mode;

        if(!state.selectedName[0])
        {
            for(int i = 0; i < count; ++i)
            {
                if(names[i][0] && (!owner->getName() || lstrcmpiA(owner->getName(), names[i]) != 0))
                {
                    CopyText32(state.selectedName, names[i]);
                    break;
                }
            }
        }

        for(int slotIndex = 0; slotIndex < COMPANION_MAX_ACTIVE; ++slotIndex)
        {
            if(IsSlotActive(state, state.slots[slotIndex]))
                ++packet.activeCount;
        }

        for(int i = 0; i < count; ++i)
        {
            protocol::CompanionUIEntry &entry = packet.entries[i];
            CopyText32(entry.characterName, names[i]);
            entry.currentCharacter =
                owner->getName() && lstrcmpiA(owner->getName(), names[i]) == 0 ? TRUE : FALSE;
            entry.selected =
                state.selectedName[0] && lstrcmpiA(state.selectedName, names[i]) == 0 ? TRUE : FALSE;

            CompanionSlot *slot = FindSlotByName(state, names[i]);
            entry.active = slot && IsSlotActive(state, *slot) ? TRUE : FALSE;

            LoadoutOverride *profile = FindLoadoutOverride(state, names[i], true);
            if(profile)
            {
                entry.mode = static_cast<int>(profile->mode);
                entry.tactic = static_cast<int>(profile->tactic);
                entry.retreatHpPercent = profile->retreatHpPercent;
                entry.preferredRange = profile->preferredRange;
                if(entry.selected)
                    packet.mode = entry.mode;
            }

            const CharacterEquipmentSnapshot &snapshot = snapshots[i];
            if(snapshot.valid)
            {
                entry.level = snapshot.level;
                entry.jobCode = snapshot.jobCode;
                entry.life = snapshot.life;
                entry.maxLife = snapshot.maxLife;
                entry.mana = snapshot.mana;
                entry.maxMana = snapshot.maxMana;
                entry.dead = snapshot.maxLife > 0 && snapshot.life <= 0 ? TRUE : FALSE;
                DWORD rightCode = snapshot.defaultRight;
                DWORD leftCode = snapshot.defaultLeft;

                if(profile && profile->active)
                {
                    rightCode = profile->rightCode;
                    leftCode = profile->leftCode;
                }

                if(entry.selected && profile)
                {
                    packet.skillLoadoutActive = profile->skillLoadoutActive ? TRUE : FALSE;
                    packet.buffLoadoutActive = profile->buffLoadoutActive ? TRUE : FALSE;
                    packet.customTacticsEnabled = profile->customTactics.enabled ? TRUE : FALSE;
                    for(int tacticRule = 0; tacticRule < protocol::COMPANION_UI_TACTIC_RULES; ++tacticRule)
                        packet.tacticRules[tacticRule] = profile->customTactics.rules[tacticRule];
                    for(int skillSlot = 0; skillSlot < 4; ++skillSlot)
                    {
                        protocol::CompanionUISkillSlot &active = packet.activeSkills[skillSlot];
                        active.useSlot = profile->activeSkillSlots[skillSlot];
                        active.minResourcePercent = profile->activeMinResourcePercent[skillSlot];
                        active.minimumTargets = profile->activeMinimumTargets[skillSlot];
                        active.targetMode = profile->activeTargetModes[skillSlot];
                        CompanionSkillDescriptor descriptor;
                        if(active.useSlot > 0 && GetCompanionSkillDescriptor(snapshot.jobCode,
                            snapshot.skillPoints, active.useSlot, &descriptor))
                        {
                            active.skillCode = descriptor.skillCode;
                            active.point = descriptor.point;
                            CopyText32(active.skillName, descriptor.name);
                        }

                        protocol::CompanionUISkillSlot &buff = packet.buffSkills[skillSlot];
                        buff.useSlot = profile->buffSkillSlots[skillSlot];
                        buff.intervalSeconds = profile->buffIntervalSeconds[skillSlot];
                        if(buff.useSlot > 0 && GetCompanionSkillDescriptor(snapshot.jobCode,
                            snapshot.skillPoints, buff.useSlot, &descriptor))
                        {
                            buff.skillCode = descriptor.skillCode;
                            buff.point = descriptor.point;
                            CopyText32(buff.skillName, descriptor.name);
                        }
                    }
                }

                entry.rightItemCode = rightCode;
                entry.leftItemCode = leftCode;
                const char *rightName = ChoiceName(snapshot.right, snapshot.rightCount, rightCode);
                const char *leftName = ChoiceName(snapshot.left, snapshot.leftCount, leftCode);
                CopyText32(entry.rightItemName, rightName);
                CopyText32(entry.leftItemName, leftName);
            }

            if(entry.active && slot && slot->entity)
            {
                entry.life = slot->entity->smCharInfo.Life[0];
                entry.maxLife = slot->entity->smCharInfo.Life[1];
                entry.mana = slot->entity->smCharInfo.Mana[0];
                entry.maxMana = slot->entity->smCharInfo.Mana[1];
                entry.dead = entry.maxLife > 0 && entry.life <= 0 ? TRUE : FALSE;
                if(slot->entity->lpTargetChar && slot->entity->lpTargetChar->Flag)
                    CopyText32(entry.targetName, slot->entity->lpTargetChar->smCharInfo.szName);
            }
        }
    }

    owner->lpsmSock->Send((char *)&packet, packet.size, TRUE);
}

namespace
{
const char *ResolveActionName(rsPLAYINFO *owner, const protocol::CompanionUIActionPacket *packet, char (&buffer)[32])
{
    buffer[0] = 0;
    if(packet && packet->characterName[0])
    {
        CopyText32(buffer, packet->characterName);
        return buffer;
    }

    const char *selected = GetSelectedName(owner);
    if(selected && selected[0])
        CopyText32(buffer, selected);
    return buffer;
}

bool IsCharacterActive(rsPLAYINFO *owner, const char *characterName)
{
    std::lock_guard<std::mutex> guard(g_mutex);
    OwnerState *state = FindState(owner);
    if(!state)
        return false;
    CompanionSlot *slot = FindSlotByName(*state, characterName);
    return slot && IsSlotActive(*state, *slot);
}

bool ClearLoadoutOverride(rsPLAYINFO *owner, const char *characterName)
{
    std::lock_guard<std::mutex> guard(g_mutex);
    OwnerState &state = GetOrCreateState(owner);
    LoadoutOverride *loadout = FindLoadoutOverride(state, characterName, true);
    if(!loadout)
        return false;

    const LoadoutOverride previous = *loadout;
    loadout->active = false;
    loadout->rightCode = 0;
    loadout->leftCode = 0;
    if(!PersistLoadoutOverride(state, *loadout))
    {
        *loadout = previous;
        return false;
    }
    return true;
}

int ResyncFromDataServer(rsPLAYINFO *owner, const char *characterName)
{
    if(!owner || !characterName || !characterName[0])
        return FALSE;

    // Validate the DAT first so a bad/missing record cannot destroy a healthy
    // active companion. Resync deliberately clears the session loadout because
    // the user requested the authoritative saved state.
    CharacterEquipmentSnapshot snapshot;
    if(!LoadCharacterSnapshot(owner, characterName, snapshot))
        return -3;

    const bool wasActive = IsCharacterActive(owner, characterName);
    if(!ClearLoadoutOverride(owner, characterName))
        return -11;
    if(!wasActive)
        return TRUE;

    const int dismissed = Dismiss(owner, characterName);
    if(dismissed != TRUE)
        return dismissed;
    return Summon(owner, characterName);
}
} // namespace

void HandleUIAction(rsPLAYINFO *owner, const protocol::CompanionUIActionPacket *packet)
{
    if(!owner || !packet ||
        packet->size != sizeof(protocol::CompanionUIActionPacket) ||
        packet->code != protocol::OPCODE_COMPANION_UI_ACTION ||
        packet->version != protocol::COMPANION_UI_PROTOCOL_VERSION)
    {
        return;
    }

    int result = 0;
    char actionName[32] = { 0 };
    const char *name = ResolveActionName(owner, packet, actionName);

    switch(packet->action)
    {
        case protocol::COMPANION_UI_REFRESH:
            result = 0;
            break;

        case protocol::COMPANION_UI_SELECT:
            result = name[0] ? Select(owner, name) : -8;
            break;

        case protocol::COMPANION_UI_SUMMON:
            if(name[0])
            {
                const int selected = Select(owner, name);
                result = selected == TRUE ? Summon(owner, name) : selected;
            }
            else
                result = SummonSelected(owner);
            break;

        case protocol::COMPANION_UI_DISMISS:
            result = name[0] ? Dismiss(owner, name) : -8;
            break;

        case protocol::COMPANION_UI_DISMISS_ALL:
            result = DismissAll(owner);
            break;

        case protocol::COMPANION_UI_SET_MODE:
            if(packet->value < (int)COMPANION_MODE_AGGRESSIVE ||
                packet->value > (int)COMPANION_MODE_PASSIVE)
                result = -10;
            else if(name[0])
                result = UpdateCompanionBehavior(owner, name, 0, packet->value);
            else
                result = SetMode(owner, (CompanionMode)packet->value);
            break;

        case protocol::COMPANION_UI_SET_TACTIC:
            result = name[0] ? UpdateCompanionBehavior(owner, name, 1, packet->value) : -8;
            break;

        case protocol::COMPANION_UI_SET_RETREAT_HP:
            result = name[0] ? UpdateCompanionBehavior(owner, name, 2, packet->value) : -8;
            break;

        case protocol::COMPANION_UI_SET_PREFERRED_RANGE:
            result = name[0] ? UpdateCompanionBehavior(owner, name, 3, packet->value) : -8;
            break;

        case protocol::COMPANION_UI_TOGGLE_CUSTOM_TACTICS:
            result = name[0] ? UpdateCustomTactics(owner, name, 0, packet->value) : -8;
            break;
        case protocol::COMPANION_UI_TOGGLE_TACTIC_RULE:
            result = name[0] ? UpdateCustomTactics(owner, name, 1, packet->value) : -8;
            break;
        case protocol::COMPANION_UI_CYCLE_TACTIC_CONDITION:
            result = name[0] ? UpdateCustomTactics(owner, name, 2, packet->value) : -8;
            break;
        case protocol::COMPANION_UI_CYCLE_TACTIC_ACTION:
            result = name[0] ? UpdateCustomTactics(owner, name, 3, packet->value) : -8;
            break;
        case protocol::COMPANION_UI_CYCLE_TACTIC_THRESHOLD:
            result = name[0] ? UpdateCustomTactics(owner, name, 4, packet->value) : -8;
            break;
        case protocol::COMPANION_UI_MOVE_TACTIC_RULE:
            result = name[0] ? UpdateCustomTactics(owner, name, 5, packet->value) : -8;
            break;

        case protocol::COMPANION_UI_CYCLE_ACTIVE_SKILL:
            result = name[0] ? ChangeSkillLoadout(owner, name, false, packet->value) : -8;
            break;
        case protocol::COMPANION_UI_RESET_ACTIVE_SKILLS:
            result = name[0] ? ResetSkillLoadout(owner, name, false) : -8;
            break;
        case protocol::COMPANION_UI_CYCLE_BUFF_SKILL:
            result = name[0] ? ChangeSkillLoadout(owner, name, true, packet->value) : -8;
            break;
        case protocol::COMPANION_UI_ADJUST_BUFF_INTERVAL:
            result = name[0] ? AdjustBuffInterval(owner, name, packet->value) : -8;
            break;
        case protocol::COMPANION_UI_RESET_BUFFS:
            result = name[0] ? ResetSkillLoadout(owner, name, true) : -8;
            break;
        case protocol::COMPANION_UI_CYCLE_ACTIVE_MIN_RESOURCE:
            result = name[0] ? CycleActiveSkillPolicy(owner, name, packet->value, 0) : -8;
            break;
        case protocol::COMPANION_UI_CYCLE_ACTIVE_MIN_TARGETS:
            result = name[0] ? CycleActiveSkillPolicy(owner, name, packet->value, 1) : -8;
            break;
        case protocol::COMPANION_UI_CYCLE_ACTIVE_TARGET_MODE:
            result = name[0] ? CycleActiveSkillPolicy(owner, name, packet->value, 2) : -8;
            break;

        case protocol::COMPANION_UI_EQUIP_RIGHT_NEXT:
            result = name[0] ? ChangeEquipment(owner, name, true, +1) : -8;
            break;
        case protocol::COMPANION_UI_EQUIP_RIGHT_PREV:
            result = name[0] ? ChangeEquipment(owner, name, true, -1) : -8;
            break;
        case protocol::COMPANION_UI_EQUIP_LEFT_NEXT:
            result = name[0] ? ChangeEquipment(owner, name, false, +1) : -8;
            break;
        case protocol::COMPANION_UI_EQUIP_LEFT_PREV:
            result = name[0] ? ChangeEquipment(owner, name, false, -1) : -8;
            break;

        case protocol::COMPANION_UI_RESET_EQUIPMENT:
            result = name[0] ? ResetEquipmentInternal(owner, name) : -8;
            break;

        case protocol::COMPANION_UI_RESYNC_DAT:
            result = name[0] ? ResyncFromDataServer(owner, name) : -8;
            break;

        default:
            result = -10;
            break;
    }

    SendUIState(owner, result);
}

} // namespace companions
} // namespace gameplay
