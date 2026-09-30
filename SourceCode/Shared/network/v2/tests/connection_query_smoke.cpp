#include <cassert>
#include <map>
#include "network/v2/ConnectionQuery.h"

namespace
{
class FakeConnections final : public gameplay::network::IConnectionQuery
{
public:
    std::map<std::uint32_t, gameplay::network::ConnectionSnapshot> states;
    bool TryGet(gameplay::PlayerId player, gameplay::network::ConnectionSnapshot& out) const override
    {
        std::map<std::uint32_t, gameplay::network::ConnectionSnapshot>::const_iterator it=states.find(player.value);
        if(it==states.end())
            return false;
        out=it->second;
        return true;
    }
    bool IsConnected(gameplay::PlayerId player) const override
    { gameplay::network::ConnectionSnapshot s; return TryGet(player,s)&&s.connected; }
    bool IsCurrent(gameplay::PlayerHandle player) const override
    { gameplay::network::ConnectionSnapshot s; return TryGet(player.id,s)&&s.player.generation==player.generation; }
};
}

int main()
{
    FakeConnections q;
    gameplay::network::ConnectionSnapshot s;
    s.player=gameplay::PlayerHandle(gameplay::PlayerId(9u),3u); s.connected=true; s.remoteAddress="127.0.0.1";
    q.states[9u]=s;
    assert(q.IsConnected(gameplay::PlayerId(9u)));
    gameplay::network::ConnectionSnapshot out; assert(q.TryGet(gameplay::PlayerId(9u), out)); assert(out.remoteAddress == "127.0.0.1");
    assert(q.IsCurrent(gameplay::PlayerHandle(gameplay::PlayerId(9u),3u)));
    assert(!q.IsCurrent(gameplay::PlayerHandle(gameplay::PlayerId(9u),4u)));
    assert(!q.IsConnected(gameplay::PlayerId(10u)));
    return 0;
}
