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
}
}
}
