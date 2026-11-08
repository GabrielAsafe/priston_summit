#pragma once
// SUMMIT356: atomic, deterministic packing of multiple incoming trade items.
// Packing by the UI's click order can fragment an otherwise sufficient bag.
// This planner tries larger footprints first, then maps the resulting slots
// back to the original request order. It never changes committed records,
// accepts unknown geometry, bypasses capacity checks or writes DATs.
#include "gameplay/inventory/v2/CompanionBagSpacePolicy.h"
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace gameplay { namespace companions { namespace v2 {
struct TradeIncoming356 {
    bool fromOwner;
    int width, height;
};
struct TradePlaced356 {
    bool secondary = false;
    int x = 0, y = 0;
    std::uint32_t slot = 0;
};

// SUMMIT357: Exact bounded recovery after the safe larger-first heuristic.
// Both target inventories are independent. Search each target separately,
// maintaining original selection indices. This does not mutate the input,
// authenticated snapshots, or caller output until the ENTIRE plan succeeds.
namespace placement357 {
using gameplay::inventory::BagRect;
using gameplay::inventory::CompanionBagSpacePolicy;
struct Pending { std::size_t index; int width, height; };
inline bool Fits(const std::vector<BagRect>& occupied, const BagRect& candidate) {
    if(!CompanionBagSpacePolicy::InBounds(candidate)) return false;
    for(const auto& r:occupied)
        if(CompanionBagSpacePolicy::Intersects(r,candidate)) return false;
    return true;
}
inline bool Search(std::vector<BagRect> (&occupied)[2],
                   std::vector<bool> (&used)[2],
                   const std::vector<Pending>& pending, std::size_t depth,
                   std::vector<TradePlaced356>& placed,
                   std::size_t& budget) {
    if(depth==pending.size()) return true;
    if(budget==0) return false;
    const Pending& p=pending[depth];
    for(int bag=0;bag<2;++bag) {
        std::size_t slot=0;
        while(slot<used[bag].size() && used[bag][slot]) ++slot;
        if(slot==used[bag].size()) continue;
        for(int row=0;row<CompanionBagSpacePolicy::Rows;++row) {
            for(int col=0;col<CompanionBagSpacePolicy::Columns;++col) {
                if(!budget) return false;
                --budget;
                const BagRect rect(CompanionBagSpacePolicy::OriginX+
                                       col*CompanionBagSpacePolicy::CellSize,
                                   CompanionBagSpacePolicy::OriginY+
                                       row*CompanionBagSpacePolicy::CellSize,
                                   p.width,p.height);
                if(!Fits(occupied[bag],rect)) continue;
                used[bag][slot]=true;
                occupied[bag].push_back(rect);
                placed[p.index]={bag==1,rect.x,rect.y,
                                 static_cast<std::uint32_t>(slot)};
                if(Search(occupied,used,pending,depth+1,placed,budget)) return true;
                occupied[bag].pop_back();
                used[bag][slot]=false;
            }
        }
    }
    return false;
}
inline bool Side(const std::vector<BagRect> (&current)[2],
                 const std::vector<bool> (&currentSlots)[2],
                 const std::vector<Pending>& pending,
                 std::vector<TradePlaced356>& placed) {
    if(pending.empty()) return true;
    std::vector<BagRect> occupied[2]={current[0],current[1]};
    std::vector<bool> used[2]={currentSlots[0],currentSlots[1]};
    std::size_t freeSlots=0;
    for(int bag=0;bag<2;++bag) {
        if(CompanionBagSpacePolicy::FindFree(occupied[bag],1,1).status==
               gameplay::inventory::BagSpaceStatus::CorruptPlacement)
            return false;
        freeSlots+=static_cast<std::size_t>(
            std::count(used[bag].begin(),used[bag].end(),false));
    }
    if(freeSlots<pending.size()) return false;
    // The fast path is identical to 356. The exact fallback addresses
    // first-fit fragmentation; it is limited to 8 items/120000 attempts
    // to avoid monopolizing the game-server worker thread.
    bool greedy=true;
    for(const Pending& p:pending) {
        bool found=false;
        for(int bag=0;bag<2 && !found;++bag) {
            std::size_t slot=0;
            while(slot<used[bag].size() && used[bag][slot]) ++slot;
            if(slot==used[bag].size()) continue;
            const auto fit=CompanionBagSpacePolicy::FindFree(
                occupied[bag],p.width,p.height);
            if(!fit.Available()) continue;
            used[bag][slot]=true;
            occupied[bag].emplace_back(fit.x,fit.y,p.width,p.height);
            placed[p.index]={bag==1,fit.x,fit.y,
                             static_cast<std::uint32_t>(slot)};
            found=true;
        }
        if(!found) {greedy=false;break;}
    }
    if(greedy) return true;
    if(pending.size()>8u) return false;
    occupied[0]=current[0]; occupied[1]=current[1];
    used[0]=currentSlots[0]; used[1]=currentSlots[1];
    std::size_t budget=120000u;
    return Search(occupied,used,pending,0,placed,budget);
}
} // namespace placement357

inline bool PlanTradeIncoming356(
    const std::vector<gameplay::inventory::BagRect> (&ownerOccupied)[2],
    const std::vector<gameplay::inventory::BagRect> (&companionOccupied)[2],
    const std::vector<bool> (&ownerSlots)[2],
    const std::vector<bool> (&companionSlots)[2],
    std::size_t slotsPerBag,
    const std::vector<TradeIncoming356>& incoming,
    std::vector<TradePlaced356>& output)
{
    namespace inv = gameplay::inventory;
    if(!slotsPerBag || slotsPerBag>4096 || incoming.empty() ||
       incoming.size()>slotsPerBag*4u) return false;
    for(int bag=0;bag<2;++bag)
        if(ownerSlots[bag].size()!=slotsPerBag ||
           companionSlots[bag].size()!=slotsPerBag) return false;
    std::vector<placement357::Pending> toOwner, toCompanion;
    for(std::size_t i=0;i<incoming.size();++i) {
        const auto& p=incoming[i];
        if(p.width<=0 || p.height<=0 ||
           p.width>inv::CompanionBagSpacePolicy::Columns*inv::CompanionBagSpacePolicy::CellSize ||
           p.height>inv::CompanionBagSpacePolicy::Rows*inv::CompanionBagSpacePolicy::CellSize)
            return false;
        (p.fromOwner?toCompanion:toOwner).push_back({i,p.width,p.height});
    }
    const auto order=[](const placement357::Pending& a,
                        const placement357::Pending& b) {
        const std::int64_t areaA=static_cast<std::int64_t>(a.width)*a.height;
        const std::int64_t areaB=static_cast<std::int64_t>(b.width)*b.height;
        if(areaA!=areaB) return areaA>areaB;
        return std::max(a.width,a.height)>std::max(b.width,b.height);
    };
    std::stable_sort(toOwner.begin(),toOwner.end(),order);
    std::stable_sort(toCompanion.begin(),toCompanion.end(),order);
    std::vector<TradePlaced356> planned(incoming.size());
    if(!placement357::Side(ownerOccupied,ownerSlots,toOwner,planned) ||
       !placement357::Side(companionOccupied,companionSlots,toCompanion,planned))
        return false;
    output=std::move(planned);
    return true;
}
}}} // gameplay::companions::v2
