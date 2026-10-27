#pragma once
// Builds an entire post-trade character record IN MEMORY, preserving all
// non-inventory fields byte-for-byte. Caller validates the server-side source,
// must hold a cross-participant lock, and must persist through a durable
// compare-and-swap backend + journal. This never writes DATs or session data.
#include "gameplay/companions/v2/CompanionTradeInventorySnapshot.h"
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <vector>
#include <algorithm>
#include <limits>
namespace gameplay { namespace companions { namespace v2 {
struct TradeRecordLayout {
    std::size_t sizeOffset, countOffset, subStartOffset, dataSizeOffset, dataOffset;
    std::size_t dataCapacity, minimumWriteSize;
};
enum class TradeImageResult {
    Ready, BadLayout, BadSourceImage, SnapshotMismatch, TooManyItems, NotEnoughSpace
};
inline bool TradeImageGet32(const std::vector<std::uint8_t>& b, std::size_t at,
                            std::int32_t& out) {
    if(at>b.size() || b.size()-at<4) return false;
    std::memcpy(&out,b.data()+at,4);return true;
}
inline void TradeImagePut32(std::vector<std::uint8_t>& b,std::size_t at,
                            std::int32_t value) {
    std::memcpy(b.data()+at,&value,4);
}
inline TradeImageResult BuildTradeRecordImage(
    const std::vector<std::uint8_t>& fullSourceImage,
    const CompanionTradeInventorySnapshot& before,
    const CompanionTradeInventorySnapshot& after,
    const TradeRecordLayout& layout,
    std::size_t maximumItems,
    std::vector<std::uint8_t>& fullAfterImage)
{
    const auto& img=fullSourceImage;
    if(layout.dataOffset<4 || layout.dataOffset>img.size() ||
       layout.dataOffset>static_cast<std::size_t>(INT32_MAX) ||
       layout.dataCapacity>static_cast<std::size_t>(INT32_MAX)-layout.dataOffset ||
       layout.minimumWriteSize>layout.dataOffset+layout.dataCapacity)
        return TradeImageResult::BadLayout;
    std::int32_t declared=0, oldCount=0, oldSub=0, oldDataBytes=0;
    if(!TradeImageGet32(img,layout.sizeOffset,declared) ||
       !TradeImageGet32(img,layout.countOffset,oldCount) ||
       !TradeImageGet32(img,layout.subStartOffset,oldSub) ||
       !TradeImageGet32(img,layout.dataSizeOffset,oldDataBytes) ||
       oldDataBytes<0 || oldCount<0 || oldSub<0 || oldSub>oldCount ||
       static_cast<std::size_t>(oldDataBytes)>layout.dataCapacity ||
       static_cast<std::size_t>(oldDataBytes)>img.size()-layout.dataOffset ||
       declared<0 || static_cast<std::size_t>(declared)>img.size() ||
       static_cast<std::size_t>(declared)<layout.dataOffset+static_cast<std::size_t>(oldDataBytes))
        return TradeImageResult::BadSourceImage;
    if(before.records.size()!=static_cast<std::size_t>(oldCount) ||
       before.primaryCount!=static_cast<std::uint32_t>(oldSub))
        return TradeImageResult::SnapshotMismatch;
    std::size_t cursor=layout.dataOffset;
    for (const auto& r:before.records) {
        if(r.encodedRecord.size()>static_cast<std::size_t>(oldDataBytes) ||
           cursor>layout.dataOffset+static_cast<std::size_t>(oldDataBytes)-r.encodedRecord.size() ||
           std::memcmp(img.data()+cursor,r.encodedRecord.data(),r.encodedRecord.size())!=0)
            return TradeImageResult::SnapshotMismatch;
        cursor+=r.encodedRecord.size();
    }
    if(cursor!=layout.dataOffset+static_cast<std::size_t>(oldDataBytes))
        return TradeImageResult::SnapshotMismatch;
    if(after.records.size()>maximumItems || after.records.size()>static_cast<std::size_t>(INT32_MAX) ||
       after.primaryCount>after.records.size()) return TradeImageResult::TooManyItems;
    std::size_t total=0;
    for(const auto& r:after.records) {
        if(r.encodedRecord.empty() || r.encodedRecord.size()>layout.dataCapacity-total)
            return TradeImageResult::NotEnoughSpace;
        total+=r.encodedRecord.size();
    }
    if(layout.dataOffset+total>static_cast<std::size_t>(INT32_MAX))
        return TradeImageResult::NotEnoughSpace;
    // Legacy DATs are variable-sized. A valid incoming file may end well
    // before the declared in-memory Data[] capacity. Grow only when the
    // incoming transfer needs it, keeping all preceding fields unchanged.
    const std::size_t required=std::max(layout.minimumWriteSize,layout.dataOffset+total);
    if(required>layout.dataOffset+layout.dataCapacity)
        return TradeImageResult::NotEnoughSpace;
    std::vector<std::uint8_t> next=img;
    if(next.size()<required) next.resize(required,0);
    const std::size_t present=std::min(layout.dataCapacity,next.size()-layout.dataOffset);
    std::memset(next.data()+layout.dataOffset,0,present);
    cursor=layout.dataOffset;
    for(const auto& r:after.records) {
        std::memcpy(next.data()+cursor,r.encodedRecord.data(),r.encodedRecord.size());
        cursor+=r.encodedRecord.size();
    }
    TradeImagePut32(next,layout.dataSizeOffset,static_cast<std::int32_t>(total));
    TradeImagePut32(next,layout.countOffset,static_cast<std::int32_t>(after.records.size()));
    TradeImagePut32(next,layout.subStartOffset,static_cast<std::int32_t>(after.primaryCount));
    TradeImagePut32(next,layout.sizeOffset,static_cast<std::int32_t>(layout.dataOffset+total));
    fullAfterImage=std::move(next);
    return TradeImageResult::Ready;
}
}}}
