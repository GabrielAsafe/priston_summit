#include "Shared/authentication/v2/CharacterSelectionService.h"
#include <cassert>
#include <cstring>
#include <iostream>

class FakeOwnership final : public authentication::v2::ICharacterOwnershipRepository
{
public:
    bool OwnsCharacter(const char* accountId, const char* characterName) const override
    {
        return std::strcmp(accountId, "account") == 0 && std::strcmp(characterName, "hero") == 0;
    }
};

int main()
{
    authentication::v2::CharacterSelectionService service;
    FakeOwnership ownership;
    using D = authentication::v2::CharacterSelectionDecision;

    assert(service.Authorize("account", "hero", false, ownership) == D::Allowed);
    assert(service.Authorize("account", "other", false, ownership) == D::CharacterNotOwned);
    assert(service.Authorize("account", "other", true, ownership) == D::Allowed);
    assert(service.Authorize("", "hero", false, ownership) == D::MissingIdentity);
    assert(service.Authorize("account", "", false, ownership) == D::MissingIdentity);

    std::cout << "character selection service smoke: PASS\n";
    return 0;
}
