#pragma once
// SUMMIT351F: companion LIVE after-image reconciliation.
// This header is separate on purpose. Do NOT replace the existing
// CompanionTradeLegacyInventoryStage.h: production installs may have
// legitimate changes to that header.
// Only call after authenticated, journal-committed live delivery and
// with the client inventory mutation freeze held.
#include "gameplay/companions/v2/CompanionTradeLegacyInventoryStage.h"
#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

namespace gameplay { namespace companions { namespace v2 {

    // SUMMIT351F: standalone LIVE-only fallback; preserves the installed legacy header.
    // A bag item (position==0) owns its original ItemCount slot. Equipped
    // records may reuse that index in legacy DATs; place those equipped records
    // into still-unused sITEM array cells, without changing their equipment
    // position, coordinates, item identity or payload. A genuine duplicate
    // bag slot or duplicate item instance continues to be rejected.
    // Regular Prepare remains strict for callers that require exact indices.
template<class LegacyItem, class Decode>
inline TradeClientStageStatus PrepareCommittedLiveInventory351F(
        const CompanionTradeInventorySnapshot& snapshot,
        std::size_t slotsPerBag, Decode decode,
        TradeClientStagedBags<LegacyItem>& output,
        std::size_t* equippedRelocations = nullptr)
    {
        if (equippedRelocations) *equippedRelocations=0;
        const auto strict=CompanionTradeLegacyInventoryStage::Prepare(snapshot,slotsPerBag,decode,output);
        // SUMMIT353: the strict stage also reports InvalidSlot for legitimate
        // equipped legacy indices (including -1). Only BAG slots need an
        // original array index. Reconcile equipment without inventing slots
        // for a bag record or replacing an existing item.
        if(strict!=TradeClientStageStatus::DuplicateSlot &&
           strict!=TradeClientStageStatus::InvalidSlot) return strict;
        if (!slotsPerBag || slotsPerBag>4096 ||
            snapshot.records.size()>slotsPerBag*2u)
            return TradeClientStageStatus::InvalidCapacity;
        if (!snapshot.TradeEligible())
            return TradeClientStageStatus::UnsafeSnapshot;
        if (snapshot.records.size()!=
              static_cast<std::size_t>(snapshot.primaryCount)+snapshot.secondaryCount)
            return TradeClientStageStatus::InvalidOrdering;

        struct Decoded {
            LegacyItem item{};
            std::size_t slot=0;
        };
        std::vector<Decoded> decoded(snapshot.records.size());
        // Decode/validate ALL records before touching the output, even if the
        // first failure was a collision. The decode callback is side-effect-free.
        for(std::size_t i=0;i<snapshot.records.size();++i) {
            const auto& r=snapshot.records[i];
            if(r.recordIndex!=i || r.inSecondaryBag!=(i>=snapshot.primaryCount))
                return TradeClientStageStatus::InvalidOrdering;
            if(r.position<0 || r.position>16 ||
               !r.itemCode || !r.itemKey || !r.itemChecksum)
                return TradeClientStageStatus::UnsafeSnapshot;
            for(std::size_t j=0;j<i;++j) {
                const auto& other=snapshot.records[j];
                if(other.itemCode==r.itemCode && other.itemKey==r.itemKey &&
                   other.itemChecksum==r.itemChecksum)
                    return TradeClientStageStatus::DuplicateIdentity;
            }
            CompanionTradeDecodedItem identity;
            if(!decode(r.encodedRecord.data(),r.encodedRecord.size(),
                       decoded[i].item,decoded[i].slot,identity))
                return TradeClientStageStatus::DecodeRejected;
            if(identity.code!=r.itemCode || identity.key!=r.itemKey ||
               identity.checksum!=r.itemChecksum ||
               identity.position!=r.position ||
               identity.x!=r.x || identity.y!=r.y)
                return TradeClientStageStatus::IdentityMismatch;
            if(r.position==0 && decoded[i].slot>=slotsPerBag)
                return TradeClientStageStatus::InvalidSlot;
        }
        TradeClientStagedBags<LegacyItem> staged;
        staged.primary.resize(slotsPerBag);
        staged.secondary.resize(slotsPerBag);
        staged.primaryUsed.assign(slotsPerBag,0);
        staged.secondaryUsed.assign(slotsPerBag,0);
        std::size_t relocated=0;
        // Two passes: reserve bag positions first. The legacy server also
        // reserves indices only for bag items in ConcreteTransfer::populate.
        // An equipment-only collision is not proof of duplicate ownership.
        for(unsigned pass=0;pass<2;++pass) {
            for(std::size_t i=0;i<snapshot.records.size();++i) {
                const auto& r=snapshot.records[i];
                const bool isBag = r.position==0;
                if(isBag!=(pass==0)) continue;
                auto& items=r.inSecondaryBag?staged.secondary:staged.primary;
                auto& used=r.inSecondaryBag?staged.secondaryUsed:staged.primaryUsed;
                std::size_t slot=decoded[i].slot;
                if(slot>=slotsPerBag || used[slot]) {
                    if(isBag) return TradeClientStageStatus::DuplicateSlot;
                    // Equipped records do not own the index in the DAT.
                    // Assign an unused client cell after reserving all bags.
                    slot=0;
                    while(slot<slotsPerBag && used[slot]) ++slot;
                    if(slot==slotsPerBag)
                        return TradeClientStageStatus::InvalidCapacity;
                    ++relocated;
                }
                items[slot]=std::move(decoded[i].item);
                used[slot]=1;
            }
        }
        output=std::move(staged);
        if(equippedRelocations) *equippedRelocations=relocated;
        return TradeClientStageStatus::Ready;
    }

}}} // gameplay::companions::v2
