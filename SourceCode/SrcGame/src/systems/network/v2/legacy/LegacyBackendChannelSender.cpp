#include "systems/network/v2/legacy/LegacyBackendChannelSender.h"

#include <limits>

#include "systems/network/transport/legacy/smwsock.h"

extern smWINSOCK* smWsockDataServer;
extern smWINSOCK* smWsockUserServer;
extern smWINSOCK* smWsockExtendServer;
extern smWINSOCK* smWsockServer;

namespace gameplay
{
namespace network
{
namespace legacy
{
namespace
{
smWINSOCK* ResolveBackendSocket(BackendChannel channel)
{
    switch(channel)
    {
    case BackendChannel::Data:
        return smWsockDataServer;
    case BackendChannel::User:
        return smWsockUserServer;
    case BackendChannel::Extend:
        return smWsockExtendServer;
    }
    return 0;
}
}

bool LegacyBackendChannelSender::Available(BackendChannel channel) const
{
    return ResolveBackendSocket(channel) != 0;
}

bool LegacyBackendChannelSender::AvailableAndDistinctFromPrimary(BackendChannel channel) const
{
    smWINSOCK* socket = ResolveBackendSocket(channel);
    return socket != 0 && socket != smWsockServer;
}

SendResult LegacyBackendChannelSender::Send(BackendChannel channel, const PacketView& packet)
{
    if(!packet.Valid() || packet.size > static_cast<std::size_t>(std::numeric_limits<int>::max()))
        return SendResult(SendStatus::InvalidPacket, 0u);

    smWINSOCK* socket = ResolveBackendSocket(channel);
    if(!socket)
        return SendResult(SendStatus::Disconnected, 0u);

    int sent = 0;
    switch(channel)
    {
    case BackendChannel::Data:
        sent = smWsockDataServer->Send(
            reinterpret_cast<char*>(const_cast<void*>(packet.data)),
            static_cast<int>(packet.size), TRUE);
        break;
    case BackendChannel::User:
        sent = smWsockUserServer->Send(
            reinterpret_cast<char*>(const_cast<void*>(packet.data)),
            static_cast<int>(packet.size), TRUE);
        break;
    case BackendChannel::Extend:
        sent = smWsockExtendServer->Send(
            reinterpret_cast<char*>(const_cast<void*>(packet.data)),
            static_cast<int>(packet.size), TRUE);
        break;
    }

    return sent == TRUE
        ? SendResult(SendStatus::Sent, packet.size)
        : SendResult(SendStatus::TransportFailure, 0u);
}

LegacyBackendChannelSender& ClientLegacyBackendChannelSender()
{
    static LegacyBackendChannelSender sender;
    return sender;
}
}
}
}
