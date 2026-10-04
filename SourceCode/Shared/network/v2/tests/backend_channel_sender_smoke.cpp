#include <cassert>
#include <cstdint>

#include "network/v2/BackendChannelSender.h"

namespace
{
class FakeSender final : public gameplay::network::IBackendChannelSender
{
public:
    FakeSender() : calls(0), last(gameplay::network::BackendChannel::Data) {}

    bool Available(gameplay::network::BackendChannel) const override { return true; }

    gameplay::network::SendResult Send(gameplay::network::BackendChannel channel,
                                       const gameplay::network::PacketView& packet) override
    {
        ++calls;
        last = channel;
        return packet.Valid()
            ? gameplay::network::SendResult(gameplay::network::SendStatus::Sent, packet.size)
            : gameplay::network::SendResult(gameplay::network::SendStatus::InvalidPacket, 0u);
    }

    int calls;
    gameplay::network::BackendChannel last;
};
}

int main()
{
    FakeSender sender;
    const std::uint32_t payload = 0x12345678u;
    const gameplay::network::PacketView packet(&payload, sizeof(payload), 0xAAu);

    assert(sender.Send(gameplay::network::BackendChannel::Data, packet).Succeeded());
    assert(sender.last == gameplay::network::BackendChannel::Data);
    assert(sender.Send(gameplay::network::BackendChannel::User, packet).Succeeded());
    assert(sender.last == gameplay::network::BackendChannel::User);
    assert(sender.Send(gameplay::network::BackendChannel::Extend, packet).Succeeded());
    assert(sender.last == gameplay::network::BackendChannel::Extend);
    assert(sender.calls == 3);
    return 0;
}
