#pragma once
// SUMMIT353: Make the already-committed inventory survive legacy LoadItemInfo.
// That renderer normalizes SpecialItemFlag and can rewrite display coordinates.
// Such cosmetic mutation must not turn the first post-save into a different
// item image. Never restore a missing/changed item or invent an identity.
// Call ONLY after a verified journal image is staged and installed while the
// client trade freeze is active; no persistence or server permission here.
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <vector>

namespace gameplay { namespace companions { namespace v2 {

enum class TradeLivePayloadPreserve353 : std::uint8_t {
    Preserved, InvalidCapacity, MissingOrUnexpected, IdentityChanged,
    EquipmentPositionChanged
};

template<class LegacyItem>
inline TradeLivePayloadPreserve353 PreserveCommittedPayloadAfterLoad353(
    const std::vector<LegacyItem>& expectedA,
    const std::vector<std::uint8_t>& usedA, LegacyItem* liveA,
    const std::vector<LegacyItem>& expectedB,
    const std::vector<std::uint8_t>& usedB, LegacyItem* liveB,
    std::size_t capacity) {
    if(!capacity || capacity>4096 || !liveA || !liveB ||
       expectedA.size()!=capacity || usedA.size()!=capacity ||
       expectedB.size()!=capacity || usedB.size()!=capacity)
        return TradeLivePayloadPreserve353::InvalidCapacity;
    // Validate BOTH bags before changing any live item, to avoid partially
    // repairing a corrupted UI result. Identity and equipment assignment
    // must still be exactly those authenticated in the committed journal.
    for(int bag=0;bag<2;++bag) {
        const auto& expected=bag==0?expectedA:expectedB;
        const auto& used=bag==0?usedA:usedB;
        const LegacyItem* live=bag==0?liveA:liveB;
        for(std::size_t i=0;i<capacity;++i) {
            const bool required=used[i]!=0;
            if((live[i].Flag!=0)!=required ||
               (required && expected[i].Flag==0))
                return TradeLivePayloadPreserve353::MissingOrUnexpected;
            if(!required) continue;
            if(live[i].ItemPosition!=expected[i].ItemPosition)
                return TradeLivePayloadPreserve353::EquipmentPositionChanged;
            if(live[i].sItemInfo.CODE!=expected[i].sItemInfo.CODE ||
               live[i].sItemInfo.ItemHeader.Head!=
                   expected[i].sItemInfo.ItemHeader.Head ||
               live[i].sItemInfo.ItemHeader.dwChkSum!=
                   expected[i].sItemInfo.ItemHeader.dwChkSum)
                return TradeLivePayloadPreserve353::IdentityChanged;
        }
    }
    for(int bag=0;bag<2;++bag) {
        const auto& expected=bag==0?expectedA:expectedB;
        const auto& used=bag==0?usedA:usedB;
        LegacyItem* live=bag==0?liveA:liveB;
        for(std::size_t i=0;i<capacity;++i) {
            if(!used[i]) continue;
            live[i].x=expected[i].x;
            live[i].y=expected[i].y;
            // Preserve the full original payload, not merely three identity
            // fields. This keeps the first accepted save exactly comparable
            // to the authenticated journal after-image.
            std::memcpy(&live[i].sItemInfo,&expected[i].sItemInfo,
                        sizeof(expected[i].sItemInfo));
        }
    }
    return TradeLivePayloadPreserve353::Preserved;
}

}}}
