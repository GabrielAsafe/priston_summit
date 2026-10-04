#include "systems/commerce/v2/legacy/LegacyMixingAdapter.h"

#include <cstring>

#include "systems/network/transport/legacy/smwsock.h"
#include "systems/rendering/legacy_3d/smd3d.h"
#include "systems/gameplay/character/core/character.h"
#include "core/server/legacy_runtime/onserver.h"
#include "systems/commerce/legacy/sinTrade.h"
#include "systems/commerce/v2/legacy/LegacyCommerceReservationGuard.h"
#include "gameplay/inventory/v2/InventorySnapshotRollback.h"
#include "systems/inventory/v2/legacy/LegacyInventoryRuntime.h"

int rsRecvMixtureResetItem(rsPLAYINFO* lpPlayInfo, sMIXTURE_RESET_ITEM_SERVER* pMixtureResetItem_Server);

namespace gameplay
{
namespace commerce
{
namespace legacy
{
LegacyMixingAdapter::LegacyMixingAdapter(const player::legacy::LegacyOnlinePlayerRegistry& registry)
    : registry_(registry)
{
}

MixingResult LegacyMixingAdapter::Process(const MixingRequest& request)
{
    MixingResult result;
    result.player = request.player;
    result.operation = request.operation;

    rsPLAYINFO* player = registry_.Resolve(request.player);
    if(!player)
    {
        result.code = MixingCode::InvalidPlayer;
        return result;
    }
    if(request.operation != MixingOperation::Reset ||
       request.requestBytes.size() != sizeof(sMIXTURE_RESET_ITEM_SERVER))
    {
        result.code = MixingCode::InvalidPayload;
        return result;
    }

    sMIXTURE_RESET_ITEM_SERVER packet;
    std::memcpy(&packet, request.requestBytes.data(), sizeof(packet));
    std::vector<inventory::ItemInstanceId> items;
    AppendLegacyReservationItem(
        items, packet.DesMixtureResetItem.sItemInfo.CODE,
        packet.DesMixtureResetItem.sItemInfo.ItemHeader.Head,
        packet.DesMixtureResetItem.sItemInfo.ItemHeader.dwChkSum);
    AppendLegacyReservationItem(
        items, packet.dwMixtureResetStoneItemCode, packet.dwMixtureResetStoneHead,
        packet.dwMixtureResetStoneCheckSum);
    LegacyCommerceReservationGuard reservation(request.player, items);
    if(!reservation.Accepted())
    {
        result.code = MixingCode::InventoryReservationRejected;
        return result;
    }
    inventory::legacy::LegacyInventoryRuntime& inventoryRuntime = inventory::legacy::ServerLegacyInventoryRuntime();
    inventory::InventorySnapshotRollback rollback(inventoryRuntime, inventoryRuntime, request.player);
    const int legacyProcessed = rsRecvMixtureResetItem(player, &packet);
    if(legacyProcessed == TRUE && sMixtureReset_Recv.Result == TRUE)
    {
        result.code = MixingCode::Processed;
        return result;
    }
    if(rollback.Captured() && !rollback.Restore().Restored())
    {
        result.code = MixingCode::RollbackFailed;
        return result;
    }
    result.code = MixingCode::Rejected;
    return result;
}

LegacyMixingAdapter& ServerLegacyMixingAdapter()
{
    static LegacyMixingAdapter adapter(player::legacy::ServerLegacyOnlinePlayerRegistry());
    return adapter;
}
}
}
}
