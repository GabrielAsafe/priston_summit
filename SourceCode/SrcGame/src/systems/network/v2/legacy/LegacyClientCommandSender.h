#pragma once

#include "network/v2/ClientCommandSender.h"

namespace gameplay
{
namespace network
{
namespace legacy
{
class LegacyClientCommandSender final : public IClientCommandSender
{
public:
    bool Available() const override;
    SendResult Send(const PacketView& packet) override;
};

LegacyClientCommandSender& ClientLegacyCommandSender();

// Compatibility bridge for frozen RED call sites. New feature code should use
// IClientCommandSender directly. The third argument preserves the legacy Send
// call shape while transport ownership stays inside this AMBER adapter.
int SendPrimaryLegacyPacket(const void* data, int size, int legacyWaitFlag);
}
}
}
