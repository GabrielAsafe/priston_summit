#pragma once
// SUMMIT351G: narrow, one-save-only fallback for the first post-commit inventory.
// The original 350Q byte-exact barrier remains authoritative and is tried first.
// Only differences in the internal ItemCount of equipped records are tolerated;
// item identity, complete sItemInfo bytes, position, coordinates, bag membership,
// bag slots, and the number of items are immutable. No DAT writes or unlocks.
#include "gameplay/companions/v2/CompanionTradeLiveHandoff350Q.h"
#include "gameplay/companions/v2/CompanionTradeBoundedItemCodec.h"
#include "gameplay/companions/v2/CompanionTradeInventorySnapshot.h"
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <map>
#include <mutex>
#include <set>
#include <utility>
#include <vector>

namespace gameplay { namespace companions { namespace v2 {

class TradeLivePostSaveEquivalence351G final {
    typedef std::vector<std::uint8_t> Row;
    typedef std::vector<Row> Canonical;
public:
    // ONLY call after authenticated ACK1, after the worker verified the
    // journaled owner-after image, and while the online save lease is held.
    template<class LegacyRecordItem>
    bool Arm(SaveSessionKey key, const std::vector<std::uint8_t>& after,
             const BufferedTradeEnvelopeLayout& layout,
             std::size_t maximumItems, std::size_t slotsPerBag) {
        if(!key.Valid()) return false;
        Canonical rows;
        if(!Build<LegacyRecordItem>(after.data(),after.size(),layout,
                                    maximumItems,slotsPerBag,rows)) return false;
        std::lock_guard<std::mutex> lock(mutex_);
        if(expected_.size()>=64u) return false; // fail closed, bounded live proof cache
        return expected_.emplace(key,std::move(rows)).second;
    }

    template<class LegacyRecordItem>
    bool Equivalent(SaveSessionKey key, const std::uint8_t* received,
                    std::size_t bytes, const BufferedTradeEnvelopeLayout& layout,
                    std::size_t maximumItems, std::size_t slotsPerBag) const {
        // Do not hold the gate mutex while parsing client-controlled bytes.
        Canonical expected;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            const auto it=expected_.find(key);
            if(it==expected_.end()) return false;
            expected=it->second;
        }
        Canonical current;
        return Build<LegacyRecordItem>(received,bytes,layout,maximumItems,
                                       slotsPerBag,current) && current==expected;
    }

    void MarkAccepted(SaveSessionKey key) { Forget(key); }
    void Forget(SaveSessionKey key) {
        std::lock_guard<std::mutex> lock(mutex_);
        expected_.erase(key);
    }
    bool Pending(SaveSessionKey key) const {
        std::lock_guard<std::mutex> lock(mutex_);
        return expected_.count(key)!=0;
    }
private:
    static bool ReadI32(const std::uint8_t* p,std::size_t n,std::size_t at,
                        std::int32_t& out) {
        if(!p || at>n || n-at<sizeof(out)) return false;
        std::memcpy(&out,p+at,sizeof(out));return true;
    }
    static void Push32(Row& out,std::uint32_t n) {
        for(int i=0;i<4;++i) out.push_back(static_cast<std::uint8_t>(n>>(8*i)));
    }
    template<class LegacyRecordItem>
    static bool Build(const std::uint8_t* bytes,std::size_t available,
                      const BufferedTradeEnvelopeLayout& layout,
                      std::size_t maximumItems,std::size_t slotsPerBag,
                      Canonical& out) {
        if(!bytes || !available || available>layout.maxBytes ||
           !maximumItems || maximumItems>4096 ||
           !slotsPerBag || slotsPerBag>4096 ||
           layout.dataOffset>available) return false;
        std::int32_t recordBytes=0,total=0,primary=0,dataBytes=0;
        if(!ReadI32(bytes,available,layout.sizeOffset,recordBytes) ||
           !ReadI32(bytes,available,layout.itemCountOffset,total) ||
           !ReadI32(bytes,available,layout.primaryOffset,primary) ||
           !ReadI32(bytes,available,layout.dataSizeOffset,dataBytes) ||
           recordBytes<0 || total<0 || primary<0 || primary>total ||
           static_cast<std::size_t>(total)>maximumItems ||
           static_cast<std::size_t>(total)>slotsPerBag*2u ||
           dataBytes<0 || static_cast<std::size_t>(recordBytes)>available ||
           static_cast<std::size_t>(recordBytes)<layout.dataOffset ||
           static_cast<std::size_t>(dataBytes)>
               static_cast<std::size_t>(recordBytes)-layout.dataOffset)
            return false;
        // The fixed-size count & bag boundary are also compared, not just rows.
        Canonical rows;
        Row header;
        const char tag[]="SUMMIT-TRADE-POSTSAVE-351G-V1";
        header.insert(header.end(),tag,tag+sizeof(tag));
        Push32(header,static_cast<std::uint32_t>(total));
        Push32(header,static_cast<std::uint32_t>(primary));
        rows.push_back(std::move(header));
        std::set<std::pair<int,std::uint32_t> > bagSlots;
        std::set<std::vector<std::uint8_t> > itemIdentities;
        std::size_t offset=layout.dataOffset;
        const std::size_t end=layout.dataOffset+static_cast<std::size_t>(dataBytes);
        for(int i=0;i<total;++i) {
            std::int32_t frame=0;
            if(!ReadI32(bytes,end,offset,frame) || frame<=4 ||
               static_cast<std::size_t>(frame)>end-offset) return false;
            LegacyRecordItem item{};
            if(CompanionTradeBoundedItemCodec::Decode(
                   bytes+offset,static_cast<std::size_t>(frame),
                   reinterpret_cast<std::uint8_t*>(&item),sizeof(item))!=
               TradeItemDecodeStatus::Ok) return false;
            offset+=static_cast<std::size_t>(frame);
            // SUMMIT353: ItemCount is a BAG-array slot only for position=0.
            // Equipped records can legitimately hold -1 or any non-bag
            // legacy index. Their count is deliberately excluded from the
            // canonical row, so require slot bounds ONLY for bag records.
            // Passive potion / old anonymous records may be present, but
            // are immutable: compare their complete sItemInfo, bag and slot.
            if(item.ItemPosition<0 || item.ItemPosition>16 ||
               !item.sItemInfo.CODE ||
               (item.ItemPosition==0 &&
                (item.ItemCount<0 ||
                 static_cast<std::size_t>(item.ItemCount)>=slotsPerBag)))
                return false;
            const int bag=(i<primary)?0:1;
            if(item.ItemPosition==0 &&
               !bagSlots.emplace(bag,static_cast<std::uint32_t>(item.ItemCount)).second)
                return false;
            Row id;
            Push32(id,static_cast<std::uint32_t>(item.sItemInfo.CODE));
            Push32(id,static_cast<std::uint32_t>(item.sItemInfo.ItemHeader.Head));
            Push32(id,static_cast<std::uint32_t>(item.sItemInfo.ItemHeader.dwChkSum));
            // Unique *complete transferable* instances must stay unique.
            // Legacy anonymous records and potion stacks are intentionally
            // allowed in different slots; their full bytes are still matched.
            if(HasCompleteTradeIdentity350O(
                static_cast<std::uint32_t>(item.sItemInfo.CODE),
                static_cast<std::uint32_t>(item.sItemInfo.ItemHeader.Head),
                static_cast<std::uint32_t>(item.sItemInfo.ItemHeader.dwChkSum)) &&
               !itemIdentities.insert(id).second) return false;
            Row row;
            row.push_back(static_cast<std::uint8_t>(bag));
            Push32(row,static_cast<std::uint32_t>(item.ItemPosition));
            Push32(row,static_cast<std::uint32_t>(item.x));
            Push32(row,static_cast<std::uint32_t>(item.y));
            // Only EQUIPPED ItemCount is normalized. For bag items, the
            // actual original slot must match the committed after-image.
            Push32(row,item.ItemPosition==0 ?
                static_cast<std::uint32_t>(item.ItemCount) : 0xFFFFFFFFu);
            const auto* itemBytes=reinterpret_cast<const std::uint8_t*>(&item.sItemInfo);
            row.insert(row.end(),itemBytes,itemBytes+sizeof(item.sItemInfo));
            rows.push_back(std::move(row));
        }
        if(offset!=end) return false;
        std::sort(rows.begin()+1,rows.end());
        out=std::move(rows);
        return true;
    }
    mutable std::mutex mutex_;
    std::map<SaveSessionKey,Canonical> expected_;
};

inline TradeLivePostSaveEquivalence351G& ServerTradePostSave351G() {
    static TradeLivePostSaveEquivalence351G gate;
    return gate;
}
}}}
