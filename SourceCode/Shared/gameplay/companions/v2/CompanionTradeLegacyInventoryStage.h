#pragma once
// Client-only application boundary for a VERIFIED durable trade after-image.
// Stages both real legacy bag arrays in temporary memory; makes NO changes
// to live state, skills, stats, money, equipment, cursor or character records.
// The caller must first validate an authenticated journal receipt, enforce a
// live inventory mutation freeze, and later perform the atomic host swap.
#include "gameplay/companions/v2/CompanionTradeInventorySnapshot.h"
#include "gameplay/companions/v2/CompanionTradeBoundedItemCodec.h"
#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

namespace gameplay { namespace companions { namespace v2 {

enum class TradeClientStageStatus : std::uint8_t {
    Ready, UnsafeSnapshot, InvalidCapacity, InvalidOrdering,
    DecodeRejected, IdentityMismatch, InvalidSlot, DuplicateSlot, DuplicateIdentity
};

template<class LegacyItem>
struct TradeClientStagedBags final {
    std::vector<LegacyItem> primary, secondary;
    // One byte per legacy item index; stage consumers must not assume that
    // all array entries are filled.
    std::vector<std::uint8_t> primaryUsed, secondaryUsed;
};

struct CompanionTradeLegacyInventoryStage final {
    // Decode translates an already strictly bounded full compressed
    // sRECORD_ITEM into a legacy sITEM (Flag, sItemInfo, x/y, position),
    // plus its ORIGINAL ItemCount array slot. It is a trusted LOCAL callback,
    // not a decoder supplied by a packet. Decode must avoid side effects.
    template<class LegacyItem, class Decode>
    static TradeClientStageStatus Prepare(
        const CompanionTradeInventorySnapshot& snapshot,
        std::size_t slotsPerBag, Decode decode,
        TradeClientStagedBags<LegacyItem>& output)
    {
        if (!slotsPerBag || slotsPerBag > 4096 ||
            snapshot.records.size() > slotsPerBag * 2u)
            return TradeClientStageStatus::InvalidCapacity;
        if (!snapshot.TradeEligible())
            return TradeClientStageStatus::UnsafeSnapshot;
        if (snapshot.records.size() !=
             static_cast<std::size_t>(snapshot.primaryCount) + snapshot.secondaryCount)
            return TradeClientStageStatus::InvalidOrdering;

        TradeClientStagedBags<LegacyItem> staged;
        staged.primary.resize(slotsPerBag);
        staged.secondary.resize(slotsPerBag);
        staged.primaryUsed.assign(slotsPerBag,0);
        staged.secondaryUsed.assign(slotsPerBag,0);
        for (std::size_t i=0; i<snapshot.records.size(); ++i) {
            const auto& record = snapshot.records[i];
            if (record.recordIndex != i ||
                record.inSecondaryBag != (i >= snapshot.primaryCount))
                return TradeClientStageStatus::InvalidOrdering;
            if (record.position < 0 || record.position > 16 ||
                !record.itemCode ||
                !record.itemCode)
                return TradeClientStageStatus::UnsafeSnapshot;
            // Do not rely solely on the snapshot's summary flags: the stage
            // must refuse a repeated complete instance even if an upstream
            // callback forgot to mark the duplicate.
            for (std::size_t j=0; j<i; ++j) {
                const auto& prev=snapshot.records[j];
                if (HasCompleteTradeIdentity350O(record.itemCode,record.itemKey,
                                                   record.itemChecksum) &&
                    prev.itemCode==record.itemCode &&
                    prev.itemKey==record.itemKey &&
                    prev.itemChecksum==record.itemChecksum)
                    return TradeClientStageStatus::DuplicateIdentity;
            }
            // The decoder itself must verify the full exact compressed frame.
            LegacyItem item{};
            std::size_t index=slotsPerBag;
            CompanionTradeDecodedItem identity;
            if (!decode(record.encodedRecord.data(),record.encodedRecord.size(),
                        item,index,identity))
                return TradeClientStageStatus::DecodeRejected;
            if (identity.code != record.itemCode ||
                identity.key != record.itemKey ||
                identity.checksum != record.itemChecksum ||
                identity.position != record.position ||
                identity.x != record.x || identity.y != record.y)
                return TradeClientStageStatus::IdentityMismatch;
            if (index >= slotsPerBag)
                return TradeClientStageStatus::InvalidSlot;
            auto& items=record.inSecondaryBag?staged.secondary:staged.primary;
            auto& used=record.inSecondaryBag?staged.secondaryUsed:staged.primaryUsed;
            if (used[index]) return TradeClientStageStatus::DuplicateSlot;
            items[index]=std::move(item);
            used[index]=1;
        }
        output=std::move(staged);
        return TradeClientStageStatus::Ready;
    }
};

}}} // gameplay::companions::v2
