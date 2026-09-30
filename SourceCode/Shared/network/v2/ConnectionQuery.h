#pragma once

#include <cstdint>
#include <string>

#include "gameplay/core/v2/GameplayIds.h"

namespace gameplay
{
namespace network
{
struct ConnectionSnapshot
{
    PlayerHandle player;
    bool connected;
    std::string remoteAddress;

    ConnectionSnapshot() : player(), connected(false), remoteAddress() {}
};

class IConnectionQuery
{
public:
    virtual ~IConnectionQuery() {}
    virtual bool TryGet(PlayerId player, ConnectionSnapshot& out) const = 0;
    virtual bool IsConnected(PlayerId player) const = 0;
    virtual bool IsCurrent(PlayerHandle player) const = 0;
};
}
}
