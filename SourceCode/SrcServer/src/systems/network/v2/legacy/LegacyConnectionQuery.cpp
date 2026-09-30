#include "systems/network/v2/legacy/LegacyConnectionQuery.h"

#include "systems/network/transport/legacy/smwsock.h"
#include "systems/rendering/legacy_3d/smd3d.h"
#include "systems/gameplay/character/core/character.h"
#include "core/server/legacy_runtime/onserver.h"

namespace gameplay
{
namespace network
{
namespace legacy
{
LegacyConnectionQuery::LegacyConnectionQuery(const player::legacy::LegacyOnlinePlayerRegistry& players)
    : players_(players)
{
}

bool LegacyConnectionQuery::TryGet(PlayerId playerId, ConnectionSnapshot& out) const
{
    rsPLAYINFO* player = players_.Resolve(playerId);
    if (!player)
        return false;
    ConnectionSnapshot snapshot;
    snapshot.player = players_.HandleOf(player);
    snapshot.connected = snapshot.player.Valid() && player->lpsmSock != 0;
    if(snapshot.connected && player->lpsmSock->szIPAddr)
        snapshot.remoteAddress = player->lpsmSock->szIPAddr;
    out = snapshot;
    return true;
}

bool LegacyConnectionQuery::IsConnected(PlayerId player) const
{
    ConnectionSnapshot snapshot;
    return TryGet(player, snapshot) && snapshot.connected;
}

bool LegacyConnectionQuery::IsCurrent(PlayerHandle player) const
{
    return players_.IsCurrent(player);
}

const LegacyConnectionQuery& ServerLegacyConnectionQuery()
{
    static LegacyConnectionQuery query(player::legacy::ServerLegacyOnlinePlayerRegistry());
    return query;
}
}
}
}
