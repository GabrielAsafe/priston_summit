#include <cassert>
#include <vector>

#include "gameplay/social/party/v2/PartyRelayService.h"

namespace
{
class FakePartyQuery final : public gameplay::social::party::IPartyQuery
{
public:
    bool TryGetParty(gameplay::PlayerId member, gameplay::social::party::PartySnapshot& out) const override
    {
        if(!member.Valid()) return false;
        out.leader = gameplay::PlayerId(1u);
        out.members.push_back(gameplay::social::party::PartyMemberSnapshot(gameplay::PlayerId(1u), gameplay::MapId(3u), true));
        out.members.push_back(gameplay::social::party::PartyMemberSnapshot(gameplay::PlayerId(2u), gameplay::MapId(3u), true));
        out.members.push_back(gameplay::social::party::PartyMemberSnapshot(gameplay::PlayerId(3u), gameplay::MapId(3u), false));
        return true;
    }
};

class FakePacketSender final : public gameplay::network::IPacketSender
{
public:
    std::vector<gameplay::PlayerId> recipients;
    gameplay::network::SendResult Send(gameplay::PlayerId player, const gameplay::network::PacketView& packet) override
    {
        if(!player.Valid() || !packet.Valid())
            return gameplay::network::SendResult(gameplay::network::SendStatus::InvalidPacket, 0u);
        recipients.push_back(player);
        return gameplay::network::SendResult(gameplay::network::SendStatus::Sent, packet.size);
    }
};
}

int main()
{
    FakePartyQuery query;
    FakePacketSender sender;
    gameplay::social::party::PartyRelayService service(query, sender);
    const unsigned int wire[2] = { 8u, 0x1234u };
    gameplay::social::party::PartyRelayResult result = service.Relay(
        gameplay::PlayerId(1u), gameplay::network::PacketView(wire, sizeof(wire), wire[1]));
    assert(result.partyFound);
    assert(result.attempted == 1u);
    assert(result.sent == 1u);
    assert(sender.recipients.size() == 1u);
    assert(sender.recipients[0] == gameplay::PlayerId(2u));
    return 0;
}
