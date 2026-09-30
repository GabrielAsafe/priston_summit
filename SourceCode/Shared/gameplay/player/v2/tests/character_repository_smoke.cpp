#include <cassert>
#include <map>
#include <string>

#include "gameplay/player/v2/ICharacterRepository.h"

class FakeCharacterRepository final : public gameplay::player::ICharacterRepository
{
public:
    bool TryLoadProgression(const gameplay::player::CharacterRecordKey& key,
                            gameplay::player::CharacterProgressionRecord& out) const override
    {
        if (!key.Valid())
            return false;
        const auto it = records.find(key.accountKey + ":" + key.characterName);
        if (it == records.end())
            return false;
        out = it->second;
        return true;
    }

    bool TryLoadSnapshot(const gameplay::player::CharacterRecordKey& key,
                         gameplay::player::CharacterPersistentSnapshot& out) const override
    {
        if (!key.Valid())
            return false;
        const auto it = snapshots.find(key.accountKey + ":" + key.characterName);
        if (it == snapshots.end())
            return false;
        out = it->second;
        return true;
    }

    std::map<std::string, gameplay::player::CharacterProgressionRecord> records;
    std::map<std::string, gameplay::player::CharacterPersistentSnapshot> snapshots;
};

int main()
{
    FakeCharacterRepository repo;
    gameplay::player::CharacterProgressionRecord stored;
    stored.statePoint = 17;
    stored.nextExperience = 9000;
    stored.changeJob = 3u;
    stored.jobBitMask = 0x20u;
    repo.records["account:Companion"] = stored;

    gameplay::player::CharacterProgressionRecord loaded;
    assert(repo.TryLoadProgression(
        gameplay::player::CharacterRecordKey("account", "Companion"), loaded));
    assert(loaded.statePoint == 17);
    assert(loaded.nextExperience == 9000);
    assert(loaded.changeJob == 3u);
    assert(loaded.jobBitMask == 0x20u);
    assert(!repo.TryLoadProgression(gameplay::player::CharacterRecordKey(), loaded));

    gameplay::player::CharacterPersistentSnapshot persisted;
    persisted.characterName = "Companion";
    persisted.level = 80;
    persisted.legacyJobCode = 2;
    persisted.life = 500;
    persisted.maxLife = 700;
    persisted.logicalSkillPoints = {1u, 2u, 3u};
    gameplay::player::CharacterRecordItem item;
    item.itemCode = 0x01020304u;
    item.position = 1;
    item.itemName = "Sword";
    persisted.items.push_back(item);
    repo.snapshots["account:Companion"] = persisted;
    gameplay::player::CharacterPersistentSnapshot loadedSnapshot;
    assert(repo.TryLoadSnapshot(gameplay::player::CharacterRecordKey("account", "Companion"), loadedSnapshot));
    assert(loadedSnapshot.characterName == "Companion");
    assert(loadedSnapshot.logicalSkillPoints.size() == 3u);
    assert(loadedSnapshot.items.size() == 1u && loadedSnapshot.items[0].position == 1);
    return 0;
}
