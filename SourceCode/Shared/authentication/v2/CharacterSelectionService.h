#pragma once

namespace authentication::v2
{
class ICharacterOwnershipRepository
{
public:
    virtual ~ICharacterOwnershipRepository() = default;
    virtual bool OwnsCharacter(const char* accountId, const char* characterName) const = 0;
};

enum class CharacterSelectionDecision
{
    Allowed,
    MissingIdentity,
    CharacterNotOwned
};

class CharacterSelectionService
{
public:
    CharacterSelectionDecision Authorize(const char* accountId,
                                         const char* characterName,
                                         bool privilegedOwnershipBypass,
                                         const ICharacterOwnershipRepository& ownership) const
    {
        if (!accountId || !accountId[0] || !characterName || !characterName[0])
            return CharacterSelectionDecision::MissingIdentity;
        if (privilegedOwnershipBypass)
            return CharacterSelectionDecision::Allowed;
        return ownership.OwnsCharacter(accountId, characterName)
            ? CharacterSelectionDecision::Allowed
            : CharacterSelectionDecision::CharacterNotOwned;
    }
};
}
