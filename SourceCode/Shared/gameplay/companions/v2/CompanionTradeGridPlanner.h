#pragma once
// Computes conflict-free V2 primary bag destinations for a two-way exchange.
// Does not authorize client packets, persist or mutate the active session.
#include "CompanionTradeItemMove.h"
#include "gameplay/inventory/v2/CompanionBagSpacePolicy.h"
#include "gameplay/inventory/v2/ItemFootprint.h"
#include <vector>
namespace gameplay { namespace companions { namespace v2 {
struct TradeSelectedItem {
    TradeSide from;
    std::uint32_t recordIndex, code, key, checksum;
};

template<class Authorize, class Relocate>
TradePlanResult PlanPrimaryBagExchange(
    const CompanionTradeInventorySnapshot& owner,
    const CompanionTradeInventorySnapshot& companion,
    const std::vector<TradeSelectedItem>& selected,
    const gameplay::inventory::IItemFootprintCatalog& catalog,
    const TradeLimits& limits, Authorize itemAuthorized,
    Relocate reencode, TradePlan& output)
{
    namespace inv=gameplay::inventory;
    if(selected.empty() || selected.size()>limits.maxMoves ||
       !TradeSnapshotStrict(owner,limits) || !TradeSnapshotStrict(companion,limits))
        return TradePlanResult::UnsafeSnapshot;
    std::vector<bool> chosenOwner(owner.records.size(),false);
    std::vector<bool> chosenCompanion(companion.records.size(),false);
    for(const auto& pick:selected) {
        if(pick.from!=TradeSide::Player && pick.from!=TradeSide::Companion)
            return TradePlanResult::Invalid;
        const bool srcOwner=pick.from==TradeSide::Player;
        const auto& source=srcOwner?owner:companion;
        auto& chosen=srcOwner?chosenOwner:chosenCompanion;
        if(pick.recordIndex>=source.records.size())return TradePlanResult::StaleSelection;
        if(chosen[pick.recordIndex])return TradePlanResult::DuplicateItem;
        const auto& r=source.records[pick.recordIndex];
        if(r.itemCode!=pick.code || r.itemKey!=pick.key || r.itemChecksum!=pick.checksum)
            return TradePlanResult::StaleSelection;
        if(r.position!=0 || r.inSecondaryBag || r.x>=10000)
            return TradePlanResult::EquippedItem;
        if(!HasCompleteTradeIdentity350O(r.itemCode,r.itemKey,r.itemChecksum))
            return TradePlanResult::Invalid;
        if(!itemAuthorized(r)) return TradePlanResult::Invalid;
        chosen[pick.recordIndex]=true;
    }
    std::vector<inv::BagRect> ownerUsed,companionUsed;
    const auto occupancy=[&](const CompanionTradeInventorySnapshot& s,
            const std::vector<bool>& chosen,std::vector<inv::BagRect>& occupied) {
        for(std::size_t i=0;i<s.records.size();++i) {
            const auto& r=s.records[i];
            if(chosen[i] || r.inSecondaryBag || r.position!=0 || r.x>=10000)
                continue;
            inv::ItemFootprint footprint;
            const gameplay::ItemId id(r.itemCode);
            if(!catalog.TryGetFootprint(id,footprint) || !footprint.ValidFor(id))return false;
            occupied.emplace_back(r.x,r.y,footprint.widthPixels,footprint.heightPixels);
        }
        // Calling FindFree triggers validation for corrupt old placements.
        return inv::CompanionBagSpacePolicy::FindFree(occupied,1,1).status!=
               inv::BagSpaceStatus::CorruptPlacement;
    };
    if(!occupancy(owner,chosenOwner,ownerUsed) ||
       !occupancy(companion,chosenCompanion,companionUsed))
        return TradePlanResult::PlacementInvalid;
    std::vector<TradeMove> moves;
    for(const auto& pick:selected) {
        const auto& source=pick.from==TradeSide::Player?owner:companion;
        const auto& r=source.records[pick.recordIndex];
        inv::ItemFootprint fp;
        const gameplay::ItemId item(r.itemCode);
        if(!catalog.TryGetFootprint(item,fp) || !fp.ValidFor(item))
            return TradePlanResult::PlacementInvalid;
        auto& occupied=pick.from==TradeSide::Player?companionUsed:ownerUsed;
        const auto fit=inv::CompanionBagSpacePolicy::FindFree(
            occupied,fp.widthPixels,fp.heightPixels);
        if(!fit.Available())return TradePlanResult::PlacementInvalid;
        occupied.emplace_back(fit.x,fit.y,fp.widthPixels,fp.heightPixels);
        moves.push_back({pick.from,pick.recordIndex,pick.code,pick.key,pick.checksum,
                         false,fit.x,fit.y});
    }
    return PlanItemMoves(owner,companion,moves,limits,reencode,output);
}
}}}
