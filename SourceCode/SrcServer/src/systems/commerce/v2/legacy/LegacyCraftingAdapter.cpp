#include "systems/commerce/v2/legacy/LegacyCraftingAdapter.h"

#include <cstring>

#include "systems/network/transport/legacy/smwsock.h"
#include "systems/rendering/legacy_3d/smd3d.h"
#include "systems/gameplay/character/core/character.h"
#include "core/server/legacy_runtime/onserver.h"
#include "systems/commerce/legacy/sinTrade.h"
#include "systems/gameplay/character/legacy_ui/sinSkinChange.h"

int rsRecvCraftItem(rsPLAYINFO* lpPlayInfo, sCRAFTITEM_SERVER* pCraftItem_Server);
int rsRecvMakeLinkCore(rsPLAYINFO* lpPlayInfo, TRANS_ITEMINFO* lpTransItemInfo);
int rsRecvUseLinkCore(rsPLAYINFO* lpPlayInfo, TRANS_ITEMINFO* lpTransItemInfo);
int rsRecvWingItem(rsPLAYINFO* lpPlayInfo, sCRAFTITEM_SERVER* pCraftItem_Server);
int rsRecvSkinChangeItem(rsPLAYINFO* lpPlayInfo, sSKIN_CHANGE_SERVER* pSkinChangeItem_Server);
int rsRecvSmeltingItem(rsPLAYINFO* lpPlayInfo, sSMELTINGITEM_SERVER* pSmeltingItem_Server);
int rsRecvManufactureItem(rsPLAYINFO* lpPlayInfo, SManufactureItem_Server* pManufactureItem_Server);

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
    result.code = rsRecvCraftItem(player, &packet) == TRUE ? CraftingCode::Processed : CraftingCode::Rejected;
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
            processed = request.operation == CraftingAuxiliaryOperation::MakeLinkCore
                ? rsRecvMakeLinkCore(player, &packet)
                : rsRecvUseLinkCore(player, &packet);
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
            processed = rsRecvWingItem(player, &packet);
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
            processed = rsRecvSkinChangeItem(player, &packet);
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
            processed = rsRecvSmeltingItem(player, &packet);
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
            processed = rsRecvManufactureItem(player, &packet);
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
