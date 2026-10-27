#pragma once
// Builds the ACTUAL two character-record after-images for a transfer.
// Unlike preview/capture helpers this changes the encoded item allocation in
// both complete record images. The caller must use authenticated snapshots,
// hold the external session+DAT lease, and commit BOTH images via the journal.
#include "gameplay/companions/v2/CompanionTradeGridPlanner.h"
#include "gameplay/companions/v2/CompanionTradeRecordImageBuilder.h"
#include "gameplay/companions/v2/CompanionTradeJournal.h"
#include <vector>
#include <cstdio>

namespace gameplay { namespace companions { namespace v2 {

enum class ConcreteTransferCode {
    Ready, InvalidOffer, InvalidSelection, BadSource, NoRoom,
    EncodeFailed, ImageBuildFailed
};
// Stable, privacy-free reasons when the selection is rejected before planning.
// Distinguish an invalid UI selection from an otherwise unsafe full snapshot.
enum class TradeSelectionReject350N : int {
    None=0, InvalidBounds=1, UnsafeOwner=2, UnsafeCompanion=3,
    InvalidSide=4, StaleOrDuplicateIndex=5, WrongIdentity=6,
    Equipped=7, InvalidCoordinates=8, UnsupportedPotion=9,
    NotAuthorized=10, MissingInstanceIdentity=11
};

// Privacy-free stage for source-inventory diagnostic only. A reported
// index is the ordinal within an authenticated source record, not an item ID.
enum class TradeBadSource351I : int {
    None=0, UnreadableBagSlot=1, SlotOutOfRange=2, DuplicateBagSlot=3,
    UnknownFootprint=4, InvalidBagPlacement=5, SelectedUnknownFootprint=6
};

struct ConcreteTransferResult {
    ConcreteTransferCode code = ConcreteTransferCode::BadSource;
    TradePlanResult planning = TradePlanResult::Invalid;
    TradeImageResult ownerImage = TradeImageResult::BadSourceImage;
    TradeImageResult companionImage = TradeImageResult::BadSourceImage;
    TradeSelectionReject350N selectionReject = TradeSelectionReject350N::None;
    TradeBadSource351I badSource351I = TradeBadSource351I::None;
    int badSourceSide351I = -1; // 0=online owner, 1=offline companion
    std::uint32_t badSourceRecord351I = 0;
    int badSourcePeer351K = -1; // ordinal of intersecting record, not identity
};

// The source may be in either bag. The target is the first free fitting cell
// in bag A, then B. Whole-item footprints are respected on BOTH sides.
// The reencode callback MUST reconstruct an original decoded sRECORD_ITEM
// with the server-picked x/y and bag, preserving code/key/checksum.
template<class Authorize, class ReadSlot, class Relocate>
ConcreteTransferResult BuildConcreteTradeTransfer(
    const CompanionTradeOffer& offer,
    const std::vector<std::uint8_t>& ownerBytes,
    const std::vector<std::uint8_t>& companionBytes,
    const CompanionTradeInventorySnapshot& owner,
    const CompanionTradeInventorySnapshot& companion,
    const TradeRecordLayout& ownerLayout,
    const TradeRecordLayout& companionLayout,
    const std::vector<TradeSelectedItem>& selected,
    const gameplay::inventory::IItemFootprintCatalog& footprintCatalog,
    const TradeLimits& limits, std::size_t slotsPerBag,
    Authorize authorize, ReadSlot readSlot, Relocate reencode,
    CompanionTradePrepared& prepared)
{
    namespace inv = gameplay::inventory;
    ConcreteTransferResult status;
    if(!offer.tradeNonce || !offer.authenticatedOwner || !offer.companionCharacter ||
       offer.authenticatedOwner == offer.companionCharacter ||
       !offer.offerDigest || !offer.offerRevision) {
        status.code=ConcreteTransferCode::InvalidOffer;return status;
    }
    if(!slotsPerBag || slotsPerBag>4096 ||
       selected.empty() || selected.size()>limits.maxMoves) {
        status.code=ConcreteTransferCode::InvalidSelection;
        status.selectionReject=TradeSelectionReject350N::InvalidBounds;return status;
    }
    if(!TradeSnapshotStrict(owner,limits)) {
        status.code=ConcreteTransferCode::InvalidSelection;
        status.selectionReject=TradeSelectionReject350N::UnsafeOwner;return status;
    }
    if(!TradeSnapshotStrict(companion,limits)) {
        status.code=ConcreteTransferCode::InvalidSelection;
        status.selectionReject=TradeSelectionReject350N::UnsafeCompanion;return status;
    }
    std::vector<bool> takeOwner(owner.records.size(), false);
    std::vector<bool> takeCompanion(companion.records.size(), false);
    for(const auto& pick:selected) {
        if(pick.from!=TradeSide::Player && pick.from!=TradeSide::Companion) {
            status.code=ConcreteTransferCode::InvalidSelection;
            status.selectionReject=TradeSelectionReject350N::InvalidSide;return status;
        }
        const bool fromOwner=pick.from==TradeSide::Player;
        const auto& source=fromOwner?owner:companion;
        auto& taken=fromOwner?takeOwner:takeCompanion;
        if(pick.recordIndex>=source.records.size() || taken[pick.recordIndex]) {
            status.code=ConcreteTransferCode::InvalidSelection;
            status.selectionReject=TradeSelectionReject350N::StaleOrDuplicateIndex;
            return status;
        }
        const auto& r=source.records[pick.recordIndex];
        // Do not guess at the selected item; wrong identity must fail closed.
        if(r.itemCode!=pick.code || r.itemKey!=pick.key ||
           r.itemChecksum!=pick.checksum) {
            status.code=ConcreteTransferCode::InvalidSelection;
            status.selectionReject=TradeSelectionReject350N::WrongIdentity;
            return status;
        }
        if(r.position!=0) {
            status.code=ConcreteTransferCode::InvalidSelection;
            status.selectionReject=TradeSelectionReject350N::Equipped;
            return status;
        }
        if(r.x>=10000 || r.x<0 || r.y<0) {
            status.code=ConcreteTransferCode::InvalidSelection;
            status.selectionReject=TradeSelectionReject350N::InvalidCoordinates;
            return status;
        }
        if(IsLegacyPotionStack350N(r.itemCode)) {
            status.code=ConcreteTransferCode::InvalidSelection;
            status.selectionReject=TradeSelectionReject350N::UnsupportedPotion;
            return status;
        }
        if(!HasCompleteTradeIdentity350O(r.itemCode,r.itemKey,r.itemChecksum)) {
            status.code=ConcreteTransferCode::InvalidSelection;
            status.selectionReject=TradeSelectionReject350N::MissingInstanceIdentity;
            return status;
        }
        if(!authorize(r,pick.from)) {
            status.code=ConcreteTransferCode::InvalidSelection;
            status.selectionReject=TradeSelectionReject350N::NotAuthorized;
            return status;
        }
        taken[pick.recordIndex]=true;
    }
    // 0 = bag A; 1 = bag B. Includes both participants' unaffected items.
    std::vector<inv::BagRect> ownerUsed[2], companionUsed[2];
    std::vector<bool> ownerSlots[2]={std::vector<bool>(slotsPerBag,false),
                                     std::vector<bool>(slotsPerBag,false)};
    std::vector<bool> companionSlots[2]={std::vector<bool>(slotsPerBag,false),
                                         std::vector<bool>(slotsPerBag,false)};
    const auto populate=[&](const CompanionTradeInventorySnapshot& snapshot,
                            const std::vector<bool>& taken,
                            std::vector<inv::BagRect> (&used)[2],
                            std::vector<bool> (&slots)[2],int diagnosticSide) {
        std::vector<std::uint32_t> usedOrdinals[2];
        for(std::size_t i=0;i<snapshot.records.size();++i) {
            const auto& r=snapshot.records[i];
            if(taken[i]) continue;
            // Only BAG items consume bag grid/slot indices. An equipped
            // sRECORD_ITEM may use ItemCount=-1 or a non-bag value; rejecting
            // it here made all transfers fail on normal equipped characters.
            if(r.position!=0) continue;
            std::uint32_t slot=0;
            if(!readSlot(r,slot)) {
                status.badSource351I=TradeBadSource351I::UnreadableBagSlot;
                status.badSourceSide351I=diagnosticSide;
                status.badSourceRecord351I=static_cast<std::uint32_t>(i);
                return false;
            }
            if(slot>=slotsPerBag) {
                status.badSource351I=TradeBadSource351I::SlotOutOfRange;
                status.badSourceSide351I=diagnosticSide;
                status.badSourceRecord351I=static_cast<std::uint32_t>(i);
                return false;
            }
            if(slots[r.inSecondaryBag?1:0][slot]) {
                status.badSource351I=TradeBadSource351I::DuplicateBagSlot;
                status.badSourceSide351I=diagnosticSide;
                status.badSourceRecord351I=static_cast<std::uint32_t>(i);
                return false;
            }
            slots[r.inSecondaryBag?1:0][slot]=true;
            // Legacy inventory may persist out-of-grid passive records with
            // x >= 10000. The older read-only planner excludes these from
            // occupancy, and the selection guard above refuses moving them.
            // Preserve their full record, identity and reserved ItemCount,
            // but do not guess a footprint or count them as a visible rect.
            if(r.x>=10000) {
                std::printf("[SUMMIT351K_TRADE] stage=offgrid-preserved side=%d record=%u code=0x%08x\n",
                    diagnosticSide,static_cast<unsigned>(i),
                    static_cast<unsigned>(r.itemCode));
                continue;
            }
            inv::ItemFootprint f;
            const gameplay::ItemId id(r.itemCode);
            if(!footprintCatalog.TryGetFootprint(id,f) || !f.ValidFor(id)) {
                status.badSource351I=TradeBadSource351I::UnknownFootprint;
                status.badSourceSide351I=diagnosticSide;
                status.badSourceRecord351I=static_cast<std::uint32_t>(i);
                return false;
            }
            const int bag=r.inSecondaryBag?1:0;
            used[bag].emplace_back(r.x,r.y,f.widthPixels,f.heightPixels);
            usedOrdinals[bag].push_back(static_cast<std::uint32_t>(i));
        }
        for(int bag=0;bag<2;++bag)
            if(inv::CompanionBagSpacePolicy::FindFree(used[bag],1,1).status==
                  inv::BagSpaceStatus::CorruptPlacement) {
                status.badSource351I=TradeBadSource351I::InvalidBagPlacement;
                status.badSourceSide351I=diagnosticSide;
                // Report the actual record/peer that failed; the prior
                // diagnostic always printed placeholder record=0.
                for(std::size_t i=0;i<used[bag].size();++i) {
                    if(!inv::CompanionBagSpacePolicy::InBounds(used[bag][i])) {
                        status.badSourceRecord351I=usedOrdinals[bag][i];
                        return false;
                    }
                    for(std::size_t j=0;j<i;++j)
                        if(inv::CompanionBagSpacePolicy::Intersects(
                               used[bag][i],used[bag][j])) {
                            status.badSourceRecord351I=usedOrdinals[bag][i];
                            status.badSourcePeer351K=
                                static_cast<int>(usedOrdinals[bag][j]);
                            return false;
                        }
                }
                // Defensive: never turn unexpected corruption into success.
                status.badSourceRecord351I=0;
                return false;
            }
        return true;
    };
    if(!populate(owner,takeOwner,ownerUsed,ownerSlots,0) ||
       !populate(companion,takeCompanion,companionUsed,companionSlots,1)) {
        status.code=ConcreteTransferCode::BadSource;
        // Report the already-determined reason directly at the rejection
        // site; no changes needed in LegacyCharacterRecordAdapter.cpp.
        // Contains no item identity, DAT data or player name.
        std::printf("[SUMMIT351J_TRADE] stage=bad-source-detail reason=%d side=%d record=%u\n",
            static_cast<int>(status.badSource351I),status.badSourceSide351I,
            static_cast<unsigned int>(status.badSourceRecord351I));
        const auto& failing=status.badSourceSide351I==0?owner:companion;
        if(status.badSourceRecord351I<failing.records.size()) {
            const auto& r=failing.records[status.badSourceRecord351I];
            std::printf("[SUMMIT351K_TRADE] stage=bad-source-context reason=%d side=%d record=%u code=0x%08x bag=%c x=%d y=%d peer=%d\n",
                static_cast<int>(status.badSource351I),status.badSourceSide351I,
                static_cast<unsigned>(status.badSourceRecord351I),
                static_cast<unsigned>(r.itemCode),r.inSecondaryBag?'B':'A',
                static_cast<int>(r.x),static_cast<int>(r.y),status.badSourcePeer351K);
        }
        return status;
    }
    std::vector<TradeMove> moves;
    std::vector<std::uint32_t> allocatedSlots;
    moves.reserve(selected.size());
    allocatedSlots.reserve(selected.size());
    for(const auto& pick:selected) {
        const auto& source=pick.from==TradeSide::Player?owner:companion;
        const auto& r=source.records[pick.recordIndex];
        inv::ItemFootprint f;
        const gameplay::ItemId item(r.itemCode);
        if(!footprintCatalog.TryGetFootprint(item,f) || !f.ValidFor(item)) {
            status.badSource351I=TradeBadSource351I::SelectedUnknownFootprint;
            status.badSourceSide351I=pick.from==TradeSide::Player?0:1;
            status.badSourceRecord351I=pick.recordIndex;
            status.code=ConcreteTransferCode::BadSource;
            std::printf("[SUMMIT351J_TRADE] stage=bad-source-detail reason=%d side=%d record=%u\n",
                static_cast<int>(status.badSource351I),status.badSourceSide351I,
                static_cast<unsigned int>(status.badSourceRecord351I));
            std::printf("[SUMMIT351K_TRADE] stage=bad-source-context reason=%d side=%d record=%u code=0x%08x bag=%c x=%d y=%d peer=-1\n",
                static_cast<int>(status.badSource351I),status.badSourceSide351I,
                static_cast<unsigned>(status.badSourceRecord351I),
                static_cast<unsigned>(r.itemCode),r.inSecondaryBag?'B':'A',
                static_cast<int>(r.x),static_cast<int>(r.y));
            return status;
        }
        auto& occupied=pick.from==TradeSide::Player?companionUsed:ownerUsed;
        auto& destinationSlots=pick.from==TradeSide::Player?companionSlots:ownerSlots;
        bool placed=false;
        for(int bag=0;bag<2 && !placed;++bag) {
            auto fit=inv::CompanionBagSpacePolicy::FindFree(
                occupied[bag],f.widthPixels,f.heightPixels);
            if(!fit.Available())continue;
            std::size_t freeSlot=0;
            while(freeSlot<slotsPerBag && destinationSlots[bag][freeSlot])
                ++freeSlot;
            if(freeSlot==slotsPerBag)continue;
            destinationSlots[bag][freeSlot]=true;
            allocatedSlots.push_back(static_cast<std::uint32_t>(freeSlot));
            occupied[bag].emplace_back(fit.x,fit.y,f.widthPixels,f.heightPixels);
            moves.push_back({pick.from,pick.recordIndex,pick.code,pick.key,
                             pick.checksum,bag==1,fit.x,fit.y});
            placed=true;
        }
        if(!placed) {status.code=ConcreteTransferCode::NoRoom;return status;}
    }
    TradePlan plan;
    std::size_t moveOrdinal=0;
    const auto relocateWithSlot=[&](const CompanionTradeItemRecord& from,
        bool secondary,int x,int y,CompanionTradeItemRecord& target) {
        if(moveOrdinal>=allocatedSlots.size())return false;
        const std::uint32_t slot=allocatedSlots[moveOrdinal++];
        return reencode(from,secondary,x,y,slot,target);
    };
    status.planning=PlanItemMoves(owner,companion,moves,limits,relocateWithSlot,plan);
    if(status.planning!=TradePlanResult::Ready) {
        status.code=ConcreteTransferCode::EncodeFailed;return status;
    }
    std::vector<std::uint8_t> ownerAfter,companionAfter;
    status.ownerImage=BuildTradeRecordImage(ownerBytes,owner,plan.playerAfter,
                                            ownerLayout,limits.maxItems,ownerAfter);
    status.companionImage=BuildTradeRecordImage(companionBytes,companion,
                                     plan.companionAfter,companionLayout,
                                     limits.maxItems,companionAfter);
    if(status.ownerImage!=TradeImageResult::Ready ||
       status.companionImage!=TradeImageResult::Ready ||
       ownerBytes==ownerAfter || companionBytes==companionAfter) {
        status.code=ConcreteTransferCode::ImageBuildFailed;return status;
    }
    CompanionTradePrepared built;
    built.offer=offer;
    built.ownerBefore=ownerBytes;
    built.ownerAfter=std::move(ownerAfter);
    built.companionBefore=companionBytes;
    built.companionAfter=std::move(companionAfter);
    prepared=std::move(built);
    status.code=ConcreteTransferCode::Ready;
    return status;
}

}}} // gameplay::companions::v2
