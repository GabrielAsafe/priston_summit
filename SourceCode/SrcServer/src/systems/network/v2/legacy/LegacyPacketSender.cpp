#include "systems/network/v2/legacy/LegacyPacketSender.h"
#include "systems/network/v2/legacy/LegacyConnectionQuery.h"

#include <limits>

#include "systems/rendering/legacy_3d/smd3d.h"
#include "systems/gameplay/character/core/character.h"
#include "systems/network/transport/legacy/smwsock.h"

extern smWINSOCK *smWsockServer;

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

bool LegacyServerCommandSender::Available() const
{
    return smWsockServer != 0;
}

SendResult LegacyServerCommandSender::Send(const PacketView& packet)
{
    if (!packet.Valid() || packet.size > static_cast<std::size_t>(std::numeric_limits<int>::max()))
        return SendResult(SendStatus::InvalidPacket, 0u);
    if (!smWsockServer)
        return SendResult(SendStatus::Disconnected, 0u);

    const int sent = smWsockServer->Send(
        reinterpret_cast<char*>(const_cast<void*>(packet.data)),
        static_cast<int>(packet.size), TRUE);
    return sent == TRUE
        ? SendResult(SendStatus::Sent, packet.size)
        : SendResult(SendStatus::TransportFailure, 0u);
}

LegacyServerCommandSender& ServerLegacyCommandSender()
{
    static LegacyServerCommandSender sender;
    return sender;
}

int SendServerPrimaryLegacyPacket(const void* data, int size, int /*legacyWaitFlag*/)
{
    if (size <= 0)
        return 0;
    return ServerLegacyCommandSender().Send(
        PacketView(data, static_cast<std::size_t>(size), 0u)).Succeeded() ? 1 : 0;
}
}
}
}
