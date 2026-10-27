#include "systems/gameplay/companions/v2/legacy/LegacyCompanionUIIngressAdapter.h"

#include "systems/gameplay/companions/v2/legacy/CompanionLegacyUIRuntime.h"

namespace gameplay
{
namespace companions
{
namespace legacy
{
LegacyCompanionUIIngressAdapter::LegacyCompanionUIIngressAdapter(
    const player::legacy::LegacyOnlinePlayerRegistry& players)
    : players_(players)
{
}

void LegacyCompanionUIIngressAdapter::HandleAction(
    PlayerId owner, const protocol::CompanionUIActionPacket& packet)
{
    rsPLAYINFO* legacyOwner = players_.Resolve(owner);
    if(legacyOwner)
        companions::HandleUIAction(legacyOwner, &packet);
}

void LegacyCompanionUIIngressAdapter::HandleTradeConfirm(
    PlayerId owner, const protocol::CompanionTradeConfirmPacket& packet)
{
    rsPLAYINFO* legacyOwner = players_.Resolve(owner);
    if(legacyOwner)
        companions::HandleTradeConfirm(legacyOwner, &packet);
}

void LegacyCompanionUIIngressAdapter::HandleTradeApplyAck350Q(
    PlayerId owner,const protocol::CompanionTradeApplyAckPacket& packet)
{
    rsPLAYINFO* player=players_.Resolve(owner);
    if(player) companions::HandleTradeApplyAck350Q(player,&packet);
}

LegacyCompanionUIIngressAdapter& ServerLegacyCompanionUIIngressAdapter()
{
    static LegacyCompanionUIIngressAdapter adapter(player::legacy::ServerLegacyOnlinePlayerRegistry());
    return adapter;
}
}
}
}
