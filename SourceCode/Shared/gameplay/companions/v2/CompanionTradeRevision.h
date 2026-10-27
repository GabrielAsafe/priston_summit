#pragma once
// Read-only preview revision. Includes logical bag boundary and decoded
// position as well as the original encoded record. Not a cryptographic proof
// or transaction authorization; the server must revalidate under a lease.
#include "gameplay/companions/v2/CompanionTradeInventorySnapshot.h"
#include <cstdint>
#include <cstddef>
namespace gameplay { namespace companions { namespace v2 {

inline void TradeRevisionMix32(std::uint64_t& h, std::uint32_t v) {
    for (unsigned i=0; i<32; i+=8) {
        h ^= static_cast<std::uint8_t>(v>>i);
        h *= UINT64_C(1099511628211);
    }
}
inline std::uint64_t CompanionTradePreviewRevision(
    const CompanionTradeInventorySnapshot& s) {
    if (!s.TradeEligible() ||
        s.primaryCount+s.secondaryCount != s.records.size()) return 0;
    std::uint64_t h = UINT64_C(14695981039346656037);
    TradeRevisionMix32(h,s.primaryCount);
    TradeRevisionMix32(h,s.secondaryCount);
    for (std::size_t i=0;i<s.records.size();++i) {
        const auto& r=s.records[i];
        if(r.recordIndex!=i ||
           r.inSecondaryBag!=(i>=s.primaryCount) ||
           !r.itemCode ||
           // Anonymous legacy records are included verbatim in the hash.
           // They never become legal selected items.
           r.encodedRecord.size()<5) return 0;
        TradeRevisionMix32(h,r.recordIndex);
        TradeRevisionMix32(h,r.itemCode);
        TradeRevisionMix32(h,r.itemKey);
        TradeRevisionMix32(h,r.itemChecksum);
        TradeRevisionMix32(h,static_cast<std::uint32_t>(r.position));
        TradeRevisionMix32(h,static_cast<std::uint32_t>(r.x));
        TradeRevisionMix32(h,static_cast<std::uint32_t>(r.y));
        TradeRevisionMix32(h,r.inSecondaryBag?1:0);
        TradeRevisionMix32(h,static_cast<std::uint32_t>(r.encodedRecord.size()));
        for(const auto b:r.encodedRecord) {
            h ^= b;
            h *= UINT64_C(1099511628211);
        }
    }
    return h ? h : 1;
}
}}}
