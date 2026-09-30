#include <cassert>
#include <cstdint>
#include "network/v2/PacketRouter.h"

namespace
{
class Handler final : public gameplay::network::IProtocolHandler
{
public:
    explicit Handler(std::uint32_t opcode) : opcode_(opcode), calls(0) {}
    std::uint32_t Opcode() const override { return opcode_; }
    gameplay::network::RouteStatus Handle(gameplay::PlayerId player,
                                           const gameplay::network::PacketView& packet) override
    {
        ++calls;
        return player.Valid() && packet.Valid()
            ? gameplay::network::RouteStatus::Handled
            : gameplay::network::RouteStatus::Rejected;
    }
    std::uint32_t opcode_;
    int calls;
};
}

int main()
{
    gameplay::network::PacketRouter router;
    Handler one(0x100u), duplicate(0x100u), two(0x200u);
    assert(router.Register(one));
    assert(!router.Register(duplicate));
    assert(router.Register(two));
    assert(router.Handles(0x100u));
    assert(!router.Handles(0x300u));
    std::uint32_t wire = 0u;
    assert(router.Route(gameplay::PlayerId(1u), gameplay::network::PacketView(&wire, sizeof(wire), 0x100u)) == gameplay::network::RouteStatus::Handled);
    assert(one.calls == 1);
    assert(router.Route(gameplay::PlayerId(1u), gameplay::network::PacketView(&wire, sizeof(wire), 0x300u)) == gameplay::network::RouteStatus::UnknownOpcode);
    assert(router.Route(gameplay::PlayerId(), gameplay::network::PacketView(&wire, sizeof(wire), 0x300u)) == gameplay::network::RouteStatus::UnknownOpcode);
    assert(router.Route(gameplay::PlayerId(), gameplay::network::PacketView(&wire, sizeof(wire), 0x100u)) == gameplay::network::RouteStatus::InvalidPacket);
    return 0;
}
