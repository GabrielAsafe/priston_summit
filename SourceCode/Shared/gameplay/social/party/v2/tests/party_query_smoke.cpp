#include "gameplay/social/party/v2/PartyQuery.h"

#include <cassert>
#include <iostream>
#include <map>

namespace
{
class FakePartyQuery final : public gameplay::social::party::IPartyQuery
{
public:
    bool TryGetParty(gameplay::PlayerId member, gameplay::social::party::PartySnapshot& out) const override
    {
        const auto found = parties.find(member.value);
        if(found == parties.end()) return false;
        out = found->second;
        return true;
    }
    std::map<std::uint32_t, gameplay::social::party::PartySnapshot> parties;
};
}

int main()
{
    using namespace gameplay;
    using namespace gameplay::social::party;
    FakePartyQuery query;
    PartySnapshot party;
    party.leader = PlayerId(10u);
    party.members.push_back(PartyMemberSnapshot(PlayerId(10u), MapId(3u), true));
    party.members.push_back(PartyMemberSnapshot(PlayerId(11u), MapId(3u), true));
    query.parties.emplace(10u, party);
    query.parties.emplace(11u, party);
    PartySnapshot out;
    assert(query.TryGetParty(PlayerId(11u), out));
    assert(out.leader == PlayerId(10u));
    assert(out.members.size() == 2u);
    assert(!query.TryGetParty(PlayerId(99u), out));
    std::cout << "party query smoke: PASS\n";
    return 0;
}
