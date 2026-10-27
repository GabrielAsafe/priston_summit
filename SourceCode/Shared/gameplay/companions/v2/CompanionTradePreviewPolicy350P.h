#pragma once
// SUMMIT350P: read-only presentation policy for trade pages.
// Passive anonymous/stacked records are visible but NEVER selectable.
// A recent in-memory accepted record can annotate the ONLINE identity ledger
// with actual equip positions. This does not authorize a commit or read a DAT.
#include "gameplay/companions/v2/CompanionTradeRevision.h"
#include <cstdint>
#include <map>
#include <tuple>

namespace gameplay { namespace companions { namespace v2 {

inline bool CanListTradePreview350P(CompanionTradeInventoryReadCode result,
                                   const CompanionTradeInventorySnapshot& snapshot) {
    return result == CompanionTradeInventoryReadCode::Ok &&
           snapshot.TradeEligible() &&
           snapshot.primaryCount + static_cast<std::uint64_t>(snapshot.secondaryCount) ==
               snapshot.records.size();
}

class TradePreviewPositionIndex350P final {
public:
    explicit TradePreviewPositionIndex350P(const CompanionTradeInventorySnapshot* source)
      : available_(source && source->TradeEligible()) {
        if(!available_) return;
        for(const auto& r : source->records) {
            if(!HasCompleteTradeIdentity350O(r.itemCode,r.itemKey,r.itemChecksum))
                continue;
            const Key key(r.itemCode,r.itemKey,r.itemChecksum);
            const auto inserted=positions_.insert(std::make_pair(key,r.position));
            if(!inserted.second) inserted.first->second=-2;
        }
    }
    int PositionOrBag350P(std::uint32_t code,std::uint32_t key,
                          std::uint32_t checksum) const {
        if(!available_) return 0; // No accepted record yet: old preview policy.
        const auto at=positions_.find(Key(code,key,checksum));
        return at==positions_.end() ? -2 : at->second;
    }
private:
    using Key = std::tuple<std::uint32_t,std::uint32_t,std::uint32_t>;
    bool available_;
    std::map<Key,int> positions_;
};

}}} // gameplay::companions::v2
