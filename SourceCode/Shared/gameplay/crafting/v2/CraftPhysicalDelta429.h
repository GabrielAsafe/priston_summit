#pragma once
// SUMMIT 429A: pure post-image identity conservation proof.
// Host MUST supply entries decoded independently from FULL authenticated DAT
// before/after images, plus an output freshly minted by native CreateItem.
// This verifier never grants, mutates, locks or writes a DAT.
#include <cstdint>
#include <map>
#include <set>
#include <string>
#include <tuple>
#include <vector>

namespace summit_craft_429 {
struct Identity {
    std::uint32_t code=0,key=0,checksum=0;
    bool Valid() const {return code && key && checksum;}
    bool operator<(const Identity& x) const {
        return std::tie(code,key,checksum)<std::tie(x.code,x.key,x.checksum);
    }
};
struct Record {
    Identity id;
    // SHA256 of full native sITEMINFO content, independent of bag slot.
    // Must be calculated from a VERIFIED decoded full record, never chat.
    std::string payloadSha256;
    int bag=0;   // 1 or 2; equipped items are never ingredients
    int position=0; // 0 in bag; other positions are not craft ingredients
};
struct Requirement {std::uint32_t code=0;std::uint32_t quantity=0;};
struct ProofInput {
    bool recipeEnabled=false;
    bool outputNativeAndBothProfiles=false;
    bool layoutAndCapacityVerified=false;
    bool saveLeaseHeld=false;
    std::uint32_t outputCode=0;
    std::uint32_t outputQuantity=1;
    std::vector<Requirement> requirements;
    std::vector<Record> before,after;
    // Each minted native output must have a unique identity and full payload SHA.
    std::vector<Record> minted;
};
enum class ProofStatus {
    Valid, NotAuthorized, BadRecipe, BadSnapshot, MissingIngredient,
    WrongDebit, ForgedOutput, UnrelatedMutation
};
inline bool IsSha256(const std::string& s) {
    if(s.size()!=64u)return false;
    for(const char c:s) if(!((c>='0'&&c<='9')||(c>='a'&&c<='f')))return false;
    return true;
}
inline ProofStatus Prove(const ProofInput& v) {
    if(!v.saveLeaseHeld || !v.layoutAndCapacityVerified)return ProofStatus::NotAuthorized;
    if(!v.recipeEnabled || !v.outputNativeAndBothProfiles || !v.outputCode ||
       v.outputQuantity==0 || v.outputQuantity>16 || v.requirements.empty() ||
       v.requirements.size()>16 || v.minted.size()!=v.outputQuantity)
        return ProofStatus::BadRecipe;
    std::map<std::uint32_t,unsigned> needs;
    for(const auto& r:v.requirements)
        if(!r.code || !r.quantity || r.quantity>16 || !needs.emplace(r.code,r.quantity).second)
            return ProofStatus::BadRecipe;
    std::map<Identity,Record> oldItems,newItems,minted;
    // Legacy DATs can contain passive potion stacks with no instance key.
    // Keep them byte-identical and in the same bag/position. They can never
    // be selected as an ingredient, but must not block unrelated crafting.
    using Passive=std::tuple<std::uint32_t,std::string,int,int>;
    std::map<Passive,unsigned> oldPassive,newPassive;
    for(const auto& r:v.before) {
        if(!r.id.code || !IsSha256(r.payloadSha256) ||
            r.bag<1 || r.bag>2 || r.position<0 || r.position>16)
            return ProofStatus::BadSnapshot;
        if(r.id.Valid()) {
            if(!oldItems.emplace(r.id,r).second)return ProofStatus::BadSnapshot;
        } else ++oldPassive[Passive(r.id.code,r.payloadSha256,r.bag,r.position)];
    }
    for(const auto& r:v.after) {
        if(!r.id.code || !IsSha256(r.payloadSha256) ||
            r.bag<1 || r.bag>2 || r.position<0 || r.position>16)
            return ProofStatus::BadSnapshot;
        if(r.id.Valid()) {
            if(!newItems.emplace(r.id,r).second)return ProofStatus::BadSnapshot;
        } else ++newPassive[Passive(r.id.code,r.payloadSha256,r.bag,r.position)];
    }
    if(oldPassive!=newPassive)return ProofStatus::UnrelatedMutation;
    for(const auto& r:v.minted) {
        if(!r.id.Valid() || !IsSha256(r.payloadSha256) ||
           r.id.code!=v.outputCode || r.position!=0 || r.bag<1 || r.bag>2 ||
           oldItems.count(r.id) || !minted.emplace(r.id,r).second)
            return ProofStatus::ForgedOutput;
    }
    // Retained items must have unchanged FULL item data, even if they move to
    // another free cell. Authorization for any bag relocation is a separate
    // geometric layout proof controlled by the host.
    std::map<std::uint32_t,unsigned> removed;
    for(const auto& pair:oldItems) {
        const auto& old=pair.second;
        const auto at=newItems.find(pair.first);
        if(at==newItems.end()) {
            if(old.position!=0 || !needs.count(old.id.code))return ProofStatus::UnrelatedMutation;
            ++removed[old.id.code];
        } else if(at->second.payloadSha256!=old.payloadSha256 ||
                  at->second.position!=old.position) {
            return ProofStatus::UnrelatedMutation;
        }
    }
    for(const auto& item:newItems)
        if(!oldItems.count(item.first)) {
            auto it=minted.find(item.first);
            if(it==minted.end() || item.second.payloadSha256!=it->second.payloadSha256 ||
               item.second.bag!=it->second.bag || item.second.position!=0)
                return ProofStatus::ForgedOutput;
        }
    for(const auto& expected:minted) if(!newItems.count(expected.first))
        return ProofStatus::ForgedOutput;
    for(const auto& need:needs)
        if(removed[need.first]!=need.second) {
            unsigned available=0;
            for(const auto& old:oldItems)
                if(old.second.id.code==need.first && old.second.position==0) ++available;
            return available<need.second ? ProofStatus::MissingIngredient : ProofStatus::WrongDebit;
        }
    return ProofStatus::Valid;
}
}
