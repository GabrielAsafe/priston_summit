#pragma once

#include "gameplay/social/clan/v2/ClanUpdateNotifier.h"
#include "systems/gameplay/player/v2/legacy/LegacyOnlinePlayerRegistry.h"
#include "network/v2/PacketSender.h"

namespace gameplay
{
namespace social
{
namespace clan
{
namespace legacy
{
class LegacyClanUpdateNotifier final : public IClanUpdateNotifier
{
public:
    LegacyClanUpdateNotifier(const player::legacy::LegacyOnlinePlayerRegistry& players,
                             network::IPacketSender& packets);
    ClanUpdateNotifyResult Notify(PlayerId source,
                                  const std::vector<ClanUpdateRecipient>& recipients) override;
private:
    const player::legacy::LegacyOnlinePlayerRegistry& players_;
    network::IPacketSender& packets_;
};

LegacyClanUpdateNotifier& ServerLegacyClanUpdateNotifier();
}
}
}
}
