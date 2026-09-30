#pragma once

#include "gameplay/world/v2/ITeleportService.h"
#include "network/v2/PacketSender.h"

namespace gameplay
{
namespace world
{
namespace legacy
{
class LegacyTeleportAdapter final : public ITeleportService
{
public:
    explicit LegacyTeleportAdapter(network::IPacketSender& sender);

    TeleportResult Teleport(PlayerId player, const WorldPosition& destination) override;

private:
    network::IPacketSender& sender_;
};

LegacyTeleportAdapter& ServerLegacyTeleportAdapter();
}
}
}
