#include "systems/commerce/v2/legacy/LegacyAgingAdapter.h"

#include <cstring>

#include "systems/network/transport/legacy/smwsock.h"
#include "systems/rendering/legacy_3d/smd3d.h"
#include "systems/gameplay/character/core/character.h"
#include "core/server/legacy_runtime/onserver.h"
#include "systems/commerce/legacy/sinTrade.h"
#include "systems/commerce/v2/legacy/LegacyCommerceReservationGuard.h"
#include "gameplay/inventory/v2/InventorySnapshotRollback.h"
#include "systems/inventory/v2/legacy/LegacyInventoryRuntime.h"

int rsRecvAgingItem(rsPLAYINFO* lpPlayInfo, sCRAFTITEM_SERVER* pCraftItem_Server);
int rsRecvAgingUpgradeItem(rsPLAYINFO* lpPlayInfo, TRANS_ITEMINFO* lpTransItemInfo);

namespace
{
std::vector<gameplay::inventory::ItemInstanceId> AgingApplyReservations(const sCRAFTITEM_SERVER& packet)
{
    std::vector<gameplay::inventory::ItemInstanceId> items;
    gameplay::commerce::legacy::AppendLegacyReservationItem(
        items, packet.DesCraftItem.sItemInfo.CODE, packet.DesCraftItem.sItemInfo.ItemHeader.Head,
        packet.DesCraftItem.sItemInfo.ItemHeader.dwChkSum);
    for(int index = 0; index < 12; ++index)
        gameplay::commerce::legacy::AppendLegacyReservationItem(
            items, packet.SheltomCode[index], packet.Head[index], packet.CheckSum[index]);
    gameplay::commerce::legacy::AppendLegacyReservationItem(
        items, packet.A_StoneItemCode, packet.A_StoneHead, packet.A_StoneCheckSum);
    return items;
}
}

namespace
{
bool RestoreAgingInventoryIfCaptured(gameplay::inventory::InventorySnapshotRollback& rollback)
{
    if(!rollback.Captured())
        return true;
    return rollback.Restore().Restored();
}
}

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
        LegacyCommerceReservationGuard reservation(request.player, AgingApplyReservations(packet));
        if(!reservation.Accepted())
        {
            result.code = AgingCode::InventoryReservationRejected;
            return result;
        }
        inventory::legacy::LegacyInventoryRuntime& inventoryRuntime = inventory::legacy::ServerLegacyInventoryRuntime();
        inventory::InventorySnapshotRollback rollback(inventoryRuntime, inventoryRuntime, request.player);
        const int legacyProcessed = rsRecvAgingItem(player, &packet);
        if(legacyProcessed == TRUE && sAging_Recv.Result != FALSE)
        {
            result.code = AgingCode::Processed;
            return result;
        }
        result.code = RestoreAgingInventoryIfCaptured(rollback) ? AgingCode::Rejected : AgingCode::RollbackFailed;
        return result;
    }

    if(request.requestBytes.size() != sizeof(TRANS_ITEMINFO))
    {
        result.code = AgingCode::InvalidPayload;
        return result;
    }
    TRANS_ITEMINFO packet;
    std::memcpy(&packet, request.requestBytes.data(), sizeof(packet));
    std::vector<inventory::ItemInstanceId> items;
    AppendLegacyReservationItem(items, packet.Item.CODE, packet.Item.ItemHeader.Head, packet.Item.ItemHeader.dwChkSum);
    LegacyCommerceReservationGuard reservation(request.player, items);
    if(!reservation.Accepted())
    {
        result.code = AgingCode::InventoryReservationRejected;
        return result;
    }
    inventory::legacy::LegacyInventoryRuntime& inventoryRuntime = inventory::legacy::ServerLegacyInventoryRuntime();
    inventory::InventorySnapshotRollback rollback(inventoryRuntime, inventoryRuntime, request.player);
    if(rsRecvAgingUpgradeItem(player, &packet) == TRUE)
    {
        result.code = AgingCode::Processed;
        return result;
    }
    result.code = RestoreAgingInventoryIfCaptured(rollback) ? AgingCode::Rejected : AgingCode::RollbackFailed;
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
