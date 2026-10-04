#pragma once

#include "gameplay/social/v2/ChatDelivery.h"
#include "systems/gameplay/player/v2/legacy/LegacyOnlinePlayerRegistry.h"
#include "network/v2/PacketSender.h"

class rsPLAYINFO;

namespace gameplay
{
namespace social
{
namespace legacy
{
class LegacyChatDeliveryAdapter final : public IChatDeliveryService
{
public:
    LegacyChatDeliveryAdapter(const player::legacy::LegacyOnlinePlayerRegistry& registry,
                              network::IPacketSender& packets);

    ChatDeliveryResult SendToPlayer(PlayerId player, const ChatMessage& message) override;
    ChatDeliveryResult SendToLegacyPlayer(rsPLAYINFO* player, const ChatMessage& message);
    ChatDeliveryResult Broadcast(const ChatMessage& message) override;

private:
    const player::legacy::LegacyOnlinePlayerRegistry& registry_;
    network::IPacketSender& packets_;
};

LegacyChatDeliveryAdapter& ServerLegacyChatDeliveryAdapter();
}
}
}
