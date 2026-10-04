#pragma once

#include "network/v2/BackendChannelSender.h"

namespace gameplay
{
namespace network
{
namespace legacy
{
class LegacyBackendChannelSender final : public IBackendChannelSender
{
public:
    bool Available(BackendChannel channel) const override;
    bool AvailableAndDistinctFromPrimary(BackendChannel channel) const;
    SendResult Send(BackendChannel channel, const PacketView& packet) override;
};

LegacyBackendChannelSender& ServerLegacyBackendChannelSender();
}
}
}
