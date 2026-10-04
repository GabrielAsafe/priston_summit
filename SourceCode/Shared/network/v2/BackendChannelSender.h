#pragma once

#include "network/v2/PacketSender.h"

namespace gameplay
{
namespace network
{
enum class BackendChannel
{
    Data = 0,
    User,
    Extend
};

class IBackendChannelSender
{
public:
    virtual ~IBackendChannelSender() {}
    virtual bool Available(BackendChannel channel) const = 0;
    virtual SendResult Send(BackendChannel channel, const PacketView& packet) = 0;
};
}
}
