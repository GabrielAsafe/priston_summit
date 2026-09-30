#include <cassert>
#include <cstdint>
#include "network/v2/PacketSender.h"

namespace
{
class FakeSender final : public gameplay::network::IPacketSender
{
public:
    gameplay::PlayerId lastPlayer;
    gameplay::network::PacketView lastPacket;

    gameplay::network::SendResult Send(gameplay::PlayerId player,
                                       const gameplay::network::PacketView& packet) override
    {
        lastPlayer = player;
        lastPacket = packet;
        if (!player.Valid())
            return gameplay::network::SendResult(gameplay::network::SendStatus::InvalidPlayer, 0u);
        if (!packet.Valid())
            return gameplay::network::SendResult(gameplay::network::SendStatus::InvalidPacket, 0u);
        return gameplay::network::SendResult(gameplay::network::SendStatus::Sent, packet.size);
    }
};
}

int main()
{
    std::uint32_t wire[2] = {8u, 0x12345678u};
    FakeSender sender;
    const gameplay::network::PacketView packet(wire, sizeof(wire), wire[1]);
    const gameplay::network::SendResult ok = sender.Send(gameplay::PlayerId(77u), packet);
    assert(ok.Succeeded());
    assert(ok.bytes == sizeof(wire));
    assert(sender.lastPlayer.value == 77u);
    assert(sender.lastPacket.opcode == 0x12345678u);
    assert(!sender.Send(gameplay::PlayerId(), packet).Succeeded());
    assert(!sender.Send(gameplay::PlayerId(77u), gameplay::network::PacketView()).Succeeded());
    return 0;
}
