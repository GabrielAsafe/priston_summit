#include "systems/gameplay/world/v2/legacy/LegacyTeleportAdapter.h"

#include <limits>

#include "core/server/legacy_sin_runtime/sinLinkHeader.h"
#include "systems/network/protocol/server/opcodes.h"
#include "systems/network/v2/legacy/LegacyPacketSender.h"

namespace gameplay
{
namespace world
{
namespace legacy
{
LegacyTeleportAdapter::LegacyTeleportAdapter(network::IPacketSender& sender)
    : sender_(sender)
{
}

TeleportResult LegacyTeleportAdapter::Teleport(PlayerId player, const WorldPosition& destination)
{
    if (!player.Valid())
        return TeleportResult(TeleportStatus::InvalidPlayer);
    if (!destination.map.Valid() || destination.map.value > static_cast<std::uint32_t>(std::numeric_limits<int>::max()))
        return TeleportResult(TeleportStatus::InvalidDestination);

    smTRANS_COMMAND packet = {};
    packet.code = OpCode::OPCODE_WARPFIELD;
    packet.size = sizeof(smTRANS_COMMAND);
    packet.WParam = static_cast<int>(destination.map.value);
    packet.SParam = destination.x;
    packet.LParam = destination.z;

    const network::SendResult sent = sender_.Send(
        player,
        network::PacketView(&packet, sizeof(packet), static_cast<std::uint32_t>(packet.code)));

    switch (sent.status)
    {
        case network::SendStatus::Sent:
            return TeleportResult(TeleportStatus::Sent);
        case network::SendStatus::InvalidPlayer:
            return TeleportResult(TeleportStatus::InvalidPlayer);
        case network::SendStatus::Disconnected:
            return TeleportResult(TeleportStatus::Disconnected);
        case network::SendStatus::InvalidPacket:
        case network::SendStatus::TransportFailure:
        default:
            return TeleportResult(TeleportStatus::TransportFailure);
    }
}

LegacyTeleportAdapter& ServerLegacyTeleportAdapter()
{
    static LegacyTeleportAdapter adapter(network::legacy::ServerLegacyPacketSender());
    return adapter;
}
}
}
}
