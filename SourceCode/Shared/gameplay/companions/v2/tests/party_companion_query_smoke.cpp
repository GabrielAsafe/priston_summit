#include "gameplay/companions/v2/PartyCompanionQuery.h"

#include <cassert>
#include <iostream>
#include <map>

namespace
{
class FakePartyQuery final : public gameplay::social::party::IPartyQuery
{
public:
    bool TryGetParty(gameplay::PlayerId, gameplay::social::party::PartySnapshot& out) const override
    {
        out = party;
        return available;
    }
    bool available = true;
    gameplay::social::party::PartySnapshot party;
};

class FakeCompanionQuery final : public gameplay::companions::ICompanionQuery
{
public:
    std::uint32_t ActiveCount(gameplay::PlayerId owner) const override
    {
        const auto found = counts.find(owner.value);
        return found == counts.end() ? 0u : found->second;
    }
    bool TryGetState(gameplay::PlayerId owner, gameplay::companions::CompanionStateSnapshot& out) const override
    {
        out = gameplay::companions::CompanionStateSnapshot();
        out.activeCount = ActiveCount(owner);
        return owner.Valid();
    }
    std::uint32_t ConfiguredCrystalItemCode() const override { return 1234u; }
    std::map<std::uint32_t, std::uint32_t> counts;
};
}

int main()
{
    using namespace gameplay;
    FakePartyQuery party;
    party.party.leader = PlayerId(1u);
    party.party.members.push_back(social::party::PartyMemberSnapshot(PlayerId(1u), MapId(5u), true));
    party.party.members.push_back(social::party::PartyMemberSnapshot(PlayerId(2u), MapId(5u), true));
    party.party.members.push_back(social::party::PartyMemberSnapshot(PlayerId(3u), MapId(6u), true));
    party.party.members.push_back(social::party::PartyMemberSnapshot(PlayerId(4u), MapId(5u), false));

    FakeCompanionQuery companions;
    companions.counts[1u] = 2u;
    companions.counts[2u] = 3u;
    companions.counts[3u] = 4u;
    companions.counts[4u] = 4u;

    assert(companions::ActivePartyCompanionsOnMap(party, companions, PlayerId(1u), MapId(5u)) == 5u);
    assert(companions::ActivePartyCompanionsOnMap(party, companions, PlayerId(1u), MapId(6u)) == 4u);
    party.available = false;
    assert(companions::ActivePartyCompanionsOnMap(party, companions, PlayerId(1u), MapId(5u)) == 0u);
    std::cout << "party companion query smoke: PASS\n";
    return 0;
}
