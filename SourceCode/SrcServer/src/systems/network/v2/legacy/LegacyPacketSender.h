#pragma once

#include "network/v2/PacketSender.h"
#include "network/v2/ClientCommandSender.h"
#include "network/v2/ConnectionQuery.h"
#include "systems/gameplay/player/v2/legacy/LegacyOnlinePlayerRegistry.h"

class smWINSOCK;

namespace gameplay
{
namespace network
{
namespace legacy
{
class LegacyPacketSender final : public IPacketSender
{
public:
    LegacyPacketSender(const player::legacy::LegacyOnlinePlayerRegistry& players, const IConnectionQuery& connections);
    SendResult Send(PlayerId player, const PacketView& packet) override;
    SendResult SendToLegacySocket(smWINSOCK* socket, const PacketView& packet);

private:
    const player::legacy::LegacyOnlinePlayerRegistry& players_;
    const IConnectionQuery& connections_;
};

LegacyPacketSender& ServerLegacyPacketSender();

// Primary upstream command channel used by frozen RED server call sites.
// New code should consume IClientCommandSender directly; this compatibility
// bridge keeps smWsockServer ownership inside the AMBER networking adapter.
class LegacyServerCommandSender final : public IClientCommandSender
{
public:
    bool Available() const override;
    SendResult Send(const PacketView& packet) override;
};

LegacyServerCommandSender& ServerLegacyCommandSender();
int SendServerPrimaryLegacyPacket(const void* data, int size, int legacyWaitFlag);
}
}
}
