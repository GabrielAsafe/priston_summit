#pragma once
// SUMMIT430A: construct a complete AFTER character DAT in memory from trusted
// native records. Never reads/writes disk, grants items, or authorizes live craft.
// The host must authenticate and lock the current character session separately.
#include "gameplay/crafting/v2/CraftPhysicalDelta429.h"
#include "gameplay/companions/v2/CompanionTradeRecordImageBuilder.h"
#include "gameplay/companions/v2/CompanionTradeInventorySnapshot.h"
#include "gameplay/companions/v2/CompanionTradeSha256.h"
#include <cstddef>
#include <cstdint>
#include <map>
#include <set>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

namespace summit_craft_430 {
namespace trade = gameplay::companions::v2;
using Item = trade::CompanionTradeItemRecord;
using Inventory = trade::CompanionTradeInventorySnapshot;
using Layout = trade::TradeRecordLayout;

struct Ingredient {std::uint32_t code=0; unsigned count=0;};
struct ItemIdentity {
    std::uint32_t code=0,key=0,checksum=0;
    bool operator<(const ItemIdentity& o) const {
        return std::tie(code,key,checksum)<std::tie(o.code,o.key,o.checksum);
    }
};
struct Recipe {
    bool enabled=false;
    bool identitiesSynchronized=false;
    std::uint32_t outputCode=0;
    std::vector<Ingredient> ingredients;
};
struct Minted {
    Item record; // Full native encoded record freshly produced by server CreateItem.
};
struct ExternalProof {
    // Assertions from a separate, trusted game-session adapter. THIS HEADER
    // cannot verify ownership, prevent deferred writes or authenticate minting.
    bool exclusiveSaveLeaseHeld=false;
    bool persistedAndOnlineMatched=false;
    bool outputMintedNatively=false;
};
struct Candidate {
    Inventory before,after;
    std::vector<std::uint8_t> fullAfterImage;
    std::string beforeSha256,afterSha256;
};
enum class Status {
    Ready, MissingAuthority, UnsafeSource, InvalidRecipe, AmbiguousItem, MissingIngredient,
    InvalidMint, InvalidPlacement, ImageBuildFailed, ImageReparseFailed, DeltaMismatch
};
inline ItemIdentity Id(const Item& x) {return {x.itemCode,x.itemKey,x.itemChecksum};}
inline std::string HexSha(const std::vector<std::uint8_t>& bytes) {
    static const char* digits="0123456789abcdef";
    const auto digest=trade::TradeSha256::Hash(bytes.data(),bytes.size());
    std::string hex;hex.reserve(64);
    for(auto b:digest){hex.push_back(digits[b>>4]);hex.push_back(digits[b&15]);}
    return hex;
}
inline std::string PayloadSha(const Item& x){return HexSha(x.encodedRecord);}
inline summit_craft_429::Record ForProof(const Item& x) {
    summit_craft_429::Record r;
    r.id={x.itemCode,x.itemKey,x.itemChecksum};
    r.payloadSha256=PayloadSha(x);
    r.bag=x.inSecondaryBag?2:1;
    r.position=x.position;
    return r;
}
inline bool SameSnapshot(const Inventory& a,const Inventory& b){
    if(a.primaryCount!=b.primaryCount || a.secondaryCount!=b.secondaryCount ||
       a.records.size()!=b.records.size())return false;
    for(std::size_t i=0;i<a.records.size();++i){
        const auto& x=a.records[i];const auto& y=b.records[i];
        if(Id(x)<Id(y) || Id(y)<Id(x) || x.recordIndex!=y.recordIndex ||
           x.inSecondaryBag!=y.inSecondaryBag || x.x!=y.x || x.y!=y.y ||
           x.position!=y.position || x.encodedRecord!=y.encodedRecord)return false;
    }
    return true;
}
// Decode must use the actual bounded legacy item codec, not unchecked packet bytes.
// PlacementMustBeValid should use native item footprint and BOTH bag geometries;
// unknown sizes or overlaps must yield false. A caller-provided `true` is not
// sufficient proof of safe online inventory mutation.
template<class Decode,class PlacementMustBeValid>
Status BuildCandidate(const std::vector<std::uint8_t>& authenticCompleteDat,
    const Layout& layout,unsigned maximumRecords,const Recipe& recipe,
    const std::vector<ItemIdentity>& consumed,
    const std::vector<Minted>& freshlyMinted,
    const ExternalProof& assertions,
    Decode boundedDecode,PlacementMustBeValid placementMustBeValid,
    Candidate& result) {
    if(!assertions.exclusiveSaveLeaseHeld ||
       !assertions.persistedAndOnlineMatched ||
       !assertions.outputMintedNatively)return Status::MissingAuthority;
    if(authenticCompleteDat.size()>8u*1024u*1024u || maximumRecords==0 || maximumRecords>4096 ||
       !recipe.enabled || !recipe.identitiesSynchronized || recipe.outputCode==0 ||
       recipe.ingredients.empty() || recipe.ingredients.size()>16 ||
       consumed.empty() || consumed.size()>16 || freshlyMinted.size()!=1)
        return Status::InvalidRecipe;
    std::map<std::uint32_t,unsigned> counts;
    unsigned requested=0;
    for(const auto& need:recipe.ingredients){
        if(!need.code || need.count==0 || need.count>16 ||
           !counts.emplace(need.code,need.count).second)return Status::InvalidRecipe;
        requested+=need.count;
    }
    if(requested!=consumed.size())return Status::InvalidRecipe;
    std::map<ItemIdentity,unsigned> selected;
    std::map<std::uint32_t,unsigned> removed;
    for(const auto& id:consumed){
        if(!id.code || !id.key || !id.checksum || !selected.emplace(id,1).second ||
           counts.count(id.code)==0)return Status::AmbiguousItem;
        ++removed[id.code];
    }
    if(removed!=counts)return Status::InvalidRecipe;
    std::int32_t itemCount=0,primary=0,dataBytes=0;
    if(!trade::TradeImageGet32(authenticCompleteDat,layout.countOffset,itemCount) ||
       !trade::TradeImageGet32(authenticCompleteDat,layout.subStartOffset,primary) ||
       !trade::TradeImageGet32(authenticCompleteDat,layout.dataSizeOffset,dataBytes) ||
       itemCount<0 || primary<0 || primary>itemCount || dataBytes<0 ||
       layout.dataOffset>authenticCompleteDat.size() ||
       static_cast<std::size_t>(dataBytes)>authenticCompleteDat.size()-layout.dataOffset ||
       itemCount>static_cast<int>(maximumRecords)) return Status::UnsafeSource;
    Inventory before;
    if(trade::ReadCompanionTradeInventory(
          authenticCompleteDat.data()+layout.dataOffset,
          static_cast<std::size_t>(dataBytes),itemCount,primary,maximumRecords,
          boundedDecode,before)!=trade::CompanionTradeInventoryReadCode::Ok ||
       !before.TradeEligible()) return Status::UnsafeSource;
    std::set<ItemIdentity> all;
    std::map<ItemIdentity,std::size_t> locations;
    for(std::size_t i=0;i<before.records.size();++i){
        const auto& r=before.records[i];const auto id=Id(r);
        if(id.key && id.checksum){
            if(!all.insert(id).second)return Status::AmbiguousItem;
            locations[id]=i;
        }
    }
    std::set<std::size_t> dropped;
    for(const auto& pick:selected){
        auto p=locations.find(pick.first);
        if(p==locations.end())return Status::MissingIngredient;
        if(before.records[p->second].position!=0 ||
           trade::IsLegacyPotionStack350N(pick.first.code))return Status::AmbiguousItem;
        dropped.insert(p->second);
    }
    if(before.records.size()-dropped.size()+freshlyMinted.size()>maximumRecords)
        return Status::InvalidPlacement;
    const auto& minted=freshlyMinted.front().record;
    if(minted.itemCode!=recipe.outputCode || minted.itemKey==0 || minted.itemChecksum==0 ||
       minted.position!=0 || minted.encodedRecord.empty() || all.count(Id(minted)))
        return Status::InvalidMint;
    trade::CompanionTradeDecodedItem parsedMint;
    if(!boundedDecode(minted.encodedRecord.data(),minted.encodedRecord.size(),parsedMint) ||
       parsedMint.code!=minted.itemCode || parsedMint.key!=minted.itemKey ||
       parsedMint.checksum!=minted.itemChecksum || parsedMint.position!=0 ||
       parsedMint.x!=minted.x || parsedMint.y!=minted.y) return Status::InvalidMint;
    Inventory after;
    // Copy unchanged A, insert minted A if selected, then unchanged B + minted B.
    for(std::size_t i=0;i<before.records.size();++i){
        if(i==before.primaryCount && !minted.inSecondaryBag) {
            Item value=minted;value.inSecondaryBag=false;
            after.records.push_back(value);
        }
        if(!dropped.count(i))after.records.push_back(before.records[i]);
    }
    if(before.primaryCount==before.records.size() && !minted.inSecondaryBag)
        after.records.push_back(minted);
    if(minted.inSecondaryBag)after.records.push_back(minted);
    after.primaryCount=before.primaryCount;
    for(std::size_t i:dropped)if(i<before.primaryCount)--after.primaryCount;
    if(!minted.inSecondaryBag)++after.primaryCount;
    after.secondaryCount=static_cast<std::uint32_t>(after.records.size()-after.primaryCount);
    for(std::size_t i=0;i<after.records.size();++i){
        after.records[i].recordIndex=static_cast<std::uint32_t>(i);
        after.records[i].inSecondaryBag=i>=after.primaryCount;
    }
    if(!placementMustBeValid(after))return Status::InvalidPlacement;
    // Independent proof that precisely the requested instance identities were
    // debited, the native output is the sole new item, and all retained payloads
    // are byte-identical. This still DOES NOT authorize persistence.
    summit_craft_429::ProofInput proof;
    proof.recipeEnabled=recipe.enabled;
    proof.outputNativeAndBothProfiles=recipe.identitiesSynchronized;
    proof.layoutAndCapacityVerified=true; // established by image input/reparse gates
    proof.saveLeaseHeld=assertions.exclusiveSaveLeaseHeld; // ONLY host assertion, not lease acquisition
    proof.outputCode=recipe.outputCode;
    proof.outputQuantity=1;
    for(const auto& need:recipe.ingredients)proof.requirements.push_back({need.code,need.count});
    for(const auto& r:before.records)proof.before.push_back(ForProof(r));
    for(const auto& r:after.records)proof.after.push_back(ForProof(r));
    proof.minted.push_back(ForProof(minted));
    if(summit_craft_429::Prove(proof)!=summit_craft_429::ProofStatus::Valid)
        return Status::DeltaMismatch;
    std::vector<std::uint8_t> fullAfter;
    if(trade::BuildTradeRecordImage(authenticCompleteDat,before,after,layout,
         maximumRecords,fullAfter)!=trade::TradeImageResult::Ready)
        return Status::ImageBuildFailed;
    std::int32_t aCount=0,aSplit=0,aData=0;
    if(!trade::TradeImageGet32(fullAfter,layout.countOffset,aCount) ||
       !trade::TradeImageGet32(fullAfter,layout.subStartOffset,aSplit) ||
       !trade::TradeImageGet32(fullAfter,layout.dataSizeOffset,aData) ||
       aCount<0 || aSplit<0 || aData<0 || layout.dataOffset>fullAfter.size() ||
       static_cast<std::size_t>(aData)>fullAfter.size()-layout.dataOffset)
        return Status::ImageReparseFailed;
    Inventory independent;
    if(trade::ReadCompanionTradeInventory(fullAfter.data()+layout.dataOffset,
        static_cast<std::size_t>(aData),aCount,aSplit,maximumRecords,
        boundedDecode,independent)!=trade::CompanionTradeInventoryReadCode::Ok ||
       !SameSnapshot(after,independent))return Status::ImageReparseFailed;
    Candidate ready;
    ready.before=std::move(before);ready.after=std::move(after);
    ready.fullAfterImage=std::move(fullAfter);
    ready.beforeSha256=HexSha(authenticCompleteDat);
    ready.afterSha256=HexSha(ready.fullAfterImage);
    result=std::move(ready); // only publish after every check succeeds
    return Status::Ready;
}
} // namespace summit_craft_430
