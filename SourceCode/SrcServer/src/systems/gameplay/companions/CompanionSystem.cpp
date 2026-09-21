#include "systems/gameplay/companions/CompanionSystem.h"
#include "systems/gameplay/companions/CompanionProfileStore.h"
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

struct CompanionSlot
{
    CompanionSlot()
        : entity(NULL), cachedLevel(0), changeJob(0),
          cachedExp(0), pendingExp(0), lastExpFlushTime(0),
          retreatLatched(false), retreatUntil(0), deadSince(0),
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
    DWORD visualMotionState;
    DWORD visualMotionSequence;
    int visualMotionFrame;
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
          retreatHpPercent(0), preferredRange(0), formationSlot(-1)
    {
        characterName[0] = 0;
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
    int seconds = GetPrivateProfileIntA(
        "Companion", "ExpFlushSeconds", 30, ".//Configs//Companion.ini");
    if(seconds < 5) seconds = 5;
    if(seconds > 300) seconds = 300;
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
    slot.visualMotionState = 0;
    slot.visualMotionSequence = 0;
    slot.visualMotionFrame = 0;
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
        slot.entity->smCharInfo.Level = newLevel;
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
    static const DWORD kDeadVisualHoldMs = 3000U;
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

void EngageTarget(smCHAR *character, smCHAR *target, int preferredRange)
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

    if(distance >= rangeSquared)
    {
        character->SetTargetPosi2(target->pX >> FLOATNS, target->pZ >> FLOATNS);
        character->TragetTraceMode = TRUE;
        // We drive the trace every AI tick, so do not also arm the legacy
        // deferred ptNextTarget state machine. It can restart STAND/RUN on
        // animation boundaries and causes visible animation popping.
        character->SetNextTarget(0, 0, 0);

        if(character->MotionInfo &&
            character->MotionInfo->State != CHRMOTION_STATE_WALK &&
            character->MotionInfo->State != CHRMOTION_STATE_RUN &&
            character->MotionInfo->State != CHRMOTION_STATE_DAMAGE &&
            character->MotionInfo->State != CHRMOTION_STATE_DEAD &&
            character->MotionInfo->State != CHRMOTION_STATE_ATTACK &&
            character->MotionInfo->State != CHRMOTION_STATE_SKILL)
        {
            if(!character->SetMotionFromCode(CHRMOTION_STATE_RUN))
                character->SetMotionFromCode(CHRMOTION_STATE_WALK);
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

void CopyText32(char (&dest)[32], const char *source)
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
        }
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
        entry.casting = slot.entity->MotionInfo &&
            slot.entity->MotionInfo->State == CHRMOTION_STATE_SKILL ? TRUE : FALSE;

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
    slot.visualMotionState = 0;
    slot.visualMotionSequence = 0;
    slot.visualMotionFrame = 0;

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

    // Companion V1 has no heal/recovery executor yet. A permanent low-HP
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
                if(!character->SetMotionFromCode(CHRMOTION_STATE_RUN))
                    character->SetMotionFromCode(CHRMOTION_STATE_WALK);
            }
            return;
        }
    }

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
                target = FindNearestHostile(character);
            break;

        case COMPANION_TACTIC_DEFENSIVE:
            // Retaliate first; otherwise keep participating in normal combat.
            if(currentTarget &&
                (currentTarget->lpTargetChar == character || currentTarget->lpTargetPlayInfo == owner))
                target = currentTarget;
            else if(ownerTarget)
                target = ownerTarget;
            else if(currentTarget)
                target = currentTarget;
            else
                target = FindNearestHostile(character);
            break;

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
            else if(currentTarget)
                target = currentTarget;
            else
                target = FindNearestHostile(character);
            break;
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
        CompanionDebugLog("AI name='%s' serial=%lu mode=%d tactic=%d life=%d/%d motion=%d ownerTarget=%lu currentTarget=%lu chosen=%lu dist2=%d preferredRange=%d",
            managedSlot->characterName, (unsigned long)character->dwObjectSerial,
            (int)mode, (int)tactic, character->smCharInfo.Life[0], character->smCharInfo.Life[1],
            character->MotionInfo ? (int)character->MotionInfo->State : -1,
            ownerTarget ? (unsigned long)ownerTarget->dwObjectSerial : 0UL,
            currentTarget ? (unsigned long)currentTarget->dwObjectSerial : 0UL,
            target ? (unsigned long)target->dwObjectSerial : 0UL, dist2, preferredRange);
    }

    if(target)
        EngageTarget(character, target, preferredRange);
    else
        DisengageTarget(character);
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
    return (DWORD)GetPrivateProfileIntA(
        "Companion",
        "CrystalItemCode",
        0,
        ".//Configs//Companion.ini");
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
