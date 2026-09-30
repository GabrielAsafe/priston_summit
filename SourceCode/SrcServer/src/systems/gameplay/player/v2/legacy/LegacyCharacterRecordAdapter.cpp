#include "systems/gameplay/player/v2/legacy/LegacyCharacterRecordAdapter.h"

#include <algorithm>

// Keep the historical server include order inside the AMBER boundary.
#include "systems/network/transport/legacy/smwsock.h"
#include "systems/rendering/legacy_3d/smd3d.h"
#include "systems/gameplay/character/core/character.h"
#include "systems/persistence/character_record/record.h"
#include "core/gameplay/legacy_support/playsub.h"

namespace gameplay
{
namespace player
{
namespace legacy
{
namespace
{
bool LoadLegacyRecord(const CharacterRecordKey& key, TRANS_RECORD_DATA& record)
{
    return key.Valid() &&
        rsLoadAccountCharacterRecord(key.accountKey.c_str(), key.characterName.c_str(), &record) != 0;
}

CharacterProgressionRecord TranslateProgression(const TRANS_RECORD_DATA& record)
{
    CharacterProgressionRecord snapshot;
    snapshot.statePoint = record.smCharInfo.StatePoint;
    snapshot.nextExperience = record.smCharInfo.Next_Exp;
    snapshot.changeJob = static_cast<std::uint32_t>(record.smCharInfo.ChangeJob);
    snapshot.jobBitMask = static_cast<std::uint32_t>(record.smCharInfo.JobBitMask);
    return snapshot;
}

void TranslateSkillPoints(const TRANS_RECORD_DATA& record, std::vector<std::uint8_t>& out)
{
    out.assign(20u, 0u);
    for (int useSlot = 1; useSlot <= 16; ++useSlot)
        out[static_cast<std::size_t>(useSlot - 1)] = static_cast<std::uint8_t>(
            std::min(10, static_cast<int>(record.GameSaveInfo.SkillInfo.bSkillPoint[useSlot & 15])));

    for (int tier5Index = 0; tier5Index < 4; ++tier5Index)
        out[static_cast<std::size_t>(16 + tier5Index)] = static_cast<std::uint8_t>(
            std::min(10, GetTier5RecordSkillPoint(&record.GameSaveInfo, tier5Index)));
}

void TranslatePrimaryItems(const TRANS_RECORD_DATA& record, std::vector<CharacterRecordItem>& out)
{
    out.clear();
    if (record.DataSize <= 0)
        return;

    const BYTE* cursor = record.Data;
    const BYTE* end = record.Data + record.DataSize;
    int primaryItemCount = record.ItemSubStart;
    if (primaryItemCount <= 0 || primaryItemCount > record.ItemCount)
        primaryItemCount = record.ItemCount;

    out.reserve(static_cast<std::size_t>(std::max(0, primaryItemCount)));
    for (int item = 0; item < primaryItemCount && cursor + sizeof(int) <= end; ++item)
    {
        const int packedSize = *reinterpret_cast<const int*>(cursor);
        if (packedSize <= static_cast<int>(sizeof(int)) || cursor + packedSize > end)
            break;

        sRECORD_ITEM legacyItem;
        ZeroMemory(&legacyItem, sizeof(legacyItem));
        DecodeCompress(const_cast<BYTE*>(cursor), reinterpret_cast<BYTE*>(&legacyItem), sizeof(legacyItem));
        cursor += packedSize;

        if (!legacyItem.sItemInfo.CODE)
            continue;

        CharacterRecordItem translated;
        translated.itemCode = static_cast<std::uint32_t>(legacyItem.sItemInfo.CODE);
        translated.position = legacyItem.ItemPosition;
        translated.itemName = legacyItem.sItemInfo.ItemName;
        out.push_back(translated);
    }
}
}

bool LegacyCharacterRecordAdapter::TryLoadProgression(
    const CharacterRecordKey& key,
    CharacterProgressionRecord& out) const
{
    TRANS_RECORD_DATA record;
    if (!LoadLegacyRecord(key, record))
        return false;
    out = TranslateProgression(record);
    return true;
}

bool LegacyCharacterRecordAdapter::TryLoadSnapshot(
    const CharacterRecordKey& key,
    CharacterPersistentSnapshot& out) const
{
    TRANS_RECORD_DATA record;
    if (!LoadLegacyRecord(key, record))
        return false;

    CharacterPersistentSnapshot snapshot;
    snapshot.characterName = record.smCharInfo.szName;
    snapshot.level = record.smCharInfo.Level;
    snapshot.legacyJobCode = static_cast<int>(record.smCharInfo.JOB_CODE);
    snapshot.life = record.smCharInfo.Life[0];
    snapshot.maxLife = record.smCharInfo.Life[1];
    snapshot.mana = record.smCharInfo.Mana[0];
    snapshot.maxMana = record.smCharInfo.Mana[1];
    snapshot.progression = TranslateProgression(record);
    TranslateSkillPoints(record, snapshot.logicalSkillPoints);
    TranslatePrimaryItems(record, snapshot.items);
    out = snapshot;
    return true;
}

const LegacyCharacterRecordAdapter& ServerLegacyCharacterRecordAdapter()
{
    static LegacyCharacterRecordAdapter adapter;
    return adapter;
}
}
}
}
