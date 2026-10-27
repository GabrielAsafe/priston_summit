#include "content/v1/gameplay_config/content/GameplayConfigRuntime.h"
#include "systems/gameplay/companions/CompanionSystem.h"
#include "systems/gameplay/companions/v2/legacy/CompanionLegacyCommandsRuntime.h"
#include "systems/gameplay/companions/v2/legacy/CompanionLegacyLifecycleRuntime.h"
#include "systems/gameplay/companions/v2/legacy/CompanionLegacyUIRuntime.h"
#include "systems/gameplay/companions/v2/legacy/CompanionLegacyCharacterHooks.h"
#include "systems/gameplay/companions/v2/legacy/CompanionLegacyExperienceRuntime.h"
#include "gameplay/companions/CompanionProtocol.h"
#include "gameplay/companions/v2/CompanionTradeConfirmPolicy.h"
#include "gameplay/companions/v2/CompanionTradeAcceptedRecord350E.h"
#include "gameplay/companions/v2/CompanionTradeDetachedJournalWorker350B.h"
#include "gameplay/companions/v2/CompanionTradeResultDispatch350C.h"
#include "gameplay/companions/v2/CompanionTradeParticipantGate350B.h"
#include "gameplay/companions/v2/CompanionTradeAdmission350C.h"
#include "gameplay/companions/v2/CompanionTradeNormalizedRecord350D.h"
#include "gameplay/companions/v2/CompanionTradeBaseline350J.h"
#include "gameplay/companions/v2/CompanionTradeDeliveryWire.h"
#include "gameplay/companions/v2/CompanionTradeCanonicalInventoryDigest.h"
#include "gameplay/companions/v2/CompanionTradeLiveHandoff350Q.h"
#include "gameplay/companions/v2/CompanionTradePostSaveEquivalence351G.h" // SUMMIT351G
#include "gameplay/companions/v2/CompanionTradeDiskJournal.h"
#include "gameplay/companions/v2/CompanionTradeExactPairBackend.h"
#include "gameplay/companions/v2/CompanionTradeSaveActivityFence.h"
#include "gameplay/companions/v2/CompanionTradeRevision.h"
#include "gameplay/companions/v2/CompanionTradePreviewPolicy350P.h"
#include "gameplay/companions/v2/CompanionTradeParticipantIdentity.h"
#include "gameplay/companions/v2/CompanionDiagnostic315.h"
#include "gameplay/inventory/v2/EquipmentPolicy.h"
#include "systems/gameplay/inventory/v2/legacy/LegacyItemCatalogAdapter.h"
#include "gameplay/companions/v2/CompanionSkillProfile.h"
#include "systems/gameplay/companions/v2/CompanionRuntimeQuery.h"
#include "systems/gameplay/companions/CompanionProfileStore.h"
#include "systems/gameplay/companions/CompanionTacticsStore.h"
#include "systems/gameplay/companions/CompanionSkillExecutor.h"
#include "systems/gameplay/player/v2/legacy/LegacyPlayerAdapter.h"
#include "systems/gameplay/player/v2/legacy/LegacyCharacterRecordAdapter.h"
#include "systems/gameplay/player/v2/legacy/LegacyOnlinePlayerRegistry.h"
#include "systems/inventory/v2/legacy/LegacyInventoryRuntime.h"
#include "systems/gameplay/loot/v2/legacy/LegacyLootAdapter.h"
#include "systems/social/party/v2/legacy/LegacyPartyQueryAdapter.h"
#include "systems/gameplay/companions/v2/legacy/LegacyCompanionQueryAdapter.h"
#include "systems/gameplay/combat/v2/legacy/LegacyThreatService.h"
#include "systems/gameplay/combat/v2/legacy/LegacyLifeAdapter.h"
#include "systems/network/v2/legacy/LegacyPacketSender.h"
#include "opcodes.h"

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <atomic>
#include <memory>
#include <cstddef>
#include <cstdint>
#include <utility>
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

extern CRITICAL_SECTION cSaveDataSection;
extern CRITICAL_SECTION cSerSection;
namespace gameplay
{
namespace companions
{

namespace
{

// Used by early AI/loot helpers; definition lives later with the other
// companion target helpers.
struct OwnerState;
bool IsValidHostile(smCHAR *candidate);

bool TryReadThreat(smCHAR *actor, combat::ThreatSnapshot &out)
{
    if(!actor || !actor->dwObjectSerial)
        return false;
    return combat::legacy::ServerLegacyThreatService().TryGetThreat(
        EntityId(static_cast<std::uint32_t>(actor->dwObjectSerial)), out);
}

bool ThreatTargetsOwner(const combat::ThreatSnapshot &threat, const rsPLAYINFO *owner)
{
    return owner && threat.targetPlayer == PlayerId(static_cast<std::uint32_t>(owner->dwObjectSerial));
}

bool ThreatTargetsCompanion(const combat::ThreatSnapshot &threat, const smCHAR *companion)
{
    return companion && companion->dwObjectSerial &&
        threat.targetEntity == EntityId(static_cast<std::uint32_t>(companion->dwObjectSerial));
}

bool ThreatTargetsParty(const combat::ThreatSnapshot &threat, const rsPLAYINFO *owner, const OwnerState &state);

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
        : owner(NULL), ownerHandle(), mode(COMPANION_MODE_AGGRESSIVE),
          lastRuntimeSendTime(0), lastVisualSendTime(0)
    {
        selectedName[0] = 0;
    }

    rsPLAYINFO *owner;
    PlayerHandle ownerHandle;
    CompanionMode mode;
    DWORD lastRuntimeSendTime;
    DWORD lastVisualSendTime;
    char selectedName[32];
    CompanionSlot slots[COMPANION_MAX_ACTIVE];
    LoadoutOverride loadouts[protocol::COMPANION_UI_MAX_CHARACTERS];
};

bool ThreatTargetsParty(const combat::ThreatSnapshot &threat, const rsPLAYINFO *owner, const OwnerState &state)
{
    if(ThreatTargetsOwner(threat, owner))
        return true;
    if(!threat.targetEntity.Valid())
        return false;

    for(int slot = 0; slot < COMPANION_MAX_ACTIVE; ++slot)
    {
        const smCHAR *entity = state.slots[slot].entity;
        if(entity && entity->Flag && entity->dwObjectSerial &&
           threat.targetEntity == EntityId(static_cast<std::uint32_t>(entity->dwObjectSerial)))
            return true;
    }
    return false;
}

std::mutex g_mutex;
std::map<rsPLAYINFO *, OwnerState> g_states;

bool IsOwnerValid(const OwnerState &state)
{
    if(!state.owner || !state.ownerHandle.Valid())
        return false;

    return player::legacy::ServerLegacyOnlinePlayerRegistry().Resolve(state.ownerHandle) == state.owner;
}

network::SendResult SendOwnerPacket(rsPLAYINFO *owner, const void *data, std::size_t size, std::uint32_t opcode)
{
    const PlayerId playerId = player::legacy::ServerLegacyOnlinePlayerRegistry().IdOf(owner);
    return network::legacy::ServerLegacyPacketSender().Send(
        playerId, network::PacketView(data, size, opcode));
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
            player::CharacterProgressionRecord refreshed;
            const player::CharacterRecordKey recordKey(
                state.owner->getAccount() ? state.owner->getAccount() : "",
                slot.characterName);
            if(player::legacy::ServerLegacyCharacterRecordAdapter().TryLoadProgression(
                    recordKey, refreshed))
            {
                slot.entity->smCharInfo.StatePoint = refreshed.statePoint;
                slot.entity->smCharInfo.Next_Exp = refreshed.nextExperience;
                slot.entity->smCharInfo.ChangeJob = refreshed.changeJob;
                slot.entity->smCharInfo.JobBitMask = refreshed.jobBitMask;
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

void SendRuntimeStateLocked(OwnerState &state, bool force);
void SendVisualStateLocked(OwnerState &state, bool force);

void PruneState(OwnerState &state)
{
    const DWORD kDeadVisualHoldMs = (DWORD)GetResurrectionCorpseSeconds() * 1000U;
    bool presentationChanged = false;
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
                presentationChanged = true;
            }
            else
            {
                // The runtime entity is already gone but buffered EXP still
                // needs a later retry. Hide it from the owner immediately.
                slot.entity = NULL;
                presentationChanged = true;
            }
        }
        else if(slot.characterName[0] && !slot.entity && slot.pendingExp > 0)
        {
            // Retry a previously failed flush on the next state touch.
            if(FlushSlotExperience(state, slot))
            {
                ClearSlot(slot);
                presentationChanged = true;
            }
        }
    }

    if(presentationChanged)
    {
        // A zero-entry snapshot is meaningful: it tells the client to clear
        // stale HUD/visual state after natural death, expiry or ownership loss.
        SendRuntimeStateLocked(state, true);
        SendVisualStateLocked(state, true);
    }
}

OwnerState &GetOrCreateState(rsPLAYINFO *owner)
{
    OwnerState &state = g_states[owner];
    const PlayerHandle handle = player::legacy::ServerLegacyOnlinePlayerRegistry().HandleOf(owner);
    if(state.owner != owner ||
       state.ownerHandle.id != handle.id ||
       state.ownerHandle.generation != handle.generation)
    {
        state = OwnerState();
        state.owner = owner;
        state.ownerHandle = handle;
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

OwnerState *FindState(PlayerId owner)
{
    if(!owner.Valid())
        return NULL;

    for(std::map<rsPLAYINFO *, OwnerState>::iterator it = g_states.begin(); it != g_states.end();)
    {
        if(!IsOwnerValid(it->second))
        {
            it = g_states.erase(it);
            continue;
        }
        if(it->second.ownerHandle.id == owner)
        {
            PruneState(it->second);
            return &it->second;
        }
        ++it;
    }
    return NULL;
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

        combat::ThreatSnapshot threat;
        if(!TryReadThreat(candidate, threat) || !ThreatTargetsParty(threat, owner, state))
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
    combat::ThreatSnapshot threat;
    if(TryReadThreat(candidate, threat))
    {
        if(ThreatTargetsOwner(threat, owner))
            score -= 50000;

        if(threat.targetEntity.Valid())
        {
            for(int slot = 0; slot < COMPANION_MAX_ACTIVE; ++slot)
            {
                const smCHAR *entity = state.slots[slot].entity;
                if(entity && entity->Flag && entity->dwObjectSerial &&
                   threat.targetEntity == EntityId(static_cast<std::uint32_t>(entity->dwObjectSerial)))
                {
                    score -= 35000;
                    break;
                }
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
    if(!target || !target->dwObjectSerial)
        return false;
    const EntityId targetId(static_cast<std::uint32_t>(target->dwObjectSerial));
    for(int slot = 0; slot < COMPANION_MAX_ACTIVE; ++slot)
    {
        smCHAR *entity = state.slots[slot].entity;
        if(!entity || entity == character || !entity->Flag)
            continue;
        combat::ThreatSnapshot threat;
        if(TryReadThreat(entity, threat) && threat.targetEntity == targetId)
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
        const loot::LootDropHandle drop = loot::legacy::LegacyLootAdapter::MakeHandle(area, item);
        const PlayerId ownerId = player::legacy::ServerLegacyOnlinePlayerRegistry().IdOf(owner);
        const loot::LootPickupResult result = loot::legacy::ServerLegacyLootAdapter().Pickup(ownerId, drop);
        CompanionDebugLog("LOOT_PICKUP name='%s' serial=%lu code=0x%08lX item='%s' result=%d",
            slot.characterName, (unsigned long)character->dwObjectSerial,
            (unsigned long)code, itemName, static_cast<int>(result.code));
        ReleaseCompanionLootTarget(slot);
        return result.PickedUp();
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

// A persisted Companion loadout is a selection over the CURRENT DAT, not
// an inventory or an entitlement to keep equipment which was transferred away.
// A stale selection is replaced with the actual saved slot. Code zero (empty
// hand) remains a legitimate UI choice.
DWORD ResolveOwnedCompanionEquipCode(const EquipmentChoice *choices, int count,
                                     DWORD requested, DWORD datDefault)
{
    return HasChoice(choices, count, requested) ? requested : datDefault;
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

bool ReadEquipmentSnapshot(const player::CharacterPersistentSnapshot &recordData,
                           CharacterEquipmentSnapshot &snapshot)
{
    snapshot = CharacterEquipmentSnapshot();
    snapshot.valid = true;
    CopyText32(snapshot.characterName, recordData.characterName.c_str());
    snapshot.level = recordData.level;
    snapshot.jobCode = recordData.legacyJobCode;
    snapshot.life = recordData.life;
    snapshot.maxLife = recordData.maxLife;
    snapshot.mana = recordData.mana;
    snapshot.maxMana = recordData.maxMana;
    const int skillCount = std::min(
        static_cast<int>(recordData.logicalSkillPoints.size()),
        COMPANION_MAX_SKILL_SLOTS);
    for(int i = 0; i < skillCount; ++i)
        snapshot.skillPoints[i] = static_cast<BYTE>(recordData.logicalSkillPoints[static_cast<std::size_t>(i)]);

    // Always allow an empty hand from the UI. It is a visual/combat loadout
    // override only and never deletes or moves an inventory record.
    AddChoice(snapshot.right, snapshot.rightCount, 0, "Nenhum");
    AddChoice(snapshot.left, snapshot.leftCount, 0, "Nenhum");

    for(std::size_t item = 0; item < recordData.items.size(); ++item)
    {
        const player::CharacterRecordItem &recordItem = recordData.items[item];
        const DWORD code = static_cast<DWORD>(recordItem.itemCode);
        if(!code)
            continue;

        // Hand choices are derived from the same V2 equipment definitions as
        // every other slot. The old sItem.Class is unpopulated in this MinGW
        // server and silently hid normal weapons and shields from the UI.
        gameplay::inventory::EquipmentDefinition equipment;
        const bool known = gameplay::inventory::legacy::ServerLegacyItemCatalogAdapter()
            .TryGetEquipment(gameplay::ItemId(static_cast<std::uint32_t>(code)), equipment);
        const bool canRight = known && (equipment.allowedSlots & (1u << 1)) != 0u;
        const bool canLeft = known && (equipment.allowedSlots & (1u << 2)) != 0u;
        if(recordItem.position != -1 && canRight)
            AddChoice(snapshot.right, snapshot.rightCount, code, recordItem.itemName.c_str());
        if(recordItem.position != -1 &&
           (canLeft || (canRight && !equipment.twoHanded && recordItem.position == 2)))
            AddChoice(snapshot.left, snapshot.leftCount, code, recordItem.itemName.c_str());

        // This is intentionally the same persisted-slot contract used by the
        // real client inventory: sInven[0] is RHAND, sInven[1] is LHAND, and
        // load/restore assigns sInven[ItemPosition - 1].
        if(recordItem.position == 1 && !snapshot.defaultRight)
        {
            snapshot.defaultRight = code;
            CopyText32(snapshot.defaultRightName, recordItem.itemName.c_str());
        }
        else if(recordItem.position == 2 && !snapshot.defaultLeft)
        {
            snapshot.defaultLeft = code;
            CopyText32(snapshot.defaultLeftName, recordItem.itemName.c_str());
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

    player::CharacterPersistentSnapshot persisted;
    const player::CharacterRecordKey key(owner->getAccount(), characterName);
    if(!player::legacy::ServerLegacyCharacterRecordAdapter().TryLoadSnapshot(key, persisted))
        return false;

    return ReadEquipmentSnapshot(persisted, snapshot);
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

    (void)itemIndex;
    gameplay::inventory::EquipmentDefinition equipment;
    if(gameplay::inventory::legacy::ServerLegacyItemCatalogAdapter().TryGetEquipment(
           gameplay::ItemId(static_cast<std::uint32_t>(code)), equipment))
        return equipment.twoHanded;

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
        combat::ThreatSnapshot threat;
        if(IsValidHostile(candidate) && TryReadThreat(candidate, threat) &&
           ThreatTargetsCompanion(threat, companion))
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

    (void)SendOwnerPacket(state.owner, &packet, packet.size, packet.code);
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

    (void)SendOwnerPacket(state.owner, &packet, packet.size, packet.code);
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
    (void)SendOwnerPacket(owner, &info, info.size, info.code);
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
    gameplay::inventory::EquipmentDefinition rightDefinition;
    if(rightIndex >= 0 &&
       gameplay::inventory::legacy::ServerLegacyItemCatalogAdapter().TryGetEquipment(
           gameplay::ItemId(static_cast<std::uint32_t>(rightCode)), rightDefinition) &&
       (rightDefinition.allowedSlots & (1u << 1)) != 0u)
        companion->dwActionItemCode = rightCode;
    else if(leftIndex >= 0) {
        // A one-handed weapon explicitly present in LHAND may be a genuine
        // dual-wield weapon; a shield must never become the attack item.
        gameplay::inventory::EquipmentDefinition leftDefinition;
        if(gameplay::inventory::legacy::ServerLegacyItemCatalogAdapter().TryGetEquipment(
               gameplay::ItemId(static_cast<std::uint32_t>(leftCode)), leftDefinition) &&
           (leftDefinition.allowedSlots & (1u << 1)) != 0u && !leftDefinition.twoHanded)
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
    loadout->rightCode = ResolveOwnedCompanionEquipCode(
        snapshot.right, snapshot.rightCount, loadout->rightCode, snapshot.defaultRight);
    loadout->leftCode = ResolveOwnedCompanionEquipCode(
        snapshot.left, snapshot.leftCount, loadout->leftCode, snapshot.defaultLeft);
    if(rightHand)
        loadout->rightCode = CycleChoice(snapshot.right, snapshot.rightCount, loadout->rightCode, direction);
    else
        loadout->leftCode = CycleChoice(snapshot.left, snapshot.leftCount, loadout->leftCode, direction);

    const int rightIndex = loadout->rightCode ? GetSinItemNumFromCode(loadout->rightCode) : -1;
    if(IsCompanionTwoHandCode(loadout->rightCode, rightIndex))
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

int EquipInventoryItem(rsPLAYINFO *owner, const char *characterName, bool rightHand, int recordIndex)
{
    if (!owner || !owner->getAccount() || !owner->getAccount()[0] ||
        !characterName || !characterName[0] || recordIndex < -1 || recordIndex >= RECORD_ITEM_MAX ||
        (owner->getName() && lstrcmpiA(owner->getName(), characterName) == 0))
        return -10;
    if (!rsCheckAccountChar(owner->getAccount(), (char *)characterName))
        return -2;
    rsPLAYINFO *online = FindUserFromName((char *)characterName);
    if (online && online->lpsmSock)
        return -4;
    // -1 is a deliberate empty-hand loadout. This does NOT mutate the DAT or
    // delete a weapon: the item remains owned by the original character.
    if (recordIndex == -1)
    {
        CharacterEquipmentSnapshot snapshot;
        if (!LoadCharacterSnapshot(owner, characterName, snapshot)) return -3;
        std::lock_guard<std::mutex> guard(g_mutex);
        OwnerState &state = GetOrCreateState(owner);
        PruneState(state);
        LoadoutOverride *loadout = FindLoadoutOverride(state, characterName, true);
        if (!loadout) return -9;
        const LoadoutOverride previous = *loadout;
        if (!loadout->active)
        {
            loadout->rightCode = snapshot.defaultRight;
            loadout->leftCode = snapshot.defaultLeft;
            loadout->active = true;
        }
        if (rightHand) loadout->rightCode = 0;
        else loadout->leftCode = 0;
        if (!PersistLoadoutOverride(state, *loadout))
        {
            *loadout = previous;
            return -11;
        }
        CompanionSlot *slot = FindSlotByName(state, characterName);
        if (slot && IsSlotActive(state, *slot))
            ApplyEquipmentCodes(owner, slot->entity, loadout->rightCode, loadout->leftCode);
        return TRUE;
    }
    TRANS_RECORD_DATA record;
    ZeroMemory(&record, sizeof(record));
    if (!rsLoadAccountCharacterRecord(owner->getAccount(), characterName, &record) ||
        record.ItemCount < 0 || record.ItemCount > RECORD_ITEM_MAX ||
        record.ItemSubStart < 0 || record.ItemSubStart > record.ItemCount ||
        record.DataSize < 0 || record.DataSize > (int)sizeof(record.Data) ||
        recordIndex >= record.ItemCount)
        return -3;
    // Currently the validated loadout choices operate on the primary DAT bag.
    // Secondary inventory rows must not bypass those existing validation gates.
    if (recordIndex >= record.ItemSubStart)
        return -10;
    const BYTE *cursor = record.Data;
    const BYTE *end = cursor + record.DataSize;
    DWORD selectedCode = 0;
    sITEMINFO selectedInfo;
    ZeroMemory(&selectedInfo, sizeof(selectedInfo));
    for (int i = 0; i <= recordIndex; ++i)
    {
        if (end - cursor < (int)sizeof(int)) return -3;
        int packedSize = 0;
        memcpy(&packedSize, cursor, sizeof(packedSize));
        if (packedSize <= (int)sizeof(int) || packedSize > end - cursor) return -3;
        if (i == recordIndex)
        {
            sRECORD_ITEM item;
            ZeroMemory(&item, sizeof(item));
            DecodeCompress((BYTE *)cursor, (BYTE *)&item, sizeof(item));
            selectedCode = item.sItemInfo.CODE;
            selectedInfo = item.sItemInfo;
            if(item.ItemPosition == -1) return -14;
        }
        cursor += packedSize;
    }
    if (!selectedCode) return -10;
    // Hand choices are validated against the V2 catalog and persistent DAT,
    // rather than a stale client equipMask or code-only sItem.Class.
    gameplay::inventory::EquipmentRequest handRequest;
    handRequest.item = gameplay::ItemId(static_cast<std::uint32_t>(selectedCode));
    handRequest.slot = rightHand ? 1 : 2;
    handRequest.restricted = selectedInfo.NotUseFlag != 0;
    handRequest.required.level = selectedInfo.Level;
    handRequest.required.strength = selectedInfo.Strength;
    handRequest.required.spirit = selectedInfo.Spirit;
    handRequest.required.talent = selectedInfo.Talent;
    handRequest.required.dexterity = selectedInfo.Dexterity;
    handRequest.required.health = selectedInfo.Health;
    handRequest.available.level = record.smCharInfo.Level;
    handRequest.available.strength = record.smCharInfo.Strength;
    handRequest.available.spirit = record.smCharInfo.Spirit;
    handRequest.available.talent = record.smCharInfo.Talent;
    handRequest.available.dexterity = record.smCharInfo.Dexterity;
    handRequest.available.health = record.smCharInfo.Health;
    const auto handDecision = gameplay::inventory::EquipmentPolicy::Evaluate(
        gameplay::inventory::legacy::ServerLegacyItemCatalogAdapter(), handRequest);
    summit::companion::diagnostic::Trace("SERVER", "hand-v2-policy",
        handRequest.slot, recordIndex, handDecision.Allowed() ? 1 : -16,
        static_cast<int>(handDecision.code));
    if(!handDecision.Allowed()) return -16;
    CharacterEquipmentSnapshot snapshot;
    if (!LoadCharacterSnapshot(owner, characterName, snapshot)) return -3;
    if (rightHand && !HasChoice(snapshot.right, snapshot.rightCount, selectedCode)) {
        summit::companion::diagnostic::Trace("SERVER", "hand-choice-missing",1,recordIndex,-10);
        return -10;
    }
    if (!rightHand && !HasChoice(snapshot.left, snapshot.leftCount, selectedCode)) {
        summit::companion::diagnostic::Trace("SERVER", "hand-choice-missing",2,recordIndex,-10);
        return -10;
    }

    std::lock_guard<std::mutex> guard(g_mutex);
    OwnerState &state = GetOrCreateState(owner);
    PruneState(state);
    LoadoutOverride *loadout = FindLoadoutOverride(state, characterName, true);
    if (!loadout) return -9;
    const LoadoutOverride previous = *loadout;
    if (!loadout->active)
    {
        loadout->rightCode = snapshot.defaultRight;
        loadout->leftCode = snapshot.defaultLeft;
        loadout->active = true;
    }
    if (rightHand) loadout->rightCode = selectedCode;
    else loadout->leftCode = selectedCode;
    const int rightIndex = loadout->rightCode ? GetSinItemNumFromCode(loadout->rightCode) : -1;
    if(IsCompanionTwoHandCode(loadout->rightCode, rightIndex))
        loadout->leftCode = 0;
    if (!PersistLoadoutOverride(state, *loadout))
    {
        *loadout = previous;
        return -11;
    }
    CompanionSlot *slot = FindSlotByName(state, characterName);
    if (slot && IsSlotActive(state, *slot))
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
        const combat::LifeCommandResult killed = combat::legacy::ServerLegacyLifeAdapter().ConsumeLethal(
            EntityId(static_cast<std::uint32_t>(slot.entity->dwObjectSerial)),
            slot.entity->smCharInfo.Life[0]);
        CompanionDebugLog("KILL_ENTITY_DEAD name='%s' serial=%lu lifeResult=%d motion=%d frame=%d",
            slot.characterName, (unsigned long)slot.entity->dwObjectSerial, static_cast<int>(killed.code),
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

    // The visual loadout profile can outlive a traded/dropped item. Check the
    // live DAT before applying any saved override. Failure only disables the
    // override: the canonical record is still available for normal summon.
    CharacterEquipmentSnapshot ownedEquipment;
    const bool ownedEquipmentReady = LoadCharacterSnapshot(owner, characterName, ownedEquipment);

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
        if(loadout->active && ownedEquipmentReady)
        {
            const DWORD right = ResolveOwnedCompanionEquipCode(
                ownedEquipment.right, ownedEquipment.rightCount, loadout->rightCode,
                ownedEquipment.defaultRight);
            const DWORD left = ResolveOwnedCompanionEquipCode(
                ownedEquipment.left, ownedEquipment.leftCount, loadout->leftCode,
                ownedEquipment.defaultLeft);
            ApplyEquipmentCodes(owner, entity, right, left);
        }
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

std::uint32_t runtime::ActiveCount(PlayerId owner)
{
    CompanionStateSnapshot state;
    return runtime::TryGetState(owner, state) ? state.activeCount : 0u;
}

bool runtime::TryGetState(PlayerId owner, CompanionStateSnapshot& out)
{
    out = CompanionStateSnapshot();
    if(!owner.Valid())
        return false;

    std::lock_guard<std::mutex> guard(g_mutex);
    OwnerState *state = FindState(owner);
    if(!state)
        return false;

    for(int slot = 0; slot < COMPANION_MAX_ACTIVE; ++slot)
    {
        if(IsSlotActive(*state, state->slots[slot]))
            ++out.activeCount;
    }

    CompanionMode mode = state->mode;
    if(state->selectedName[0])
    {
        out.selectedName = state->selectedName;
        LoadoutOverride *profile = FindLoadoutOverride(*state, state->selectedName, true);
        if(profile)
            mode = profile->mode;
    }

    switch(mode)
    {
        case COMPANION_MODE_FOLLOW: out.mode = CompanionBehaviorMode::Follow; break;
        case COMPANION_MODE_PASSIVE: out.mode = CompanionBehaviorMode::Passive; break;
        default: out.mode = CompanionBehaviorMode::Aggressive; break;
    }
    return true;
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
    player::PlayerSnapshot ownerSnapshot;
    if(!player::legacy::ServerLegacyPlayerAdapter().TryGet(state->ownerHandle, ownerSnapshot))
        return;

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
    if(!ownerSnapshot.alive)
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
    if(!ownerSnapshot.accountKey.empty())
    {
        if(TryUseCompanionConsumable(ownerSnapshot.accountKey.c_str(), managedSlot->characterName,
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
        else if(HasCompanionHealingRole(static_cast<classes::PlayableClassId>(
                    character->smCharInfo.JOB_CODE)) &&
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
        if(HasCompanionHealingRole(static_cast<classes::PlayableClassId>(
               character->smCharInfo.JOB_CODE)) &&
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
                (ownerSnapshot.position.x + offsetX) >> FLOATNS,
                (ownerSnapshot.position.z + offsetZ) >> FLOATNS);
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
            (ownerSnapshot.position.x + offsetX) >> FLOATNS,
            (ownerSnapshot.position.z + offsetZ) >> FLOATNS);
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
                    const int dx = (candidate->pX - (ownerSnapshot.position.x + offsetX)) >> FLOATNS;
                    const int dz = (candidate->pZ - (ownerSnapshot.position.z + offsetZ)) >> FLOATNS;
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
                combat::ThreatSnapshot currentThreat;
                if(currentTarget && TryReadThreat(currentTarget, currentThreat) &&
                    (ThreatTargetsCompanion(currentThreat, character) || ThreatTargetsOwner(currentThreat, owner)))
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
                    const int dx = (candidate->pX - (ownerSnapshot.position.x + offsetX)) >> FLOATNS;
                    const int dz = (candidate->pZ - (ownerSnapshot.position.z + offsetZ)) >> FLOATNS;
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

    player::PlayerSnapshot ownerSnapshot;
    if(!player::legacy::ServerLegacyPlayerAdapter().TryGet(state->ownerHandle, ownerSnapshot))
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
            const int ownerDx = (character->pX - ownerSnapshot.position.x) >> FLOATNS;
            const int ownerDz = (character->pZ - ownerSnapshot.position.z) >> FLOATNS;
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
        *x = ownerSnapshot.position.x + offsetX;
        *z = ownerSnapshot.position.z + offsetZ;
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

std::uint32_t runtime::ConfiguredCrystalItemCode()
{
    // Decimal item code. 0 disables item interception while keeping commands.
    return static_cast<std::uint32_t>(SummitCompanionConfigInt("CrystalItemCode", 0));
}

int HandleCrystalUse(rsPLAYINFO *owner, std::uint32_t itemCode)
{
    const std::uint32_t configuredCode = runtime::ConfiguredCrystalItemCode();
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
                    rightCode = ResolveOwnedCompanionEquipCode(
                        snapshot.right, snapshot.rightCount, profile->rightCode, snapshot.defaultRight);
                    leftCode = ResolveOwnedCompanionEquipCode(
                        snapshot.left, snapshot.leftCount, profile->leftCode, snapshot.defaultLeft);
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

    (void)SendOwnerPacket(owner, &packet, packet.size, packet.code);
}

// Read-only, bounded DAT inventory transport. Never trusts client-provided items,
// never writes any character record and never exposes another account.
void SendInventoryPage(rsPLAYINFO *owner, const char *characterName, int requestedPage)
{
    if (!owner || !owner->getAccount() || !owner->getAccount()[0] ||
        !characterName || !characterName[0] || requestedPage < 0 || requestedPage > 1000)
        return;

    protocol::CompanionInventoryPagePacket packet;
    ZeroMemory(&packet, sizeof(packet));
    packet.size = sizeof(packet);
    packet.code = protocol::OPCODE_COMPANION_INVENTORY_PAGE;
    packet.version = protocol::COMPANION_INVENTORY_PROTOCOL_VERSION;
    CopyText32(packet.characterName, characterName);
    packet.status = -3;

    // Never open/display the currently logged-in character's live inventory
    // through this offline-DAT path.
    if ((owner->getName() && lstrcmpiA(owner->getName(), characterName) == 0) ||
        !rsCheckAccountChar(owner->getAccount(), (char *)characterName))
    {
        packet.status = -2;
    }
    else
    {
        rsPLAYINFO *online = FindUserFromName((char *)characterName);
        if (online && online->lpsmSock)
            packet.status = -4;
        else
        {
            TRANS_RECORD_DATA record;
            ZeroMemory(&record, sizeof(record));
            if (rsLoadAccountCharacterRecord(owner->getAccount(), characterName, &record) &&
                record.ItemCount >= 0 && record.ItemCount <= RECORD_ITEM_MAX &&
                record.DataSize >= 0 && record.DataSize <= (int)sizeof(record.Data) &&
                record.ItemSubStart >= 0 && record.ItemSubStart <= record.ItemCount)
            {
                CharacterEquipmentSnapshot snapshot;
                const bool validEquipment = LoadCharacterSnapshot(owner, characterName, snapshot);
                std::vector<protocol::CompanionInventoryItem> items;
                items.reserve(record.ItemCount);
                const BYTE *cursor = record.Data;
                const BYTE *end = record.Data + record.DataSize;
                bool valid = true;
                for (int i = 0; i < record.ItemCount; ++i)
                {
                    if (end - cursor < (int)sizeof(int)) { valid = false; break; }
                    int packedSize = 0;
                    memcpy(&packedSize, cursor, sizeof(packedSize));
                    if (packedSize <= (int)sizeof(int) || packedSize > end - cursor)
                    { valid = false; break; }
                    sRECORD_ITEM decoded;
                    ZeroMemory(&decoded, sizeof(decoded));
                    DecodeCompress((BYTE *)cursor, (BYTE *)&decoded, sizeof(decoded));
                    cursor += packedSize;
                    if (!decoded.sItemInfo.CODE) continue;
                    protocol::CompanionInventoryItem item;
                    ZeroMemory(&item, sizeof(item));
                    item.recordIndex = i;
                    item.itemPosition = decoded.ItemPosition;
                    item.itemCode = decoded.sItemInfo.CODE;
                    item.bagNumber = i < record.ItemSubStart ? 1 : 2;
                    const int x = decoded.x - 23, y = decoded.y - 436;
                    item.gridX = decoded.ItemPosition == 0 && x >= 0 && y >= 0 ? x/22 : -1;
                    item.gridY = decoded.ItemPosition == 0 && x >= 0 && y >= 0 ? y/22 : -1;
                    // The 306 loadout only validates primary-bag weapon choices;
                    // never advertise secondary inventory or armor as directly equippable.
                    if (validEquipment && i < record.ItemSubStart)
                    {
                        // Advertise only choices that the existing server loadout
                        // can really accept, never class/position approximations.
                        if (HasChoice(snapshot.right, snapshot.rightCount, item.itemCode))
                            item.equipMask |= (1u << 1);
                        if (HasChoice(snapshot.left, snapshot.leftCount, item.itemCode))
                            item.equipMask |= (1u << 2);
                    }
                    // One authoritative V2 equipment policy for the display
                    // and DAT writer. Do not reimplement legacy slot masks here.
                    if(i < record.ItemSubStart && decoded.ItemPosition != -1)
                    {
                        gameplay::inventory::EquipmentRequest request;
                        request.item = gameplay::ItemId(
                            static_cast<std::uint32_t>(decoded.sItemInfo.CODE));
                        request.restricted = decoded.sItemInfo.NotUseFlag != 0;
                        request.required.level = decoded.sItemInfo.Level;
                        request.required.strength = decoded.sItemInfo.Strength;
                        request.required.spirit = decoded.sItemInfo.Spirit;
                        request.required.talent = decoded.sItemInfo.Talent;
                        request.required.dexterity = decoded.sItemInfo.Dexterity;
                        request.required.health = decoded.sItemInfo.Health;
                        request.available.level = record.smCharInfo.Level;
                        request.available.strength = record.smCharInfo.Strength;
                        request.available.spirit = record.smCharInfo.Spirit;
                        request.available.talent = record.smCharInfo.Talent;
                        request.available.dexterity = record.smCharInfo.Dexterity;
                        request.available.health = record.smCharInfo.Health;
                        for(int slot=3; slot<=16; ++slot) {
                            request.slot = slot;
                            if(gameplay::inventory::EquipmentPolicy::Evaluate(
                                gameplay::inventory::legacy::ServerLegacyItemCatalogAdapter(),
                                request).Allowed())
                                item.equipMask |= (1u << slot);
                        }
                    }
                    CopyText32(item.itemName, decoded.sItemInfo.ItemName);
                    items.push_back(item);
                }
                if (valid && cursor == end)
                {
                    packet.status = 1;
                    packet.totalItems = static_cast<int>(items.size());
                    const int pageSize = protocol::COMPANION_INVENTORY_PAGE_SIZE;
                    const int lastPage = packet.totalItems > 0 ? (packet.totalItems - 1) / pageSize : 0;
                    packet.page = std::min(requestedPage, lastPage);
                    const int begin = packet.page * pageSize;
                    packet.itemCount = std::min(pageSize, packet.totalItems - begin);
                    for (int i = 0; i < packet.itemCount; ++i)
                        packet.items[i] = items[static_cast<std::size_t>(begin + i)];
                }
            }
        }
    }
    summit::companion::diagnostic::Trace("SERVER","inventory-page-response",0,
        requestedPage,packet.status,packet.itemCount);
    summit::companion::diagnostic::Trace("SERVER","inventory-total",0,
        requestedPage,packet.status,packet.totalItems);
    (void)SendOwnerPacket(owner, &packet, packet.size, packet.code);
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

    if(owner)
    {
        std::lock_guard<std::mutex> guard(g_mutex);
        OwnerState *state = FindState(owner);
        if(state && state->selectedName[0])
            CopyText32(buffer, state->selectedName);
    }
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

// Authenticated server-side pages for the player's trade UI. Do not expose
// full item records, do not accept an item blob from the client, and never
// mutate either inventory from a preview request.
void SendTradePreviewPage(rsPLAYINFO* owner, const char* name, int side, int requestedPage)
{
    protocol::CompanionTradePagePacket packet;
    ZeroMemory(&packet, sizeof(packet));
    packet.size = sizeof(packet);
    packet.code = protocol::OPCODE_COMPANION_TRADE_PAGE;
    packet.version = protocol::COMPANION_TRADE_PAGE_VERSION;
    packet.side = side;
    packet.page = requestedPage;
    packet.status = -2;
    if(!owner || !owner->lpsmSock || !owner->getAccount() ||
       !owner->getAccount()[0] || !owner->getName() ||
       !owner->getName()[0] || !name || !name[0] ||
       side < 0 || side > 1 || requestedPage < 0 || requestedPage > 1000)
        return;
    CopyText32(packet.characterName, name);
    if(lstrcmpiA(owner->getName(), name) == 0 ||
       !rsCheckAccountChar(owner->getAccount(), (char*)name))
    {
        (void)SendOwnerPacket(owner, &packet, packet.size, packet.code);
        return;
    }
    if(FindUserFromName((char*)name) && FindUserFromName((char*)name)->lpsmSock)
        packet.status = -4;
    else if(IsCharacterActive(owner, name))
        packet.status = -12;
    else if(side == 0)
    {
        const auto& registry = player::legacy::ServerLegacyOnlinePlayerRegistry();
        const PlayerHandle session = registry.HandleOf(owner);
        OnlineTradeIdentitySnapshot snapshot;
        if(inventory::legacy::ServerLegacyInventoryRuntime()
            .TryGetTradeIdentitySnapshot(session, snapshot))
        {
            // 350P: identity-only live ledger does not contain equip slots.
            // If a validated accepted record is already in memory, use it
            // ONLY to annotate the preview. No disk IO / journal / saves in
            // this cSerSection path. Commit still validates a fresh save.
            v2::CompanionTradeInventorySnapshot acceptedView;
            v2::AcceptedRecordEvidence350E evidence;
            const v2::SaveSessionKey saveSession(session.id.value, session.generation);
            const bool acceptedKnown =
                v2::ServerAcceptedRecordBridge350E().Copy(saveSession,evidence) &&
                player::legacy::ServerLegacyCharacterRecordAdapter()
                    .TryDecodeTradeBufferedPayloadCandidate(
                        evidence.bytes.data(),evidence.bytes.size(),
                        owner->getName(),acceptedView) ==
                    v2::BufferedTradeCaptureCode::CandidateReadOnly;
            const v2::TradePreviewPositionIndex350P knownPositions(
                acceptedKnown ? &acceptedView : nullptr);
            packet.status = 1;
            packet.sourceRevision = snapshot.revision;
            packet.totalItems = static_cast<int>(snapshot.entries.size());
            const int last = packet.totalItems ? (packet.totalItems-1) / protocol::COMPANION_TRADE_PAGE_SIZE : 0;
            packet.page = std::min(requestedPage, last);
            const int first = packet.page * protocol::COMPANION_TRADE_PAGE_SIZE;
            packet.itemCount = std::min(protocol::COMPANION_TRADE_PAGE_SIZE, packet.totalItems-first);
            for(int i=0; i<packet.itemCount; ++i)
            {
                const auto& entry = snapshot.entries[static_cast<std::size_t>(first+i)];
                auto& item = packet.items[i];
                item.index = static_cast<int>(entry.slot);
                item.itemCode = entry.identity.item.value;
                item.itemKey = entry.identity.key;
                item.itemChecksum = entry.identity.checksum;
                // -2: identity absent/ambiguous in the latest accepted save;
                // this is not a transferable item until the lists are fresh.
                item.position = knownPositions.PositionOrBag350P(
                    item.itemCode,item.itemKey,item.itemChecksum);
                // Online ledger has no trusted item names: show code, not a guess.
                _snprintf_s(item.name, sizeof(item.name), _TRUNCATE, "Item %08X", item.itemCode);
            }
        }
        else packet.status = -20;
    }
    else
    {
        v2::CompanionTradeInventorySnapshot snapshot;
        const player::CharacterRecordKey key(owner->getAccount(), name);
        const auto read = player::legacy::ServerLegacyCharacterRecordAdapter()
            .TryLoadTradeInventorySnapshot(key, snapshot);
        // 350P: the 350O decoder permits anonymous legacy/passive records.
        // The preview must not reject the whole character because of them.
        // TradeEligible still rejects duplicates, cursor and corrupt positions;
        // selection of a passive record remains forbidden by the server.
        if(v2::CanListTradePreview350P(read,snapshot))
        {
            packet.status = 1;
            packet.totalItems = static_cast<int>(snapshot.records.size());
            const int last = packet.totalItems ? (packet.totalItems-1) / protocol::COMPANION_TRADE_PAGE_SIZE : 0;
            packet.page = std::min(requestedPage, last);
            const int first = packet.page * protocol::COMPANION_TRADE_PAGE_SIZE;
            packet.itemCount = std::min(protocol::COMPANION_TRADE_PAGE_SIZE, packet.totalItems-first);
            // Include bag membership in the revision: moving A->B without
            // changing the item bytes must invalidate an earlier selection.
            packet.sourceRevision = v2::CompanionTradePreviewRevision(snapshot);
            if(!packet.sourceRevision) { packet.status=-21; packet.itemCount=0; packet.totalItems=0; }

            for(int i=0; i<packet.itemCount; ++i)
            {
                const auto& entry = snapshot.records[static_cast<std::size_t>(first+i)];
                auto& item = packet.items[i];
                item.index = static_cast<int>(entry.recordIndex);
                item.itemCode = entry.itemCode;
                item.itemKey = entry.itemKey;
                item.itemChecksum = entry.itemChecksum;
                item.position = entry.position;
                _snprintf_s(item.name, sizeof(item.name), _TRUNCATE, "Item %08X", item.itemCode);
            }
        }
        else {
            packet.status = -21;
            const unsigned flags =
                (snapshot.hasMissingInstanceIdentity?1u:0u) |
                (snapshot.hasDuplicateInstanceIdentity?2u:0u) |
                (snapshot.hasCursorItem?4u:0u) |
                (snapshot.hasUnknownPosition?8u:0u) |
                (snapshot.ambiguousSegments?16u:0u);
            std::printf("[SUMMIT350P_TRADE] stage=preview-companion-refused read=%d flags=%u\n",
                        static_cast<int>(read),flags);
        }
    }
    (void)SendOwnerPacket(owner, &packet, packet.size, packet.code);
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


// SUMMIT350H EXPERIMENTAL: a production-shaped detached journal context.
// No live rsPLAYINFO, socket or packet buffer is used in these classes.
// Keep admission disabled by default until the MinGW/Wine and disposable-DAT
// acceptance matrix proves that EVERY competing writer is excluded.
namespace {
using v2::DetachedTradeJob;
using v2::DetachedTradeOutcome;

// SUMMIT350J: real transfer path. A server operator may disable it immediately
// before start by setting SUMMIT_DISABLE_COMPANION_TRADE=1. Every admission
// still requires an authenticated, verified save and both participant leases.
bool Trade350HLabMode() {
    const char* disabled=std::getenv("SUMMIT_DISABLE_COMPANION_TRADE");
    if(disabled && std::strcmp(disabled,"1")==0) return false;
    // Keep the historical LAB variable only as an explicit opt-out; previous
    // releases required setting it to 1, which prevented normal transfers.
    const char* enabled=std::getenv("SUMMIT_350H_DISPOSABLE_DAT_LAB");
    return !enabled || std::strcmp(enabled,"0")!=0;
}

// SUMMIT350S: live inventory synchronization is on by default after a
// committed companion trade. Operators may disable only the live handoff
// (falling back to durable commit + relogin) by setting the existing flag to 0.
static bool Live350QEnabled() {
    const char* v=std::getenv("SUMMIT_ENABLE_TRADE_LIVE_350Q");
    return !v || std::strcmp(v,"0")!=0;
}
class DetachedPairHost350H final : public v2::ICompanionTradeExactPairHost {
public:
    explicit DetachedPairHost350H(const DetachedTradeJob& job):job_(job){}
    bool AcquireExternalLease(std::uint64_t owner,std::uint64_t companion) override {
        if(locked_ || !Matches(owner,companion)) return false;
        locked_=true;
        if(!StillOwnsExternalLease(owner,companion)) {locked_=false;return false;}
        return true;
    }
    void ReleaseExternalLease(std::uint64_t,std::uint64_t) override {locked_=false;}
    bool StillOwnsExternalLease(std::uint64_t owner,std::uint64_t companion) override {
        const v2::SaveSessionKey session(job_.session.serial,job_.session.generation);
        std::uint64_t accepted=0;
        return locked_ && Matches(owner,companion) &&
            v2::ServerTradeParticipantGate350B().IsReserved(owner) &&
            v2::ServerTradeParticipantGate350B().IsReserved(companion) &&
            v2::ServerSaveActivityFence().TryLeasedAcceptedSaveSequence(
                session,job_.leaseToken,accepted) &&
            accepted==job_.acceptedSaveSequence;
    }
    bool TrustedPathsUnderLease(std::uint64_t owner,std::uint64_t companion,
                       std::string& a,std::string& b) override {
        if(!StillOwnsExternalLease(owner,companion) ||
           job_.trustedOwnerPath.empty() || job_.trustedCompanionPath.empty() ||
           job_.trustedOwnerPath==job_.trustedCompanionPath) return false;
        a=job_.trustedOwnerPath; b=job_.trustedCompanionPath; return true;
    }
    bool BuildPreparedUnderLease(const v2::CompanionTradeOffer& offer,
                         v2::CompanionTradePrepared& prepared) override {
        if(!StillOwnsExternalLease(offer.authenticatedOwner,offer.companionCharacter))return false;
        std::vector<std::uint8_t> ownerRaw,companionRaw;
        if(!v2::CompanionTradeExactRecordStore::Read(job_.trustedOwnerPath,ownerRaw)) {
            std::printf("[SUMMIT350M_TRADE] stage=record-read-failed side=owner\n");
            return false;
        }
        if(!v2::CompanionTradeExactRecordStore::Read(job_.trustedCompanionPath,companionRaw)) {
            std::printf("[SUMMIT350M_TRADE] stage=record-read-failed side=companion\n");
            return false;
        }
        // Decode the ALREADY READ exact bytes. Calling the usual account DAT
        // loader here would hit its LoadGuard and reject our own pair lease.
        v2::CompanionTradeInventorySnapshot companionSnapshot;
        const auto companionStatus=player::legacy::ServerLegacyCharacterRecordAdapter()
            .TryDecodeTradeBufferedPayloadCandidate(companionRaw.data(),
             companionRaw.size(),job_.companionName.c_str(),companionSnapshot);
        if(companionStatus!=v2::BufferedTradeCaptureCode::CandidateReadOnly) {
            std::printf("[SUMMIT350M_TRADE] stage=companion-snapshot-invalid reason=%d\n",
                static_cast<int>(companionStatus));
            return false;
        }
        if(v2::CompanionTradePreviewRevision(companionSnapshot)!=
              job_.request.companionRevision) {
            std::printf("[SUMMIT350M_TRADE] stage=companion-revision-stale\n");
            return false;
        }
        const v2::TradeSide side=job_.request.side==0 ?
            v2::TradeSide::Player : v2::TradeSide::Companion;
        const v2::TradeSelectedItem item{side,
            static_cast<std::uint32_t>(job_.request.itemIndex),
            job_.request.itemCode,job_.request.itemKey,job_.request.itemChecksum};
        const auto allowed=[](const v2::CompanionTradeItemRecord& r,
                              v2::TradeSide)->bool {
            return r.position==0 && r.itemCode && r.itemKey && r.itemChecksum;
        };
        // First prove that the accepted online save and companion record can
        // produce a balanced, valid, two-sided trade. Do not touch either DAT
        // when item/slot/record validation fails.
        v2::CompanionTradePrepared verified;
        player::legacy::TradePairBuildDiagnostic350M buildReason;
        if(!player::legacy::ServerLegacyCharacterRecordAdapter()
            .BuildTradePairedRecordImages(job_.acceptedOwnerRecord,companionRaw,
                job_.ownerName.c_str(),job_.companionName.c_str(),offer,
                std::vector<v2::TradeSelectedItem>(1,item),allowed,verified,&buildReason)) {
            // No private names, paths, identities, item codes, or DAT bytes in log.
            std::printf("[SUMMIT350M_TRADE] stage=pair-build-refused cause=%d decode=%d transfer=%d plan=%d ownerImage=%d companionImage=%d unsafeFlags=%u selectionReason=%d\n",
                static_cast<int>(buildReason.stage),buildReason.decode,
                buildReason.transfer,buildReason.plan,buildReason.ownerImage,
                buildReason.companionImage,buildReason.unsafeFlags350N,
                buildReason.selection350N);
            std::printf("[SUMMIT350J_TRADE] stage=prepare-refused\n");
            return false;
        }
        // SUMMIT350J: legacy ACK confirms a save *buffer*, not necessarily an
        // on-disk DAT. Materialise exactly the validated, normalised online
        // save outside cSerSection, under both participant gates and the live
        // session lease. This is an ordinary save, BEFORE trade journal intent.
        // If interrupted here, only that accepted save (not a half-trade) has
        // been persisted. All subsequent item moves use the sealed journal.
        if(ownerRaw!=job_.acceptedOwnerRecord) {
            const auto flush=v2::MaterializeAcceptedOwner350J(
                job_.trustedOwnerPath,ownerRaw,job_.acceptedOwnerRecord);
            if(flush==v2::AcceptedOwnerBaseline350J::Uncertain) {
                std::printf("[SUMMIT350J_TRADE] stage=preflight-uncertain\n");
                preflightUncertain_=true;
                return false;
            }
            if(flush!=v2::AcceptedOwnerBaseline350J::Ready) {
                std::printf("[SUMMIT350J_TRADE] stage=preflight-refused\n");
                return false;
            }
        }
        // The pair backend re-reads and compares BOTH DATs before intent.
        if(ownerRaw!=job_.acceptedOwnerRecord) ownerRaw=job_.acceptedOwnerRecord;
        if(ownerRaw!=job_.acceptedOwnerRecord) return false;
        prepared=std::move(verified);
        return true;
    }
    bool PreflightUncertain() const {return preflightUncertain_;}
private:
    bool preflightUncertain_=false;
    bool Matches(std::uint64_t owner,std::uint64_t companion) const {
        return owner==job_.session.owner && companion==job_.session.companion &&
            job_.session.Valid() && job_.leaseToken &&
            job_.acceptedSaveSequence && !job_.acceptedOwnerRecord.empty() &&
            v2::TradeParticipantId(job_.accountName.c_str(),job_.ownerName.c_str())==owner &&
            v2::TradeParticipantId(job_.accountName.c_str(),job_.companionName.c_str())==companion;
    }
    const DetachedTradeJob& job_;
    bool locked_=false;
};

bool OpenTradeJournalDirectory350H() {
    if(!CreateDirectoryA("LOGS",nullptr) && GetLastError()!=ERROR_ALREADY_EXISTS)return false;
    const char* root="LOGS\\SummitCompanionTradeJournal";
    if(!CreateDirectoryA(root,nullptr) && GetLastError()!=ERROR_ALREADY_EXISTS)return false;
    const DWORD attrs=GetFileAttributesA(root);
    return attrs!=INVALID_FILE_ATTRIBUTES &&
        (attrs&FILE_ATTRIBUTE_DIRECTORY) && !(attrs&FILE_ATTRIBUTE_REPARSE_POINT);
}
class TradeContext350H final : public v2::IDetachedJournalContext {
public:
    explicit TradeContext350H(const DetachedTradeJob& job)
        :host_(job),backend_(host_),journal_("LOGS/SummitCompanionTradeJournal",backend_){}
    v2::ICompanionTradeJournalPort& Port() override {return journal_;}
    bool Healthy() const override {return journal_.Healthy() && !host_.PreflightUncertain();}
private:
    DetachedPairHost350H host_;
    v2::CompanionTradeExactPairBackend backend_;
    v2::CompanionTradeDiskJournal journal_;
};
class TradeFactory350H final : public v2::IDetachedJournalContextFactory {
public:
    std::unique_ptr<v2::IDetachedJournalContext> Open(const DetachedTradeJob& job) override {
        if(!job.session.Valid() || job.trustedOwnerPath.empty() ||
           job.trustedCompanionPath.empty() || !OpenTradeJournalDirectory350H())return nullptr;
        return std::unique_ptr<v2::IDetachedJournalContext>(new TradeContext350H(job));
    }
};
// SUMMIT350Q: worker-owned durable after-images. Never read DATs in WM_TIMER.
class CommittedLiveImages350Q final {
public:
    void Put(std::uint64_t nonce,std::vector<std::uint8_t> bytes) {
        if(!nonce || bytes.empty() || bytes.size()>v2::kTradeDeliveryMaxRecord) return;
        std::lock_guard<std::mutex> lock(mutex_);
        if(images_.size()<8) images_[nonce]=std::move(bytes);
    }
    bool Take(std::uint64_t nonce,std::vector<std::uint8_t>& out) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it=images_.find(nonce);
        if(it==images_.end()) return false;
        out=std::move(it->second);images_.erase(it);return true;
    }
private:
    std::mutex mutex_;
    std::map<std::uint64_t,std::vector<std::uint8_t>> images_;
};
CommittedLiveImages350Q& LiveImages350Q() {
    static CommittedLiveImages350Q images;return images;
}
class TradeExecutor350H final : public v2::IDetachedTradeExecutor {
public:
    TradeExecutor350H():journal_(factory_){}
    DetachedTradeOutcome Execute(const DetachedTradeJob& job) override {
        DetachedTradeOutcome outcome=DetachedTradeOutcome::Unknown();
        try {outcome=journal_.Execute(job);}
        catch(...) {outcome=DetachedTradeOutcome::Unknown();}
        const v2::SaveSessionKey key(job.session.serial,job.session.generation);
        if((outcome.status==2 || outcome.status==3) && outcome.durableOrUncertain) {
            // Read only AFTER the journal has durably committed and while
            // both participant gates still prevent a competing writer.
            std::vector<std::uint8_t> ownerAfter;
            if(v2::CompanionTradeExactRecordStore::Read(job.trustedOwnerPath,ownerAfter))
                LiveImages350Q().Put(job.tradeNonce,std::move(ownerAfter));
        }
        if(outcome.durableOrUncertain) {
            // No old save may overwrite either DAT, even if delivery fails.
            if(!v2::ServerSaveActivityFence().SealCommittedForReconnect(key,job.leaseToken))
                return DetachedTradeOutcome::Unknown();
        } else {
            (void)v2::ServerSaveActivityFence().ReleaseReservation(key,job.leaseToken);
            (void)v2::ServerTradeParticipantGate350B().ReleaseVerified(
                job.session.owner,job.session.companion);
        }
        return outcome;
    }
private:
    TradeFactory350H factory_;
    v2::DetachedJournalExecutor350B journal_;
};

v2::DetachedTradeWorker& ServerTradeWorker350H() {
    static TradeExecutor350H executor;
    static v2::DetachedTradeWorker worker(executor,4);
    return worker;
}
std::atomic<bool>& TradeWorkerStarted350H() {
    static std::atomic<bool> started{false}; return started;
}
// Only the world/timer thread touches this list, with cSerSection held.
struct VerifiedCommitHold350H {
    v2::DetachedTradeSession session;
    std::string account,owner;
};
std::vector<VerifiedCommitHold350H>& VerifiedCommittedHolds350H() {
    static std::vector<VerifiedCommitHold350H> holds;
    return holds;
}
struct LivePending350Q {
    DetachedTradeJob job;
    std::vector<std::uint8_t> ownerAfter;
    v2::TradeSha256Digest expectedAfterInventory{};
};
std::vector<LivePending350Q>& PendingLive350Q() {
    static std::vector<LivePending350Q> pending;return pending;
}
static v2::BufferedTradeEnvelopeLayout LiveLayout350Q() {
    return v2::BufferedTradeEnvelopeLayout(
        offsetof(TRANS_RECORD_DATA,size),offsetof(TRANS_RECORD_DATA,ItemCount),
        offsetof(TRANS_RECORD_DATA,ItemSubStart),offsetof(TRANS_RECORD_DATA,DataSize),
        offsetof(TRANS_RECORD_DATA,smCharInfo)+offsetof(smCHAR_INFO,szName),
        sizeof(((smCHAR_INFO*)0)->szName),offsetof(TRANS_RECORD_DATA,Data),
        sizeof(TRANS_RECORD_DATA));
}
// Validate ALL index and sizes before the one inventory-only server cache rebase.
// No I/O, no legacy full-record restoration and no character-stat replacement.
static bool RebaseLivePlayer350Q(rsPLAYINFO* owner,const LivePending350Q& pending) {
    if(!owner || !owner->lpRecordDataBuff ||
       pending.ownerAfter.size()<offsetof(TRANS_RECORD_DATA,Data) ||
       pending.ownerAfter.size()>sizeof(TRANS_RECORD_DATA) ||
       pending.ownerAfter.size()>srRECORD_MEMORY_SIZE) return false;
    auto* cache=reinterpret_cast<TRANS_RECORD_DATA*>(owner->lpRecordDataBuff);
    const auto* after=reinterpret_cast<const TRANS_RECORD_DATA*>(pending.ownerAfter.data());
    if(cache->size<=0 || cache->size>srRECORD_MEMORY_SIZE ||
       after->size<=0 || static_cast<std::size_t>(after->size)>pending.ownerAfter.size() ||
       after->DataSize<0 || static_cast<std::size_t>(after->DataSize)>
           pending.ownerAfter.size()-offsetof(TRANS_RECORD_DATA,Data) ||
       cache->DataSize<0 || static_cast<std::size_t>(cache->DataSize)>
           static_cast<std::size_t>(srRECORD_MEMORY_SIZE)-offsetof(TRANS_RECORD_DATA,Data))
        return false;
    const auto& r=pending.job.request;
    if(!r.itemCode || !r.itemKey || !r.itemChecksum || (r.side!=0 && r.side!=1) ||
       !rsServerConfig.ServerInventoryMode) return false;
    int matching=-1,empty=-1;
    for(int n=0;n<INVEN_ITEM_INFO_MAX;++n) {
        const auto& entry=owner->InvenItemInfo[n];
        if(!entry.dwCode) empty=n;
        if(entry.dwCode==r.itemCode && entry.dwKey==r.itemKey &&
           entry.dwSum==r.itemChecksum) {
            if(matching!=-1) return false;
            matching=n;
        }
    }
    // Player->Companion needs exactly one registered identity. Reverse
    // direction needs exactly one free ledger position and no duplicate.
    if((r.side==0 && matching<0) || (r.side==1 && (matching>=0 || empty<0)))
        return false;
    if(r.side==0) owner->InvenItemInfo[matching].dwCode=0;
    else {
        auto& e=owner->InvenItemInfo[empty];
        e.dwCode=r.itemCode;e.dwKey=r.itemKey;e.dwSum=r.itemChecksum;
    }
    const std::size_t oldSize=static_cast<std::size_t>(cache->DataSize);
    const std::size_t newSize=static_cast<std::size_t>(after->DataSize);
    cache->ItemCount=after->ItemCount;
    cache->ItemSubStart=after->ItemSubStart;
    cache->DataSize=after->DataSize;
    cache->size=after->size;
    std::memcpy(cache->Data,after->Data,newSize);
    if(newSize<oldSize) std::memset(reinterpret_cast<unsigned char*>(cache->Data)+newSize,
                                 0,oldSize-newSize);
    return true;
}
void ReapCommittedDisconnected350H() {
    auto& holds=VerifiedCommittedHolds350H();
    for(auto it=holds.begin();it!=holds.end();) {
        rsPLAYINFO* online=FindUserFromName(const_cast<char*>(it->owner.c_str()));
        const bool same=online && online->lpsmSock &&
            online->dwObjectSerial==it->session.serial &&
            online->dwConnectCount==it->session.generation &&
            online->getAccount() && it->account==online->getAccount();
        if(same){++it;continue;}
        // Old or disconnected client can never receive a live commit image.
        auto& pending=PendingLive350Q();
        for(auto p=pending.begin();p!=pending.end();) {
            if(p->job.session.SameConnection(it->session)) p=pending.erase(p);
            else ++p;
        }
        // Only a verified worker COMMIT can appear here. The old session is
        // no longer connected, and its saved fence remains sealed. The pair
        // gate can now be released to permit a NEW login in this process.
        if(ServerTradeWorker350H().ReleaseAfterVerifiedRecovery(
               it->session.owner,it->session.companion)) {
            (void)v2::ServerTradeParticipantGate350B().ReleaseVerified(
                it->session.owner,it->session.companion);
            it=holds.erase(it);
        } else ++it;
    }
}
std::uint64_t TradeNonce350H(const v2::SaveSessionKey& key,std::uint64_t request) {
    static std::atomic<std::uint64_t> counter{1};
    FILETIME clock{};
    GetSystemTimeAsFileTime(&clock);
    const std::uint64_t time=(static_cast<std::uint64_t>(clock.dwHighDateTime)<<32)|clock.dwLowDateTime;
    std::uint64_t value=(static_cast<std::uint64_t>(key.serial)<<32)^
       key.generation^time^counter.fetch_add(1,std::memory_order_relaxed)^request;
    return value ? value : 1;
}
} // anonymous namespace SUMMIT350H

// WM_TIMER calls these OUTSIDE the world critical section; no IO in pump.
void StartDetachedTrade350H() {
    if(!Trade350HLabMode() || TradeWorkerStarted350H().load())return;
    if(ServerTradeWorker350H().Start())TradeWorkerStarted350H().store(true);
}
void StopDetachedTrade350H() {
    if(TradeWorkerStarted350H().exchange(false))ServerTradeWorker350H().Stop();
}
void PumpDetachedTrade350H() {
    if(!TradeWorkerStarted350H().load())return;
    // Resolve the current, authenticated session only AFTER entering cSerSection.
    EnterCriticalSection(&cSerSection);
    ReapCommittedDisconnected350H();
    for(int count=0;count<4;++count) {
        DetachedTradeJob job;
        int status=0;
        if(!ServerTradeWorker350H().TryTakeCompletion(job,status))break;
        std::printf("[SUMMIT350J_TRADE] stage=worker-result status=%d\n",status);
        if(status==2 || status==3) {
            VerifiedCommitHold350H hold;
            hold.session=job.session;
            hold.account=job.accountName;hold.owner=job.ownerName;
            VerifiedCommittedHolds350H().push_back(std::move(hold));
        }
        rsPLAYINFO* owner=FindUserFromName(const_cast<char*>(job.ownerName.c_str()));
        v2::ResolvedTradeSession350C live;
        live.session=job.session;
        live.account=job.accountName; live.owner=job.ownerName;
        live.companion=job.companionName;
        live.connected=owner && owner->lpsmSock &&
            owner->dwObjectSerial==job.session.serial &&
            owner->dwConnectCount==job.session.generation &&
            owner->getAccount() && job.accountName==owner->getAccount() &&
            owner->getName() && job.ownerName==owner->getName();
        live.staleSaveWritesSealed=v2::ServerSaveActivityFence().IsSealedForReconnect(
            v2::SaveSessionKey(job.session.serial,job.session.generation));
        const auto decision=v2::DecideReply350C(job,status,live);
        if(decision==v2::TradeReplyDecision350C::Drop)continue;
        protocol::CompanionTradeResultPacket result{};
        result.size=sizeof(result);
        result.code=protocol::OPCODE_COMPANION_TRADE_RESULT;
        result.version=protocol::COMPANION_TRADE_CONFIRM_VERSION;
        result.requestId=job.request.requestId;
        result.status=status;
        CopyText32(result.characterName,job.companionName.c_str());
        (void)SendOwnerPacket(owner,&result,result.size,result.code);
        std::vector<std::uint8_t> committed350Q;
        const bool imageAvailable350Q=(status==2 || status==3) &&
            LiveImages350Q().Take(job.tradeNonce,committed350Q);
        if(status==2 && Live350QEnabled() && live.connected && live.staleSaveWritesSealed) {
            v2::TradeSha256Digest before{},after{};
            const auto layout=LiveLayout350Q();
            if(imageAvailable350Q &&
               v2::TradeCanonicalInventoryDigest::Compute(job.acceptedOwnerRecord,layout,before) &&
               v2::TradeCanonicalInventoryDigest::Compute(committed350Q,layout,after) &&
               PendingLive350Q().size()<8) {
                LivePending350Q pending;
                pending.job=job;
                pending.ownerAfter=std::move(committed350Q);
                pending.expectedAfterInventory=after;
                const bool sent=v2::SendCommittedImage(pending.ownerAfter,
                    protocol::OPCODE_COMPANION_TRADE_DELIVERY,
                    protocol::COMPANION_TRADE_DELIVERY_VERSION,
                    job.request.requestId,job.tradeNonce,before,
                    job.ownerName.c_str(),job.companionName.c_str(),
                    [owner](const v2::TradeDeliveryChunk& chunk) {
                        return SendOwnerPacket(owner,&chunk,chunk.size,chunk.code).Succeeded();
                    });
                if(sent) {
                    PendingLive350Q().push_back(std::move(pending));
                    std::printf("[SUMMIT350Q_TRADE] stage=live-image-sent\n");
                }
            }
        }
    }
    ReapCommittedDisconnected350H();
    LeaveCriticalSection(&cSerSection);
}

// Called by the authenticated gameplay router under cSerSection. The client
// ACK by itself is NOT durable authority: the transaction had already been
// committed by the journal and its image independently read by the worker.
void HandleTradeApplyAck350Q(rsPLAYINFO* owner,
    const protocol::CompanionTradeApplyAckPacket* ack) {
    if(!owner || !owner->lpsmSock || !ack ||
       ack->size!=sizeof(*ack) ||
       ack->code!=protocol::OPCODE_COMPANION_TRADE_APPLY_ACK ||
       ack->version!=protocol::COMPANION_TRADE_DELIVERY_VERSION ||
       ack->status!=1 || !ack->requestId || !ack->txnId ||
       !std::memchr(ack->characterName,0,sizeof(ack->characterName))) return;
    auto& pending=PendingLive350Q();
    for(auto it=pending.begin();it!=pending.end();++it) {
        const auto& job=it->job;
        if(ack->requestId!=job.request.requestId || ack->txnId!=job.tradeNonce ||
           job.companionName!=ack->characterName ||
           owner->dwObjectSerial!=job.session.serial ||
           owner->dwConnectCount!=job.session.generation ||
           !owner->getAccount() || job.accountName!=owner->getAccount() ||
           !owner->getName() || job.ownerName!=owner->getName() ||
           !v2::ServerSaveActivityFence().IsSealedForReconnect(
               v2::SaveSessionKey(job.session.serial,job.session.generation))) continue;
        const v2::SaveSessionKey key(job.session.serial,job.session.generation);
        // First-save digest is armed BEFORE the reserved gate is released.
        // A stale client save then fails closed even if it arrives immediately.
        if(!v2::ServerTradeLiveFirstSave350Q().Arm(key,it->expectedAfterInventory))return;
        // SUMMIT351G: optional exact-equivalence proof from VERIFIED journaled
        // owner-after bytes. Failure leaves the original 350Q strict gate.
        // SUMMIT351H: the legacy inventory holds 100 entries per bag.
        // INVENTORY_MAXITEM is defined only by sinInvenTory.h, not in this
        // translation unit. The installer checks its authoritative value.
        const std::size_t slotsPerBag351H = 100u;
        const bool proof351G = v2::ServerTradePostSave351G().Arm<sRECORD_ITEM>(
            key,it->ownerAfter,LiveLayout350Q(),RECORD_ITEM_MAX,slotsPerBag351H);
        std::printf("[SUMMIT351G_TRADE] stage=postsave-proof-%s\n",
                    proof351G ? "armed" : "exact-only");
        if(!RebaseLivePlayer350Q(owner,*it)) {
            v2::ServerTradeLiveFirstSave350Q().Forget(key);
            v2::ServerTradePostSave351G().Forget(key); // SUMMIT351G
            std::printf("[SUMMIT350Q_TRADE] stage=live-rebase-refused\n");
            return; // keep old session sealed; relogin fallback
        }
        if(!v2::ServerSaveActivityFence().ReleaseAfterVerifiedLiveApply(key,job.leaseToken))
            return; // still sealed: no save may run
        const bool pairReleased=v2::ServerTradeParticipantGate350B().ReleaseVerified(
            job.session.owner,job.session.companion);
        const bool durableReleased=ServerTradeWorker350H().ReleaseAfterVerifiedRecovery(
            job.session.owner,job.session.companion);
        if(!pairReleased || !durableReleased) return; // keep client frozen
        auto& holds=VerifiedCommittedHolds350H();
        for(auto h=holds.begin();h!=holds.end();) {
            if(h->session.SameConnection(job.session)) h=holds.erase(h);
            else ++h;
        }
        protocol::CompanionTradeResultPacket complete{};
        complete.size=sizeof(complete);
        complete.code=protocol::OPCODE_COMPANION_TRADE_RESULT;
        complete.version=protocol::COMPANION_TRADE_CONFIRM_VERSION;
        complete.requestId=job.request.requestId;
        complete.status=1; // LIVE: first post-apply save must match after-image
        CopyText32(complete.characterName,job.companionName.c_str());
        (void)SendOwnerPacket(owner,&complete,complete.size,complete.code);
        pending.erase(it);
        std::printf("[SUMMIT350Q_TRADE] stage=live-rebased\n");
        return;
    }
}

// SUMMIT 350A: fail-closed boundary before the asynchronous rewrite.
//
// RecvMessage routes this command while holding cSerSection. The legacy
// synchronous transaction (347-349) performed filesystem I/O, journal
// recovery, a fresh DAT save and a second legacy lock acquisition on this
// path. The world tick shares the lock. It MUST NOT run here.
//
// This intermediate patch intentionally DOES NOT claim successful transfer:
// no journal and no DAT are even opened by this handler. Until the fully
// detached session snapshot, persistence worker, login gate and safe result
// pump have been compiled and tested together, -76 is a definitive rejection.
// The client recognises negative replies and unfreezes its inventory.
void HandleTradeConfirm(rsPLAYINFO* owner,
    const protocol::CompanionTradeConfirmPacket* request)
{
    if(!owner || !owner->lpsmSock || !request ||
       !v2::ValidTradeConfirmRequest(*request))return;
    // Experimental engine only with disposable DATs; default production
    // remains -76 until MinGW, Wine, failpoint and recovery checks pass.
    bool admitted=false;
    int admissionStage=0;
    // SUMMIT350K: independently distinguish immutable-capture refusal from
    // the pair gate, rather than showing checkpoint=4 for both conditions.
    int captureRejection350K=0;
    int gateRejection350K=0;
    if(Trade350HLabMode() && TradeWorkerStarted350H().load() &&
       owner->getAccount() && owner->getAccount()[0] &&
       owner->getName() && owner->getName()[0] &&
       owner->lpRecordDataBuff && !owner->szServerID[0]) {
        const std::string account(owner->getAccount());
        const std::string playerName(owner->getName());
        const std::string companionName(request->characterName);
        if(account.size()<=31 && playerName.size()<=31 &&
           companionName.size()<=31 && playerName!=companionName &&
           rsCheckAccountChar(owner->getAccount(),const_cast<char*>(companionName.c_str())) &&
           !IsCharacterActive(owner,companionName.c_str()) &&
           (!FindUserFromName(const_cast<char*>(companionName.c_str())) ||
           !FindUserFromName(const_cast<char*>(companionName.c_str()))->lpsmSock)) {
            const v2::SaveSessionKey session(
                static_cast<std::uint32_t>(owner->dwObjectSerial),
                static_cast<std::uint32_t>(owner->dwConnectCount));
            // A previous committed live delivery cannot be traded again until
            // its first post-apply save has matched the durable inventory.
            std::uint64_t token=0;
            if(!v2::ServerTradeLiveFirstSave350Q().Pending(session) &&
               v2::ServerSaveActivityFence().TryReserve(session,token)) {
                do {
                    admissionStage=1;
                    std::uint64_t sequence=0;
                    v2::AcceptedRecordEvidence350E evidence;
                    if(!v2::ServerSaveActivityFence().TryLeasedAcceptedSaveSequence(
                            session,token,sequence) || !sequence ||
                       !v2::ServerAcceptedRecordBridge350E().Copy(session,evidence) ||
                       evidence.bytes.size()!=evidence.exactWriteBytes)break;
                    admissionStage=2;
                    // SUMMIT350J: an accepted client save may still be only
                    // in the legacy server's memory buffer when ACK arrives.
                    // Produce its exact normalised disk image from an OWNED
                    // copy. These two short adapters only read the authenticated
                    // live player's potion/effect fields; no DAT or journal IO.
                    const v2::TradeLegacyRecordLayout350D layout={
                        offsetof(TRANS_RECORD_DATA,size),srRECORD_DEFAULT_SIZE,
                        offsetof(TRANS_RECORD_DATA,GameSaveInfo),sizeof(sGAME_SAVE_INFO),
                        offsetof(TRANS_RECORD_DATA,Data)};
                    v2::TradeNormalizedRecord350D normalized;
                    const auto norm=v2::BuildNormalizedRecord350D(
                        evidence.bytes.data(),evidence.bytes.size(),layout,
                        [owner](std::uint8_t* dst,std::size_t length)->bool {
                            if(length!=sizeof(sGAME_SAVE_INFO)) return false;
                            sGAME_SAVE_INFO save{};
                            std::memcpy(&save,dst,sizeof(save));
                            if(rsSaveServerPotion(owner,&save)!=TRUE ||
                               rsSaveServerForce(owner,&save)!=TRUE) return false;
                            std::memcpy(dst,&save,sizeof(save));
                            return true;
                        },normalized);
                    if(norm!=v2::TradeRecordNormalizeCode350D::Ready ||
                       normalized.exactWriteBytes!=evidence.exactWriteBytes)
                        break;
                    admissionStage=3;
                    const PlayerHandle live=player::legacy::ServerLegacyOnlinePlayerRegistry().HandleOf(owner);
                    OnlineTradeIdentitySnapshot online;
                    if(!live.Valid() || live.id.value!=session.serial ||
                       live.generation!=session.generation ||
                       !inventory::legacy::ServerLegacyInventoryRuntime()
                          .TryGetTradeIdentitySnapshot(live,online) ||
                       online.revision!=request->ownerRevision)break;
                    char a[256]={},b[256]={};
                    if(!GetUserDataFile(playerName.c_str(),a) ||
                       !GetUserDataFile(companionName.c_str(),b) ||
                       !a[0] || !b[0] || lstrcmpiA(a,b)==0)break;
                    admissionStage=4;
                    v2::AcceptedTradeSource350C source;
                    source.session.serial=session.serial;
                    source.session.generation=session.generation;
                    source.session.account=v2::TradeParticipantId(account.c_str(),
                        "__summit_account_350h__");
                    source.session.owner=v2::TradeParticipantId(account.c_str(),playerName.c_str());
                    source.session.companion=v2::TradeParticipantId(account.c_str(),companionName.c_str());
                    source.account=account;source.owner=playerName;
                    source.companion=companionName;
                    source.acceptedSaveSequence=sequence;source.leaseToken=token;
                    source.tradeNonce=TradeNonce350H(session,request->requestId);
                    source.completeRecord=normalized.bytes.data();
                    source.allocatedRecordBytes=normalized.bytes.size();
                    source.acknowledgedRecordBytes=normalized.bytes.size();
                    source.acknowledgedDigest=normalized.digest;
                    DetachedTradeJob job;
                    const auto captured=v2::CaptureAcceptedTrade350C(source,*request,job);
                    if(captured!=v2::TradeCaptureCode350C::Captured) {
                        captureRejection350K=static_cast<int>(captured);
                        break;
                    }
                    job.trustedOwnerPath=a;
                    job.trustedCompanionPath=b;
                    v2::TradeParticipantGate350B::ReservationBlock350K blocked=
                        v2::TradeParticipantGate350B::ReservationBlock350K::None;
                    if(!v2::ServerTradeParticipantGate350B().TryReserve(
                         job.session.owner,job.session.companion,&blocked)) {
                        gateRejection350K=static_cast<int>(blocked);
                        break;
                    }
                    admissionStage=5;
                    const auto enqueue=ServerTradeWorker350H().Enqueue(job);
                    if(enqueue==v2::DetachedAdmission::Accepted) {
                        admitted=true;
                        std::printf("[SUMMIT350J_TRADE] stage=queued\n");
                    } else {
                        (void)v2::ServerTradeParticipantGate350B().ReleaseVerified(
                             job.session.owner,job.session.companion);
                    }
                } while(false);
                if(!admitted)(void)v2::ServerSaveActivityFence().ReleaseReservation(session,token);
            }
        }
    }
    if(admitted)return; // outcome ONLY from timer pump after durable journal processing
    std::printf("[SUMMIT350K_TRADE] stage=admission-refused checkpoint=%d capture=%d gate=%d\n",
                admissionStage,captureRejection350K,gateRejection350K);
    protocol::CompanionTradeResultPacket result{};
    result.size=sizeof(result);
    result.code=protocol::OPCODE_COMPANION_TRADE_RESULT;
    result.version=protocol::COMPANION_TRADE_CONFIRM_VERSION;
    result.requestId=request->requestId;
    result.status=-76;
    CopyText32(result.characterName,request->characterName);
    (void)SendOwnerPacket(owner,&result,result.size,result.code);
}

void HandleUIAction(rsPLAYINFO *owner, const protocol::CompanionUIActionPacket *packet)
{
    if(!owner || !packet ||
        packet->size != sizeof(protocol::CompanionUIActionPacket) ||
        packet->code != protocol::OPCODE_COMPANION_UI_ACTION ||
        packet->version != protocol::COMPANION_UI_PROTOCOL_VERSION)
    {
        summit::companion::diagnostic::Trace("SERVER","malformed-ui-packet",0,0,-104);
        return;
    }

    summit::companion::diagnostic::Trace("SERVER","received-ui-action",packet->action,
        packet->value,0);
    // New trade pages require a terminated, explicit owned character name.
    if(packet->action == protocol::COMPANION_UI_TRADE_PAGE &&
       (!packet->characterName[0] ||
        !memchr(packet->characterName, 0, sizeof(packet->characterName))))
        return;
    int result = 0;
    char actionName[32] = { 0 };
    const char *name = ResolveActionName(owner, packet, actionName);

    switch(packet->action)
    {
        case protocol::COMPANION_UI_REFRESH:
            result = 0;
            break;

        case protocol::COMPANION_UI_EQUIP_INVENTORY_RIGHT:
            result = name[0] ? EquipInventoryItem(owner, name, true, packet->value) : -8;
            break;
        case protocol::COMPANION_UI_EQUIP_INVENTORY_LEFT:
            result = name[0] ? EquipInventoryItem(owner, name, false, packet->value) : -8;
            break;

        case protocol::COMPANION_UI_SET_INVENTORY_SLOT:
            {
                const int slot = (static_cast<unsigned>(packet->value) >> 16) & 0xFF;
                const int encodedIndex = packet->value & 0xFFFF;
                const int itemIndex = encodedIndex == 0xFFFF ? -1 : encodedIndex;
                if(!name[0] || !owner->getAccount() || !owner->getAccount()[0] ||
                   (owner->getName() && lstrcmpiA(owner->getName(), name) == 0) ||
                   !rsCheckAccountChar(owner->getAccount(), (char *)name))
                    result = -2;
                else if (FindUserFromName((char *)name) &&
                         FindUserFromName((char *)name)->lpsmSock)
                    result = -4;
                else if (IsCharacterActive(owner, name))
                    result = -12; // dismiss companion before editing the live DAT
                else
                {
                    summit::companion::diagnostic::Trace("SERVER","dat-write-request",
                        packet->action,itemIndex,0,slot);
                    const int writeResult = rsSetOfflineCompanionEquipmentSlot(
                        owner->getAccount(), name, itemIndex, slot);
                    summit::companion::diagnostic::Trace("SERVER","dat-write-result",
                        packet->action,itemIndex,writeResult,slot);
                    result = writeResult == TRUE ? TRUE :
                        (writeResult < 0 ? writeResult : -13);
                    if(result == TRUE)
                        SendInventoryPage(owner, name, 0);
                }
            }
            break;

        case protocol::COMPANION_UI_MOVE_BAG:
        case protocol::COMPANION_UI_ORGANIZE_BAG:
        case protocol::COMPANION_UI_MOVE_BAG_CELL:
            {
                const bool manual=packet->action==protocol::COMPANION_UI_MOVE_BAG_CELL;
                const int targetBag=manual ?
                    ((static_cast<unsigned>(packet->value)>>15)&3) :
                    (packet->action==protocol::COMPANION_UI_MOVE_BAG ?
                    ((static_cast<unsigned>(packet->value)>>16)&0xffff) : packet->value);
                const int index=manual ? (packet->value&255) :
                    (packet->action==protocol::COMPANION_UI_MOVE_BAG ?
                    (packet->value&0xffff) : -1);
                const int cellX=manual ? ((static_cast<unsigned>(packet->value)>>8)&15) : -1;
                const int cellY=manual ? ((static_cast<unsigned>(packet->value)>>12)&7) : -1;
                if(!name[0] || !owner->getAccount() || !owner->getAccount()[0] ||
                   (owner->getName() && lstrcmpiA(owner->getName(),name)==0) ||
                   !rsCheckAccountChar(owner->getAccount(),(char*)name)) result=-2;
                else if((FindUserFromName((char*)name) &&
                        FindUserFromName((char*)name)->lpsmSock) || IsCharacterActive(owner,name))
                    result=-12;
                else if(targetBag!=1 && targetBag!=2) result=-14;
                else {
                    result=rsMutateOfflineCompanionBag(owner->getAccount(),name,index,targetBag,cellX,cellY);
                    if(result==TRUE) SendInventoryPage(owner,name,0);
                }
            }
            break;

        case protocol::COMPANION_UI_TRADE_PAGE:
            if(name[0] && packet->value >= 0 &&
               (packet->value >> 16) <= 1 && (packet->value & 0xFFFF) <= 1000)
            {
                SendTradePreviewPage(owner, name, packet->value >> 16,
                                     packet->value & 0xFFFF);
                result = 0;
            }
            else result = -10;
            break;

        case protocol::COMPANION_UI_INVENTORY_PAGE:
            if (name[0] && packet->value >= 0 && packet->value <= 1000)
            {
                SendInventoryPage(owner, name, packet->value);
                result = 0;
            }
            else result = -10;
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

    summit::companion::diagnostic::Trace("SERVER","completed-ui-action",packet->action,
        packet->value,result);
    SendUIState(owner, result);
}

} // namespace companions
} // namespace gameplay
