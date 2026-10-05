#include <cassert>

#include "gameplay/social/clan/v2/ClanMembershipService.h"

namespace
{
class FakeMembershipPort final : public gameplay::social::clan::IClanMembershipMutationPort
{
public:
    int calls = 0;

    gameplay::social::clan::ClanMembershipMutationResult SynchronizeMark(
        const gameplay::social::clan::ClanMembershipMutationRequest& request) override
    {
        ++calls;
        gameplay::social::clan::ClanMembershipMutationResult result;
        result.player = request.player;
        result.mark = request.mark;
        result.code = gameplay::social::clan::ClanMembershipMutationCode::Applied;
        return result;
    }
};
}

int main()
{
    using namespace gameplay;
    using namespace gameplay::social::clan;
    FakeMembershipPort port;

    ClanMembershipMutationResult applied = ClanMembershipService::SynchronizeMark(
        port, ClanMembershipMutationRequest(PlayerId(7u), 123));
    assert(applied.Applied());
    assert(port.calls == 1);

    ClanMembershipMutationResult cleared = ClanMembershipService::SynchronizeMark(
        port, ClanMembershipMutationRequest(PlayerId(7u), 0));
    assert(cleared.Applied());
    assert(port.calls == 2);

    ClanMembershipMutationResult invalidPlayer = ClanMembershipService::SynchronizeMark(
        port, ClanMembershipMutationRequest(PlayerId(), 123));
    assert(invalidPlayer.code == ClanMembershipMutationCode::InvalidRequest);
    assert(port.calls == 2);

    ClanMembershipMutationResult invalidMark = ClanMembershipService::SynchronizeMark(
        port, ClanMembershipMutationRequest(PlayerId(7u), -1));
    assert(invalidMark.code == ClanMembershipMutationCode::InvalidRequest);
    assert(port.calls == 2);
    return 0;
}
