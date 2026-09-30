#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace gameplay
{
namespace player
{
struct CharacterRecordKey
{
    std::string accountKey;
    std::string characterName;

    CharacterRecordKey() {}
    CharacterRecordKey(const std::string& accountValue, const std::string& characterValue)
        : accountKey(accountValue), characterName(characterValue) {}

    bool Valid() const { return !accountKey.empty() && !characterName.empty(); }
};

struct CharacterProgressionRecord
{
    int statePoint;
    int nextExperience;
    std::uint32_t changeJob;
    std::uint32_t jobBitMask;

    CharacterProgressionRecord()
        : statePoint(0), nextExperience(0), changeJob(0u), jobBitMask(0u) {}
};

struct CharacterRecordItem
{
    std::uint32_t itemCode;
    int position;
    std::string itemName;

    CharacterRecordItem() : itemCode(0u), position(0) {}
};

struct CharacterPersistentSnapshot
{
    std::string characterName;
    int level;
    int legacyJobCode;
    int life;
    int maxLife;
    int mana;
    int maxMana;
    CharacterProgressionRecord progression;
    std::vector<std::uint8_t> logicalSkillPoints;
    std::vector<CharacterRecordItem> items;

    CharacterPersistentSnapshot()
        : level(0), legacyJobCode(0), life(0), maxLife(0), mana(0), maxMana(0) {}
};

class ICharacterRepository
{
public:
    virtual ~ICharacterRepository() {}
    virtual bool TryLoadProgression(const CharacterRecordKey& key,
                                    CharacterProgressionRecord& out) const = 0;
    virtual bool TryLoadSnapshot(const CharacterRecordKey& key,
                                 CharacterPersistentSnapshot& out) const = 0;
};
}
}
