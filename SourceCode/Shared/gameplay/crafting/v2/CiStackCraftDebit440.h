#pragma once
// SUMMIT440A: construct a quantity-aware crafting debit proposal from *real*
// native sITEMINFO fields (no DAT disk mutation). The caller must authenticate
// session ownership, native records, recipe enablement and journal the WHOLE
// after-image together with the minted equipment. No half commits.
#include "gameplay/crafting/v2/CiStackDatSeal440.h"
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <set>
#include <tuple>
#include <vector>

namespace summit_ci440 {
struct Need {std::uint32_t code,quantity;Need(std::uint32_t c=0,std::uint32_t q=0):code(c),quantity(q){} };
template<class Info> struct Slot {
    unsigned bag=0,slot=0; // 1/2, 0..99
    bool isOwnedAndLegacyVerified=false;
    Info info{};
};
template<class Info> struct Change {
    unsigned bag=0,slot=0;
    std::uint32_t before=0,after=0;
    bool remove=false;
    Info updated{}; // valid only if remove=false
};
enum class DebitResult {Ready, Unauthorised, Invalid, Tampered, Missing};

template<class Info>
inline DebitResult PlanDebit(const std::vector<Slot<Info>>& bagItems,
                             const std::vector<Need>& requirements,
                             const Key& secret,bool authenticatedRecipeEnabled,
                             bool exclusiveLeaseHeld,std::vector<Change<Info>>& output) {
    output.clear(); // never expose a stale proposal after a failed validation
    if(!authenticatedRecipeEnabled||!exclusiveLeaseHeld)return DebitResult::Unauthorised;
    if(!ValidKey(secret)||requirements.empty()||requirements.size()>16||bagItems.size()>200)
        return DebitResult::Invalid;
    std::set<std::uint32_t> needCodes;
    for(const auto& need:requirements)
        if(!IsNativeCi(need.code)||need.quantity==0||need.quantity>10000||
           !needCodes.insert(need.code).second)return DebitResult::Invalid;
    std::set<std::pair<unsigned,unsigned>> places;
    std::set<std::tuple<std::uint32_t,std::uint32_t,std::uint32_t>> identities;
    for(const auto& entry:bagItems) {
        if((entry.bag!=1&&entry.bag!=2)||entry.slot>99||!entry.isOwnedAndLegacyVerified||
           !entry.info.ItemHeader.Head || !entry.info.ItemHeader.dwChkSum ||
           !places.insert({entry.bag,entry.slot}).second ||
           !identities.insert(std::make_tuple(static_cast<std::uint32_t>(entry.info.CODE),
             static_cast<std::uint32_t>(entry.info.ItemHeader.Head),
             static_cast<std::uint32_t>(entry.info.ItemHeader.dwChkSum))).second)
            return DebitResult::Invalid;
        if(IsNativeCi(static_cast<std::uint32_t>(entry.info.CODE))) {
            const Reading amount=Read(entry.info,secret);
            if(amount.status!=Status::LegacyOne && amount.status!=Status::VerifiedStack)
                return DebitResult::Tampered;
        }
    }
    // Deterministic order; prefer partial piles then full, A before B.
    std::vector<std::size_t> order;
    for(std::size_t i=0;i<bagItems.size();++i)order.push_back(i);
    std::sort(order.begin(),order.end(),[&](std::size_t a,std::size_t b){
        const Reading x=Read(bagItems[a].info,secret),y=Read(bagItems[b].info,secret);
        if(x.quantity!=y.quantity)return x.quantity<y.quantity;
        if(bagItems[a].bag!=bagItems[b].bag)return bagItems[a].bag<bagItems[b].bag;
        return bagItems[a].slot<bagItems[b].slot;
    });
    std::vector<Change<Info>> staged;
    for(const auto& need:requirements) {
        std::uint32_t left=need.quantity;
        for(auto index:order) {
            if(!left)break;
            const auto& entry=bagItems[index];
            if(entry.info.CODE!=need.code)continue;
            const Reading original=Read(entry.info,secret);
            const std::uint32_t take=std::min(left,original.quantity);
            Change<Info> change;
            change.bag=entry.bag;change.slot=entry.slot;
            change.before=original.quantity;change.after=original.quantity-take;
            change.remove=change.after==0;
            if(!change.remove) {
                change.updated=entry.info;
                if(Seal(change.updated,change.after,secret,true,true,exclusiveLeaseHeld)!=Status::Sealed)
                    return DebitResult::Tampered;
            }
            staged.push_back(change);
            left-=take;
        }
        if(left)return DebitResult::Missing;
    }
    output=std::move(staged); // caller may not apply until full DAT+mint journal commits
    return DebitResult::Ready;
}
} // namespace summit_ci440
