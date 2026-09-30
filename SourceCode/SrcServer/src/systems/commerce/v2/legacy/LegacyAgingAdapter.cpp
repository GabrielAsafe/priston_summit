#include "systems/commerce/v2/legacy/LegacyAgingAdapter.h"

#include <cstring>

#include "systems/network/transport/legacy/smwsock.h"
#include "systems/rendering/legacy_3d/smd3d.h"
#include "systems/gameplay/character/core/character.h"
#include "core/server/legacy_runtime/onserver.h"
#include "systems/commerce/legacy/sinTrade.h"

int rsRecvAgingItem(rsPLAYINFO* lpPlayInfo, sCRAFTITEM_SERVER* pCraftItem_Server);
int rsRecvAgingUpgradeItem(rsPLAYINFO* lpPlayInfo, TRANS_ITEMINFO* lpTransItemInfo);

namespace gameplay
{
namespace commerce
{
namespace legacy
{
LegacyAgingAdapter::LegacyAgingAdapter(const player::legacy::LegacyOnlinePlayerRegistry& registry)
    : registry_(registry)
{
}

AgingResult LegacyAgingAdapter::Process(const AgingRequest& request)
{
    AgingResult result;
    result.player = request.player;
    result.operation = request.operation;

    rsPLAYINFO* player = registry_.Resolve(request.player);
    if(!player)
    {
        result.code = AgingCode::InvalidPlayer;
        return result;
    }

    if(request.operation == AgingOperation::Apply)
    {
        if(request.requestBytes.size() != sizeof(sCRAFTITEM_SERVER))
        {
            result.code = AgingCode::InvalidPayload;
            return result;
        }
        sCRAFTITEM_SERVER packet;
        std::memcpy(&packet, request.requestBytes.data(), sizeof(packet));
        result.code = rsRecvAgingItem(player, &packet) == TRUE ? AgingCode::Processed : AgingCode::Rejected;
        return result;
    }

    if(request.requestBytes.size() != sizeof(TRANS_ITEMINFO))
    {
        result.code = AgingCode::InvalidPayload;
        return result;
    }
    TRANS_ITEMINFO packet;
    std::memcpy(&packet, request.requestBytes.data(), sizeof(packet));
    result.code = rsRecvAgingUpgradeItem(player, &packet) == TRUE ? AgingCode::Processed : AgingCode::Rejected;
    return result;
}

LegacyAgingAdapter& ServerLegacyAgingAdapter()
{
    static LegacyAgingAdapter adapter(player::legacy::ServerLegacyOnlinePlayerRegistry());
    return adapter;
}
}
}
}
