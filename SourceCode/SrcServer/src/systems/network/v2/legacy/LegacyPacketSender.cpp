#include "systems/network/v2/legacy/LegacyPacketSender.h"
#include "systems/network/v2/legacy/LegacyConnectionQuery.h"

#include <limits>

#include "systems/rendering/legacy_3d/smd3d.h"
#include "systems/gameplay/character/core/character.h"
#include "systems/network/transport/legacy/smwsock.h"

namespace gameplay
{
namespace network
{
namespace legacy
{
LegacyPacketSender::LegacyPacketSender(const player::legacy::LegacyOnlinePlayerRegistry& players, const IConnectionQuery& connections)
    : players_(players), connections_(connections)
{
}

SendResult LegacyPacketSender::Send(PlayerId playerId, const PacketView& packet)
{
    if (!playerId.Valid())
        return SendResult(SendStatus::InvalidPlayer, 0u);
    if (!packet.Valid() || packet.size > static_cast<std::size_t>(std::numeric_limits<int>::max()))
        return SendResult(SendStatus::InvalidPacket, 0u);

    if (!connections_.IsConnected(playerId))
        return SendResult(SendStatus::Disconnected, 0u);
    rsPLAYINFO* player = players_.Resolve(playerId);
    if (!player || !player->lpsmSock)
        return SendResult(SendStatus::Disconnected, 0u);

    const int size = static_cast<int>(packet.size);
    if (!player->lpsmSock->Send(reinterpret_cast<char*>(const_cast<void*>(packet.data)), size, TRUE))
        return SendResult(SendStatus::TransportFailure, 0u);

    return SendResult(SendStatus::Sent, packet.size);
}

LegacyPacketSender& ServerLegacyPacketSender()
{
    static LegacyPacketSender sender(player::legacy::ServerLegacyOnlinePlayerRegistry(), ServerLegacyConnectionQuery());
    return sender;
}
}
}
}
