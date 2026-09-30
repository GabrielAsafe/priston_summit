#pragma once

#include "network/v2/PacketSender.h"
#include "network/v2/ConnectionQuery.h"
#include "systems/gameplay/player/v2/legacy/LegacyOnlinePlayerRegistry.h"

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

private:
    const player::legacy::LegacyOnlinePlayerRegistry& players_;
    const IConnectionQuery& connections_;
};

LegacyPacketSender& ServerLegacyPacketSender();
}
}
}
