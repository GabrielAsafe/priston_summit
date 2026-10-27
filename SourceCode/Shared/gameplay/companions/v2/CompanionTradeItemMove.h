#pragma once
// SUMMIT: item movement plan for authoritative server integration.
// NO IO. NO network. NO DAT writes.  Requires exclusive external locks and
// server-origin decoded full records; no client-origin payload is authoritative.
#include "gameplay/companions/v2/CompanionTradeInventorySnapshot.h"
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <vector>
namespace gameplay { namespace companions { namespace v2 {

enum class TradeSide : std::uint8_t { Player=0, Companion=1 };
struct TradeMove {
    TradeSide from;
    std::uint32_t recordIndex, code, key, checksum;
    // Server-picked, legally empty position in the destination inventory.
    bool secondary;
    std::int32_t x, y;
};
enum class TradePlanResult {
    Ready, Invalid, UnsafeSnapshot, StaleSelection, EquippedItem,
    DuplicateItem, CapacityExceeded, PlacementInvalid, CodecRejected,
    BytesExceeded, ConservationFailed
};
struct TradePlan {
    CompanionTradeInventorySnapshot playerAfter, companionAfter;
};
struct TradeLimits {
    std::size_t maxItems, maxEncodedBytes, maxMoves;
    TradeLimits(std::size_t i, std::size_t b, std::size_t m)
       : maxItems(i), maxEncodedBytes(b), maxMoves(m) {}
};

struct TradeItemIdentity {
    std::uint32_t code, key, checksum;
    bool operator<(const TradeItemIdentity& rhs) const {
        if (code != rhs.code) return code < rhs.code;
        if (key != rhs.key) return key < rhs.key;
        return checksum < rhs.checksum;
    }
    bool operator==(const TradeItemIdentity& rhs) const {
        return code==rhs.code && key==rhs.key && checksum==rhs.checksum;
    }
};
inline TradeItemIdentity TradeIdentity(const CompanionTradeItemRecord& r) {
    return {r.itemCode,r.itemKey,r.itemChecksum};
}
inline bool TradeRecordEnvelopeValid(const CompanionTradeItemRecord& r) {
    if (!r.itemCode ||
        r.encodedRecord.size()<=4 ||
        r.encodedRecord.size()>static_cast<std::size_t>(INT32_MAX)) return false;
    std::int32_t n=0;
    std::memcpy(&n,r.encodedRecord.data(),4);
    return n>4 && static_cast<std::size_t>(n)==r.encodedRecord.size();
}
inline bool TradeSnapshotStrict(const CompanionTradeInventorySnapshot& s,
                                const TradeLimits& limits) {
    if (!s.TradeEligible() || s.records.size()>limits.maxItems ||
        s.primaryCount+s.secondaryCount!=s.records.size()) return false;
    std::size_t bytes=0;
    for (std::size_t i=0; i<s.records.size(); ++i) {
        const auto& r=s.records[i];
        if (!TradeRecordEnvelopeValid(r) || r.recordIndex!=i ||
            r.inSecondaryBag!=(i>=s.primaryCount) ||
            r.position<0 || r.position>16 ||
            r.encodedRecord.size()>limits.maxEncodedBytes-bytes) return false;
        bytes+=r.encodedRecord.size();
        if (HasCompleteTradeIdentity350O(r.itemCode,r.itemKey,r.itemChecksum))
            for(std::size_t j=0;j<i;++j)
                if(TradeIdentity(r)==TradeIdentity(s.records[j])) return false;
    }
    return true;
}
inline std::vector<TradeItemIdentity> TradeIdentities(
    const CompanionTradeInventorySnapshot& a,
    const CompanionTradeInventorySnapshot& b) {
    std::vector<TradeItemIdentity> result;
    result.reserve(a.records.size()+b.records.size());
    // Stack records AND incomplete-ID legacy records are passive and cannot
    // be counted as uniquely identified transferable item instances.
    for(const auto& r:a.records)
        if(HasCompleteTradeIdentity350O(r.itemCode,r.itemKey,r.itemChecksum))
            result.push_back(TradeIdentity(r));
    for(const auto& r:b.records)
        if(HasCompleteTradeIdentity350O(r.itemCode,r.itemKey,r.itemChecksum))
            result.push_back(TradeIdentity(r));
    std::sort(result.begin(),result.end()); return result;
}
// A transfer of an identified item MUST leave every passive record
// (potion stack OR incomplete identity) on its original participant and bag,
// with identical encoded bytes, in the same relative order.
// Record indices may change when a different item is removed from the array.
inline bool TradePassiveStacksUnchanged350N(
    const CompanionTradeInventorySnapshot& before,
    const CompanionTradeInventorySnapshot& after) {
    std::size_t next=0;
    for(const auto& old:before.records) {
        if(!IsPassiveTradeRecord350O(old.itemCode,old.itemKey,old.itemChecksum)) continue;
        while(next<after.records.size() &&
              !IsPassiveTradeRecord350O(after.records[next].itemCode,
                                        after.records[next].itemKey,
                                        after.records[next].itemChecksum)) ++next;
        if(next==after.records.size()) return false;
        const auto& current=after.records[next++];
        if(old.itemCode!=current.itemCode || old.itemKey!=current.itemKey ||
           old.itemChecksum!=current.itemChecksum ||
           old.position!=current.position || old.x!=current.x ||
           old.y!=current.y || old.inSecondaryBag!=current.inSecondaryBag ||
           old.encodedRecord!=current.encodedRecord) return false;
    }
    while(next<after.records.size()) {
        const auto& item=after.records[next++];
        if(IsPassiveTradeRecord350O(item.itemCode,item.itemKey,item.itemChecksum)) return false;
    }
    return true;
}

inline bool TradeNoDuplicates(const std::vector<TradeItemIdentity>& ids) {
    return std::adjacent_find(ids.begin(),ids.end())==ids.end();
}
inline void TradeRenumber(CompanionTradeInventorySnapshot& s) {
    s.primaryCount=0;
    // Stable partition preserving the order of the existing records.
    std::stable_partition(s.records.begin(),s.records.end(),
                          [](const CompanionTradeItemRecord& r){return !r.inSecondaryBag;});
    for(std::size_t i=0;i<s.records.size();++i) {
        if(!s.records[i].inSecondaryBag) ++s.primaryCount;
        s.records[i].recordIndex=static_cast<std::uint32_t>(i);
    }
    s.secondaryCount=static_cast<std::uint32_t>(s.records.size()-s.primaryCount);
}

// Relocate callback must have server-side authority: decode the ORIGINAL
// record, assign an unoccupied destination cell under complete grid policy,
// encode it, and return decoded identity/position/coordinates. The planner
// verifies the resulting instance identity and encoded envelope. It does
// NOT interpret item shape/grid geometry.
template<class Relocate>
TradePlanResult PlanItemMoves(
    const CompanionTradeInventorySnapshot& player,
    const CompanionTradeInventorySnapshot& companion,
    const std::vector<TradeMove>& moves, const TradeLimits& limits,
    Relocate relocate, TradePlan& output)
{
    if(!limits.maxItems || !limits.maxEncodedBytes || !limits.maxMoves ||
       moves.empty() || moves.size()>limits.maxMoves ||
       !TradeSnapshotStrict(player,limits) || !TradeSnapshotStrict(companion,limits))
        return TradePlanResult::UnsafeSnapshot;
    const auto before=TradeIdentities(player,companion);
    if(!TradeNoDuplicates(before)) return TradePlanResult::DuplicateItem;

    std::vector<bool> takePlayer(player.records.size(),false);
    std::vector<bool> takeCompanion(companion.records.size(),false);
    for(const auto& m:moves) {
        if(m.from!=TradeSide::Player && m.from!=TradeSide::Companion)
            return TradePlanResult::Invalid;
        const bool fromPlayer=m.from==TradeSide::Player;
        const auto& source=fromPlayer?player:companion;
        auto& take=fromPlayer?takePlayer:takeCompanion;
        if(m.recordIndex>=source.records.size()) return TradePlanResult::StaleSelection;
        if(take[m.recordIndex]) return TradePlanResult::DuplicateItem;
        const auto& r=source.records[m.recordIndex];
        if(r.itemCode!=m.code || r.itemKey!=m.key || r.itemChecksum!=m.checksum)
            return TradePlanResult::StaleSelection;
        if(r.position!=0) return TradePlanResult::EquippedItem;
        // An anonymous legacy item or quantity-based potion has no unique
        // instance identity. It may exist passively but cannot be moved.
        if(!HasCompleteTradeIdentity350O(r.itemCode,r.itemKey,r.itemChecksum))
            return TradePlanResult::Invalid;
        if(m.x<0 || m.y<0) return TradePlanResult::PlacementInvalid;
        take[m.recordIndex]=true;
    }

    TradePlan next;
    std::vector<CompanionTradeItemRecord> toPlayer,toCompanion;
    for(std::size_t i=0;i<player.records.size();++i)
        if(!takePlayer[i]) next.playerAfter.records.push_back(player.records[i]);
    for(std::size_t i=0;i<companion.records.size();++i)
        if(!takeCompanion[i]) next.companionAfter.records.push_back(companion.records[i]);
    for(const auto& m:moves) {
        const auto& r=(m.from==TradeSide::Player?player:companion).records[m.recordIndex];
        CompanionTradeItemRecord relocated;
        // Callback failure leaves caller's output untouched.
        if(!relocate(r,m.secondary,m.x,m.y,relocated)) return TradePlanResult::CodecRejected;
        if(!(TradeIdentity(relocated)==TradeIdentity(r)) ||
           relocated.position!=0 || relocated.x!=m.x || relocated.y!=m.y ||
           relocated.inSecondaryBag!=m.secondary ||
           !TradeRecordEnvelopeValid(relocated)) return TradePlanResult::PlacementInvalid;
        if(m.from==TradeSide::Player) toCompanion.push_back(std::move(relocated));
        else toPlayer.push_back(std::move(relocated));
    }
    next.playerAfter.records.insert(next.playerAfter.records.end(),toPlayer.begin(),toPlayer.end());
    next.companionAfter.records.insert(next.companionAfter.records.end(),toCompanion.begin(),toCompanion.end());
    TradeRenumber(next.playerAfter);
    TradeRenumber(next.companionAfter);
    // A basic necessary placement check; the production legacy grid/codec
    // callback MUST additionally check dimensions and all occupied cells.
    const auto uniqueAnchors=[](const CompanionTradeInventorySnapshot& inv) {
        for(std::size_t i=0;i<inv.records.size();++i) {
            const auto& a=inv.records[i];
            if(a.position!=0) continue;
            for(std::size_t j=0;j<i;++j) {
                const auto& b=inv.records[j];
                if(b.position==0 && a.inSecondaryBag==b.inSecondaryBag &&
                   a.x==b.x && a.y==b.y) return false;
            }
        }
        return true;
    };
    if(!uniqueAnchors(next.playerAfter) || !uniqueAnchors(next.companionAfter))
        return TradePlanResult::PlacementInvalid;
    if(next.playerAfter.records.size()>limits.maxItems ||
       next.companionAfter.records.size()>limits.maxItems)
        return TradePlanResult::CapacityExceeded;
    auto total=[](const CompanionTradeInventorySnapshot& s){
        std::size_t n=0;for(const auto& r:s.records)n+=r.encodedRecord.size();return n;
    };
    if(total(next.playerAfter)>limits.maxEncodedBytes ||
       total(next.companionAfter)>limits.maxEncodedBytes)
        return TradePlanResult::BytesExceeded;
    const auto after=TradeIdentities(next.playerAfter,next.companionAfter);
    if(!TradeNoDuplicates(after) || before!=after ||
       !TradePassiveStacksUnchanged350N(player,next.playerAfter) ||
       !TradePassiveStacksUnchanged350N(companion,next.companionAfter) ||
       !TradeSnapshotStrict(next.playerAfter,limits) ||
       !TradeSnapshotStrict(next.companionAfter,limits))
        return TradePlanResult::ConservationFailed;
    output=std::move(next);
    return TradePlanResult::Ready;
}
}}}
