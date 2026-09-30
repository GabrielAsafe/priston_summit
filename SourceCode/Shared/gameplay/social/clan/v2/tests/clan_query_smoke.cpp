#include <cassert>
#include "gameplay/social/clan/v2/ClanQuery.h"

namespace
{
class FakeClanQuery final : public gameplay::social::clan::IClanQuery
{
public:
    bool TryGetMembership(gameplay::PlayerId player, gameplay::social::clan::ClanMembershipSnapshot& out) const override
    {
        if(!player.Valid()) return false;
        out.player = player;
        out.clan = gameplay::social::clan::ClanId(player.value < 3u ? 77u : 88u);
        out.mark = 123;
        return true;
    }
};
}

int main()
{
    FakeClanQuery query;
    gameplay::social::clan::ClanMembershipSnapshot a, b, c;
    assert(query.TryGetMembership(gameplay::PlayerId(1u), a));
    assert(query.TryGetMembership(gameplay::PlayerId(2u), b));
    assert(query.TryGetMembership(gameplay::PlayerId(3u), c));
    assert(a.IsMember());
    assert(gameplay::social::clan::SameClan(a, b));
    assert(!gameplay::social::clan::SameClan(a, c));
    return 0;
}
