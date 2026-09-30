#pragma once

#include <cstddef>
#include <cstdint>

#include "gameplay/core/v2/GameplayIds.h"

namespace gameplay
{
namespace network
{
// Non-owning view used only for synchronous sends. The sender must not retain data.
struct PacketView
{
    const void* data;
    std::size_t size;
    std::uint32_t opcode;

    PacketView(const void* packetData = 0,
               std::size_t packetSize = 0u,
               std::uint32_t packetOpcode = 0u)
        : data(packetData), size(packetSize), opcode(packetOpcode) {}

    bool Valid() const { return data != 0 && size > 0u; }
};

enum class SendStatus
{
    Sent = 0,
    InvalidPlayer,
    Disconnected,
    InvalidPacket,
    TransportFailure
};

struct SendResult
{
    SendStatus status;
    std::size_t bytes;

    SendResult(SendStatus value = SendStatus::InvalidPacket, std::size_t sentBytes = 0u)
        : status(value), bytes(sentBytes) {}

    bool Succeeded() const { return status == SendStatus::Sent; }
};

class IPacketSender
{
public:
    virtual ~IPacketSender() {}
    virtual SendResult Send(PlayerId player, const PacketView& packet) = 0;
};
}
}
