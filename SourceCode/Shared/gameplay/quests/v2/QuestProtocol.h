#pragma once

#include <cstdint>

namespace gameplay
{
namespace quests
{
namespace protocol
{
// Wire-compatible with legacy smTRANS_COMMAND. Kept feature-local so the
// PacketRouter does not need to include the legacy packet header.
struct QuestCommandPacket
{
    std::int32_t size;
    std::int32_t code;
    std::int32_t LParam;
    std::int32_t WParam;
    std::int32_t SParam;
    std::int32_t EParam;
};

static_assert(sizeof(QuestCommandPacket) == 24u, "quest command wire layout changed");
}
}
}
