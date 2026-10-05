#include "systems/network/v2/legacy/LegacyClientCommandSender.h"

#include <limits>

#include "systems/network/transport/legacy/smwsock.h"

extern smWINSOCK *smWsockServer;

namespace gameplay
{
namespace network
{
namespace legacy
{
bool LegacyClientCommandSender::Available() const
{
    return smWsockServer != 0;
}

SendResult LegacyClientCommandSender::Send(const PacketView& packet)
{
    return SendToLegacySocket(smWsockServer, packet);
}

SendResult LegacyClientCommandSender::SendToLegacySocket(smWINSOCK* socket, const PacketView& packet)
{
    if (!packet.Valid() || packet.size > static_cast<std::size_t>(std::numeric_limits<int>::max()))
        return SendResult(SendStatus::InvalidPacket, 0u);
    if (!socket)
        return SendResult(SendStatus::Disconnected, 0u);

    const int sent = socket->Send(
        reinterpret_cast<char*>(const_cast<void*>(packet.data)),
        static_cast<int>(packet.size), TRUE);
    return sent == TRUE
        ? SendResult(SendStatus::Sent, packet.size)
        : SendResult(SendStatus::TransportFailure, 0u);
}

LegacyClientCommandSender& ClientLegacyCommandSender()
{
    static LegacyClientCommandSender sender;
    return sender;
}

int SendPrimaryLegacyPacket(const void* data, int size, int /*legacyWaitFlag*/)
{
    if (size <= 0)
        return 0;
    return ClientLegacyCommandSender().Send(
        PacketView(data, static_cast<std::size_t>(size), 0u)).Succeeded() ? 1 : 0;
}
}
}
}
