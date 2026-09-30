#pragma once

#include "gameplay/player/v2/ICharacterRepository.h"

namespace gameplay
{
namespace player
{
namespace legacy
{
class LegacyCharacterRecordAdapter final : public ICharacterRepository
{
public:
    bool TryLoadProgression(const CharacterRecordKey& key,
                            CharacterProgressionRecord& out) const override;
    bool TryLoadSnapshot(const CharacterRecordKey& key,
                         CharacterPersistentSnapshot& out) const override;
};

const LegacyCharacterRecordAdapter& ServerLegacyCharacterRecordAdapter();
}
}
}
