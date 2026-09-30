#include <cassert>

#include "gameplay/social/party/v2/PartyCommands.h"

namespace
{
class FakePartyCommands final : public gameplay::social::party::IPartyCommands
{
public:
    mutable int calls;
    FakePartyCommands() : calls(0) {}

    bool Join(gameplay::PlayerId requester, gameplay::PlayerId target) const override
    {
        ++calls;
        return requester.Valid() && target.Valid() && requester != target;
    }

    bool TransferLeadership(gameplay::PlayerId requester, gameplay::PlayerId target) const override
    { ++calls; return requester.Valid() && target.Valid(); }
    bool Release(gameplay::PlayerId requester) const override
    { ++calls; return requester.Valid(); }
    bool Kick(gameplay::PlayerId requester, gameplay::PlayerId target) const override
    { ++calls; return requester.Valid() && target.Valid(); }
    bool Leave(gameplay::PlayerId requester) const override
    { ++calls; return requester.Valid(); }
    bool ClearMembership(gameplay::PlayerId member) const override
    { ++calls; return member.Valid(); }
    bool SynchronizeMembership(gameplay::PlayerId receiver,
                               const gameplay::social::party::PartyMembershipSync& sync) const override
    { ++calls; return receiver.Valid() && sync.members.size() < 6u; }
};
}

int main()
{
    FakePartyCommands commands;
    assert(commands.Join(gameplay::PlayerId(1u), gameplay::PlayerId(2u)));
    assert(!commands.Join(gameplay::PlayerId(), gameplay::PlayerId(2u)));
    assert(commands.TransferLeadership(gameplay::PlayerId(1u), gameplay::PlayerId(2u)));
    assert(commands.Release(gameplay::PlayerId(1u)));
    assert(commands.Kick(gameplay::PlayerId(1u), gameplay::PlayerId(2u)));
    assert(commands.Leave(gameplay::PlayerId(1u)));
    assert(commands.ClearMembership(gameplay::PlayerId(1u)));
    gameplay::social::party::PartyMembershipSync sync;
    sync.members.push_back(gameplay::PlayerId(1u));
    sync.members.push_back(gameplay::PlayerId(2u));
    assert(commands.SynchronizeMembership(gameplay::PlayerId(1u), sync));
    assert(commands.calls == 8);
    return 0;
}
