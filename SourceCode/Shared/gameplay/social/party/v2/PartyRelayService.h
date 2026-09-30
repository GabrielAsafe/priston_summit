#pragma once

#include <cstddef>

#include "gameplay/social/party/v2/PartyQuery.h"
#include "network/v2/PacketSender.h"

namespace gameplay
{
namespace social
{
namespace party
{
struct PartyRelayResult
{
    bool partyFound;
    std::size_t attempted;
    std::size_t sent;

    PartyRelayResult() : partyFound(false), attempted(0u), sent(0u) {}
};

class IPartyRelayService
{
public:
    virtual ~IPartyRelayService() {}
    virtual PartyRelayResult Relay(PlayerId sender, const network::PacketView& packet) = 0;
};

class PartyRelayService final : public IPartyRelayService
{
public:
    PartyRelayService(const IPartyQuery& parties, network::IPacketSender& packets)
        : parties_(parties), packets_(packets) {}

    PartyRelayResult Relay(PlayerId sender, const network::PacketView& packet) override
    {
        PartyRelayResult result;
        if(!sender.Valid() || !packet.Valid())
            return result;

        PartySnapshot party;
        if(!parties_.TryGetParty(sender, party))
            return result;

        result.partyFound = true;
        for(std::vector<PartyMemberSnapshot>::const_iterator it = party.members.begin();
            it != party.members.end(); ++it)
        {
            if(!it->online || !it->player.Valid() || it->player == sender)
                continue;

            ++result.attempted;
            if(packets_.Send(it->player, packet).Succeeded())
                ++result.sent;
        }
        return result;
    }

private:
    const IPartyQuery& parties_;
    network::IPacketSender& packets_;
};
}
}
}
