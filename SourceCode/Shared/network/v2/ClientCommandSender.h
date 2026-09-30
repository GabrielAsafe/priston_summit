#pragma once

#include "network/v2/PacketSender.h"

namespace gameplay
{
namespace network
{
class IClientCommandSender
{
public:
    virtual ~IClientCommandSender() {}
    virtual bool Available() const = 0;
    virtual SendResult Send(const PacketView& packet) = 0;
};
}
}
