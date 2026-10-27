#include "gameplay/companions/v2/CompanionTradeInventorySnapshot.h"
#include "gameplay/companions/v2/CompanionTradeItemMove.h"
#include "gameplay/companions/v2/CompanionTradeRevision.h"
#include "gameplay/companions/v2/CompanionTradeLegacyInventoryStage.h"
#include "gameplay/companions/v2/CompanionTradeRecordImageBuilder.h"
#include "gameplay/companions/v2/CompanionTradeConcreteTransfer.h"
#include <cassert>
#include <cstdint>
#include <cstring>
#include <vector>
using namespace gameplay::companions::v2;
static std::vector<std::uint8_t> Enc(std::uint8_t marker) {
    return {8,0,0,0,marker,2,3,4};
}
static CompanionTradeDecodedItem Decoded(std::uint8_t token) {
    CompanionTradeDecodedItem out;
    out.position=0;
    out.x=token*2;
    out.y=0;
    if(token==1 || token==2){out.code=0x04010005;out.key=0;out.checksum=0;}
    else if(token==3){out.code=0x01020003;out.key=7;out.checksum=9;}
    else {out.code=0x02030003;out.key=0;out.checksum=0;}
    return out;
}
static CompanionTradeInventorySnapshot Read(const std::vector<std::uint8_t>& bytes,int count) {
    CompanionTradeInventorySnapshot s;
    const auto dec=[](const std::uint8_t* p,std::size_t n,CompanionTradeDecodedItem& i) {
        if(n!=8)return false;
        i=Decoded(p[4]); return true;
    };
    assert(ReadCompanionTradeInventory(bytes.data(),bytes.size(),count,count,10,dec,s)==
        CompanionTradeInventoryReadCode::Ok);
    return s;
}
static std::vector<std::uint8_t> Compose(std::initializer_list<std::uint8_t> ids) {
    std::vector<std::uint8_t> result;
    for(auto i:ids) {auto e=Enc(i); result.insert(result.end(),e.begin(),e.end());}
    return result;
}
static std::vector<std::uint8_t> Image(const std::vector<std::uint8_t>& items,int count) {
    std::vector<std::uint8_t> b(20+items.size(),0);
    std::int32_t size=static_cast<std::int32_t>(b.size()), bytes=static_cast<std::int32_t>(items.size());
    std::memcpy(b.data()+0,&size,4);std::memcpy(b.data()+4,&count,4);
    std::memcpy(b.data()+8,&count,4);std::memcpy(b.data()+12,&bytes,4);
    if(!items.empty()) std::memcpy(b.data()+20,items.data(),items.size());return b;
}
int main() {
    assert(IsLegacyPotionStack350N(0x04010005));
    assert(IsLegacyPotionStack350N(0x04020001));
    assert(!IsLegacyPotionStack350N(0x01020003));
    auto online=Read(Compose({1,2,3}),3);
    assert(online.TradeEligible());
    assert(online.records.size()==3);
    assert(online.records[0].itemKey==0 && online.records[1].itemKey==0);
    assert(CompanionTradePreviewRevision(online)!=0);
    TradeLimits limits(10,200,2);
    assert(TradeSnapshotStrict(online,limits));
    CompanionTradeInventorySnapshot offline; // no items in Companion
    assert(offline.TradeEligible());
    assert(TradeSnapshotStrict(offline,limits));
    TradePlan planned;
    auto relocate=[](const CompanionTradeItemRecord& r,bool second,int x,int y,
                     CompanionTradeItemRecord& target){
        target=r;target.inSecondaryBag=second;target.x=x;target.y=y;return true;
    };
    TradeMove sword{TradeSide::Player,2,online.records[2].itemCode,7,9,false,10,0};
    assert(PlanItemMoves(online,offline,{sword},limits,relocate,planned)==TradePlanResult::Ready);
    assert(planned.playerAfter.records.size()==2 && planned.companionAfter.records.size()==1);
    assert(TradePassiveStacksUnchanged350N(online,planned.playerAfter));
    assert(TradePassiveStacksUnchanged350N(offline,planned.companionAfter));
    auto corrupt=planned;
    corrupt.playerAfter.records[0].encodedRecord[5]^=1;
    assert(!TradePassiveStacksUnchanged350N(online,corrupt.playerAfter));
    TradeMove potion{TradeSide::Player,0,online.records[0].itemCode,0,0,false,5,0};
    assert(PlanItemMoves(online,offline,{potion},limits,relocate,planned)!=TradePlanResult::Ready);
    auto cursor=online;cursor.records[0].position=-1;cursor.hasCursorItem=true;
    assert(!cursor.TradeEligible());
    auto malformed=Read(Compose({4}),1);
    assert(malformed.hasMissingInstanceIdentity && malformed.TradeEligible());
    assert(CompanionTradePreviewRevision(malformed)!=0);
    TradeMove anonymous{TradeSide::Player,0,malformed.records[0].itemCode,0,0,false,10,0};
    assert(PlanItemMoves(malformed,offline,{anonymous},limits,relocate,planned)!=TradePlanResult::Ready);
    auto dup=Read(Compose({3,3}),2);
    assert(dup.hasDuplicateInstanceIdentity && !dup.TradeEligible());
    struct Item{std::uint32_t code=0;};
    TradeClientStagedBags<Item> stage;
    auto dec=[](const std::uint8_t* data,std::size_t n,Item& item,std::size_t& slot,
                CompanionTradeDecodedItem& out) {
        if(n!=8)return false;
        out=Decoded(data[4]);slot=data[4]-1;item.code=out.code;return true;
    };
    assert(CompanionTradeLegacyInventoryStage::Prepare(online,10,dec,stage)==
        TradeClientStageStatus::Ready);
    auto owner=Image(Compose({1,2,3}),3);
    auto companion=Image({},0);
    // Selection failures that previously all appeared as transfer=2.
    struct FakeFootprints final : gameplay::inventory::IItemFootprintCatalog {
        bool TryGetFootprint(gameplay::ItemId id,gameplay::inventory::ItemFootprint& out) const override {
            out.item=id;out.widthPixels=1;out.heightPixels=1;return id.Valid();
        }
    } fake;
    CompanionTradeOffer offer;
    offer.tradeNonce=1;offer.authenticatedOwner=2;offer.companionCharacter=3;
    offer.offerDigest=4;offer.offerRevision=5;
    const auto allow=[](const CompanionTradeItemRecord&,TradeSide){return true;};
    const auto slot=[](const CompanionTradeItemRecord&,std::uint32_t& out){out=0;return true;};
    auto malformedSelection=online;
    CompanionTradePrepared prep;
    TradeRecordLayout fakeLayout{0,4,8,12,20,200,20};
    const auto relocateWithSlot=[&](const CompanionTradeItemRecord& r,bool secondary,
        int x,int y,std::uint32_t,CompanionTradeItemRecord& out) {
        return relocate(r,secondary,x,y,out);
    };
    auto badPick=[&](const TradeSelectedItem& choice,const CompanionTradeInventorySnapshot& src) {
        return BuildConcreteTradeTransfer(offer,Image(Compose({1,2,3}),3),Image({},0),
            src,offline,fakeLayout,fakeLayout,{choice},fake,limits,10,
            allow,slot,relocateWithSlot,prep);
    };
    auto potionPick=badPick({TradeSide::Player,0,online.records[0].itemCode,0,0},online);
    assert(potionPick.code==ConcreteTransferCode::InvalidSelection &&
           potionPick.selectionReject==TradeSelectionReject350N::UnsupportedPotion);
    auto wrongPick=badPick({TradeSide::Player,2,sword.code,7,12345},online);
    assert(wrongPick.selectionReject==TradeSelectionReject350N::WrongIdentity);
    malformedSelection.records[2].position=1;
    auto equippedPick=badPick({TradeSide::Player,2,sword.code,7,9},malformedSelection);
    assert(equippedPick.selectionReject==TradeSelectionReject350N::Equipped);
    malformedSelection=online;malformedSelection.records[2].x=10000;
    auto badCoords=badPick({TradeSide::Player,2,sword.code,7,9},malformedSelection);
    assert(badCoords.selectionReject==TradeSelectionReject350N::InvalidCoordinates);
    TradeRecordLayout layout{0,4,8,12,20,200,20};
    std::vector<std::uint8_t> afterOwner,afterComp;
    assert(BuildTradeRecordImage(owner,online,planned.playerAfter,layout,10,afterOwner)==
        TradeImageResult::Ready);
    assert(BuildTradeRecordImage(companion,offline,planned.companionAfter,layout,10,afterComp)==
        TradeImageResult::Ready);
    assert(std::memcmp(owner.data()+20,afterOwner.data()+20,16)==0);
    assert(afterComp.size()>=28);
    // Reverse move: one exact item comes back while both potion stacks remain.
    auto reverseOwner=planned.playerAfter,reverseComp=planned.companionAfter;
    TradeMove back{TradeSide::Companion,0,sword.code,7,9,false,15,0};
    TradePlan reversed;
    assert(PlanItemMoves(reverseOwner,reverseComp,{back},limits,relocate,reversed)==
        TradePlanResult::Ready);
    assert(reversed.playerAfter.records.size()==3 && reversed.companionAfter.records.empty());
    assert(TradePassiveStacksUnchanged350N(online,reversed.playerAfter));
    return 0;
}
