#pragma once

#include "authentication/v2/CharacterSelectionService.h"
#include "systems/persistence/character_record/record.h"

namespace authentication::v2::legacy
{
class LegacyCharacterOwnershipRepository final : public ICharacterOwnershipRepository
{
public:
    bool OwnsCharacter(const char* accountId, const char* characterName) const override
    {
        return rsCheckAccountChar(accountId, const_cast<char*>(characterName)) != FALSE;
    }
};

inline const LegacyCharacterOwnershipRepository& ServerLegacyCharacterOwnershipRepository()
{
    static const LegacyCharacterOwnershipRepository repository;
    return repository;
}
}
