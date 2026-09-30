#pragma once

#include <cstddef>
#include <cstdint>

#include "gameplay/core/v2/GameplayIds.h"
#include "network/v2/PacketRouter.h"

namespace gameplay
{
namespace network
{
namespace legacy
{
RouteStatus RouteServerGameplayPacket(PlayerId player,
                                      std::uint32_t opcode,
                                      const void* data,
                                      std::size_t size);
}
}
}
