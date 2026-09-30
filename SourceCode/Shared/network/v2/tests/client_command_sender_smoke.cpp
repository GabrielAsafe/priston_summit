#include "network/v2/ClientCommandSender.h"

#include <cassert>
#include <cstdint>

namespace
{
class FakeSender final : public gameplay::network::IClientCommandSender
{
public:
    bool available = true;
    std::uint32_t lastOpcode = 0u;
    std::size_t lastSize = 0u;

    bool Available() const override { return available; }

    gameplay::network::SendResult Send(const gameplay::network::PacketView& packet) override
    {
        if (!available)
            return gameplay::network::SendResult(gameplay::network::SendStatus::Disconnected, 0u);
        if (!packet.Valid())
            return gameplay::network::SendResult(gameplay::network::SendStatus::InvalidPacket, 0u);
        lastOpcode = packet.opcode;
        lastSize = packet.size;
        return gameplay::network::SendResult(gameplay::network::SendStatus::Sent, packet.size);
    }
};
}

int main()
{
    FakeSender sender;
    std::uint32_t wire[3] = { 12u, 0x50320001u, 7u };
    auto result = sender.Send(gameplay::network::PacketView(wire, sizeof(wire), wire[1]));
    assert(result.Succeeded());
    assert(result.bytes == sizeof(wire));
    assert(sender.lastOpcode == wire[1]);

    sender.available = false;
    result = sender.Send(gameplay::network::PacketView(wire, sizeof(wire), wire[1]));
    assert(result.status == gameplay::network::SendStatus::Disconnected);
    return 0;
}
