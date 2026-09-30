#include "systems/social/party/v2/legacy/LegacyPartyRelayRuntime.h"

#include "systems/network/v2/legacy/LegacyPacketSender.h"
#include "systems/social/party/v2/legacy/LegacyPartyQueryAdapter.h"

namespace gameplay
{
namespace social
{
namespace party
{
namespace legacy
{
IPartyRelayService& ServerLegacyPartyRelayService()
{
    static PartyRelayService service(
        ServerLegacyPartyQueryAdapter(),
        network::legacy::ServerLegacyPacketSender());
    return service;
}
}
}
}
}
