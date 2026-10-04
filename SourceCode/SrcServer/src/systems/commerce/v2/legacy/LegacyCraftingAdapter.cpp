#include "systems/commerce/v2/legacy/LegacyCraftingAdapter.h"

#include <cstring>

#include "systems/network/transport/legacy/smwsock.h"
#include "systems/rendering/legacy_3d/smd3d.h"
#include "systems/gameplay/character/core/character.h"
#include "core/server/legacy_runtime/onserver.h"
#include "systems/commerce/legacy/sinTrade.h"
#include "systems/gameplay/character/legacy_ui/sinSkinChange.h"
#include "systems/commerce/v2/legacy/LegacyCommerceReservationGuard.h"
#include "gameplay/inventory/v2/InventorySnapshotRollback.h"
#include "systems/inventory/v2/legacy/LegacyInventoryRuntime.h"

int rsRecvCraftItem(rsPLAYINFO* lpPlayInfo, sCRAFTITEM_SERVER* pCraftItem_Server);
int rsRecvMakeLinkCore(rsPLAYINFO* lpPlayInfo, TRANS_ITEMINFO* lpTransItemInfo);
int rsRecvUseLinkCore(rsPLAYINFO* lpPlayInfo, TRANS_ITEMINFO* lpTransItemInfo);
int rsRecvWingItem(rsPLAYINFO* lpPlayInfo, sCRAFTITEM_SERVER* pCraftItem_Server);
int rsRecvSkinChangeItem(rsPLAYINFO* lpPlayInfo, sSKIN_CHANGE_SERVER* pSkinChangeItem_Server);
int rsRecvSmeltingItem(rsPLAYINFO* lpPlayInfo, sSMELTINGITEM_SERVER* pSmeltingItem_Server);
int rsRecvManufactureItem(rsPLAYINFO* lpPlayInfo, SManufactureItem_Server* pManufactureItem_Server);

namespace
{
std::vector<gameplay::inventory::ItemInstanceId> CraftPacketReservations(const sCRAFTITEM_SERVER& packet)
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

std::vector<gameplay::inventory::ItemInstanceId> SkinChangeReservations(const sSKIN_CHANGE_SERVER& packet)
{
    std::vector<gameplay::inventory::ItemInstanceId> items;
    gameplay::commerce::legacy::AppendLegacyReservationItem(
        items, packet.DesSkinChangeItem.sItemInfo.CODE, packet.DesSkinChangeItem.sItemInfo.ItemHeader.Head,
        packet.DesSkinChangeItem.sItemInfo.ItemHeader.dwChkSum);
    gameplay::commerce::legacy::AppendLegacyReservationItem(
        items, packet.dwSkinChangeStoneItemCode, packet.dwSkinChangeStoneHead, packet.dwSkinChangeStoneCheckSum);
    return items;
}

std::vector<gameplay::inventory::ItemInstanceId> SmeltingReservations(const sSMELTINGITEM_SERVER& packet)
{
    std::vector<gameplay::inventory::ItemInstanceId> items;
    for(int index = 0; index < 5; ++index)
        gameplay::commerce::legacy::AppendLegacyReservationItem(
            items, packet.SmeltingCode[index], packet.Head[index], packet.CheckSum[index]);
    gameplay::commerce::legacy::AppendLegacyReservationItem(
        items, packet.A_StoneItemCode, packet.A_StoneHead, packet.A_StoneCheckSum);
    return items;
}

std::vector<gameplay::inventory::ItemInstanceId> ManufactureReservations(const SManufactureItem_Server& packet)
{
    std::vector<gameplay::inventory::ItemInstanceId> items;
    for(int index = 0; index < 4; ++index)
        gameplay::commerce::legacy::AppendLegacyReservationItem(
            items, packet.RuneCode[index], packet.Head[index], packet.CheckSum[index]);
    gameplay::commerce::legacy::AppendLegacyReservationItem(
        items, packet.A_StoneItemCode, packet.A_StoneHead, packet.A_StoneCheckSum);
    return items;
}
}

namespace
{
bool RestoreInventoryIfCaptured(gameplay::inventory::InventorySnapshotRollback& rollback)
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
LegacyCraftingAdapter::LegacyCraftingAdapter(const player::legacy::LegacyOnlinePlayerRegistry& registry)
    : registry_(registry)
{
}

CraftResult LegacyCraftingAdapter::Craft(const CraftRequest& request)
{
    CraftResult result;
    result.player = request.player;

    rsPLAYINFO* player = registry_.Resolve(request.player);
    if(!player)
    {
        result.code = CraftingCode::InvalidPlayer;
        return result;
    }
    if(request.requestBytes.size() != sizeof(sCRAFTITEM_SERVER))
    {
        result.code = CraftingCode::InvalidPayload;
        return result;
    }

    sCRAFTITEM_SERVER packet;
    std::memcpy(&packet, request.requestBytes.data(), sizeof(packet));
    LegacyCommerceReservationGuard reservation(request.player, CraftPacketReservations(packet));
    if(!reservation.Accepted())
    {
        result.code = CraftingCode::InventoryReservationRejected;
        return result;
    }
    inventory::legacy::LegacyInventoryRuntime& inventoryRuntime = inventory::legacy::ServerLegacyInventoryRuntime();
    inventory::InventorySnapshotRollback rollback(inventoryRuntime, inventoryRuntime, request.player);
    const int legacyProcessed = rsRecvCraftItem(player, &packet);
    if(legacyProcessed == TRUE && sCraftItem_Recv.Result == TRUE)
    {
        result.code = CraftingCode::Processed;
        return result;
    }
    result.code = RestoreInventoryIfCaptured(rollback) ? CraftingCode::Rejected : CraftingCode::RollbackFailed;
    return result;
}


CraftingAuxiliaryResult LegacyCraftingAdapter::ProcessAuxiliary(const CraftingAuxiliaryRequest& request)
{
    CraftingAuxiliaryResult result;
    result.player = request.player;
    result.operation = request.operation;

    rsPLAYINFO* player = registry_.Resolve(request.player);
    if(!player)
    {
        result.code = CraftingCode::InvalidPlayer;
        return result;
    }

    int processed = FALSE;
    switch(request.operation)
    {
        case CraftingAuxiliaryOperation::MakeLinkCore:
        case CraftingAuxiliaryOperation::UseLinkCore:
        {
            if(request.requestBytes.size() != sizeof(TRANS_ITEMINFO))
            {
                result.code = CraftingCode::InvalidPayload;
                return result;
            }
            TRANS_ITEMINFO packet;
            std::memcpy(&packet, request.requestBytes.data(), sizeof(packet));
            if(request.operation == CraftingAuxiliaryOperation::MakeLinkCore)
            {
                std::vector<inventory::ItemInstanceId> items;
                AppendLegacyReservationItem(items, packet.Item.CODE, packet.Item.ItemHeader.Head, packet.Item.ItemHeader.dwChkSum);
                LegacyCommerceReservationGuard reservation(request.player, items);
                if(!reservation.Accepted())
                {
                    result.code = CraftingCode::InventoryReservationRejected;
                    return result;
                }
                inventory::legacy::LegacyInventoryRuntime& inventoryRuntime = inventory::legacy::ServerLegacyInventoryRuntime();
                inventory::InventorySnapshotRollback rollback(inventoryRuntime, inventoryRuntime, request.player);
                processed = rsRecvMakeLinkCore(player, &packet);
                if(processed != TRUE && !RestoreInventoryIfCaptured(rollback))
                {
                    result.code = CraftingCode::RollbackFailed;
                    return result;
                }
            }
            else
            {
                // UseLinkCore validates and routes an existing core but does not mutate inventory here.
                processed = rsRecvUseLinkCore(player, &packet);
            }
            break;
        }
        case CraftingAuxiliaryOperation::Wing:
        {
            if(request.requestBytes.size() != sizeof(sCRAFTITEM_SERVER))
            {
                result.code = CraftingCode::InvalidPayload;
                return result;
            }
            sCRAFTITEM_SERVER packet;
            std::memcpy(&packet, request.requestBytes.data(), sizeof(packet));
            LegacyCommerceReservationGuard reservation(request.player, CraftPacketReservations(packet));
            if(!reservation.Accepted())
            {
                result.code = CraftingCode::InventoryReservationRejected;
                return result;
            }
            inventory::legacy::LegacyInventoryRuntime& inventoryRuntime = inventory::legacy::ServerLegacyInventoryRuntime();
            inventory::InventorySnapshotRollback rollback(inventoryRuntime, inventoryRuntime, request.player);
            processed = rsRecvWingItem(player, &packet);
            if((processed != TRUE || packet.Result != TRUE) && !RestoreInventoryIfCaptured(rollback))
            {
                result.code = CraftingCode::RollbackFailed;
                return result;
            }
            if(packet.Result != TRUE)
                processed = FALSE;
            break;
        }
        case CraftingAuxiliaryOperation::SkinChange:
        {
            if(request.requestBytes.size() != sizeof(sSKIN_CHANGE_SERVER))
            {
                result.code = CraftingCode::InvalidPayload;
                return result;
            }
            sSKIN_CHANGE_SERVER packet;
            std::memcpy(&packet, request.requestBytes.data(), sizeof(packet));
            LegacyCommerceReservationGuard reservation(request.player, SkinChangeReservations(packet));
            if(!reservation.Accepted())
            {
                result.code = CraftingCode::InventoryReservationRejected;
                return result;
            }
            inventory::legacy::LegacyInventoryRuntime& inventoryRuntime = inventory::legacy::ServerLegacyInventoryRuntime();
            inventory::InventorySnapshotRollback rollback(inventoryRuntime, inventoryRuntime, request.player);
            processed = rsRecvSkinChangeItem(player, &packet);
            if((processed != TRUE || sSkinChange_Recv.Result != TRUE) && !RestoreInventoryIfCaptured(rollback))
            {
                result.code = CraftingCode::RollbackFailed;
                return result;
            }
            if(sSkinChange_Recv.Result != TRUE)
                processed = FALSE;
            break;
        }
        case CraftingAuxiliaryOperation::Smelting:
        {
            if(request.requestBytes.size() != sizeof(sSMELTINGITEM_SERVER))
            {
                result.code = CraftingCode::InvalidPayload;
                return result;
            }
            sSMELTINGITEM_SERVER packet;
            std::memcpy(&packet, request.requestBytes.data(), sizeof(packet));
            LegacyCommerceReservationGuard reservation(request.player, SmeltingReservations(packet));
            if(!reservation.Accepted())
            {
                result.code = CraftingCode::InventoryReservationRejected;
                return result;
            }
            inventory::legacy::LegacyInventoryRuntime& inventoryRuntime = inventory::legacy::ServerLegacyInventoryRuntime();
            inventory::InventorySnapshotRollback rollback(inventoryRuntime, inventoryRuntime, request.player);
            processed = rsRecvSmeltingItem(player, &packet);
            if((processed != TRUE || sSmeltingItem_Recv.Result != TRUE) && !RestoreInventoryIfCaptured(rollback))
            {
                result.code = CraftingCode::RollbackFailed;
                return result;
            }
            if(sSmeltingItem_Recv.Result != TRUE)
                processed = FALSE;
            break;
        }
        case CraftingAuxiliaryOperation::Manufacture:
        {
            if(request.requestBytes.size() != sizeof(SManufactureItem_Server))
            {
                result.code = CraftingCode::InvalidPayload;
                return result;
            }
            SManufactureItem_Server packet;
            std::memcpy(&packet, request.requestBytes.data(), sizeof(packet));
            LegacyCommerceReservationGuard reservation(request.player, ManufactureReservations(packet));
            if(!reservation.Accepted())
            {
                result.code = CraftingCode::InventoryReservationRejected;
                return result;
            }
            inventory::legacy::LegacyInventoryRuntime& inventoryRuntime = inventory::legacy::ServerLegacyInventoryRuntime();
            inventory::InventorySnapshotRollback rollback(inventoryRuntime, inventoryRuntime, request.player);
            processed = rsRecvManufactureItem(player, &packet);
            if((processed != TRUE || g_sManufactureItem_Recv.Result != TRUE) && !RestoreInventoryIfCaptured(rollback))
            {
                result.code = CraftingCode::RollbackFailed;
                return result;
            }
            if(g_sManufactureItem_Recv.Result != TRUE)
                processed = FALSE;
            break;
        }
    }

    result.code = processed == TRUE ? CraftingCode::Processed : CraftingCode::Rejected;
    return result;
}

LegacyCraftingAdapter& ServerLegacyCraftingAdapter()
{
    static LegacyCraftingAdapter adapter(player::legacy::ServerLegacyOnlinePlayerRegistry());
    return adapter;
}
}
}
}
