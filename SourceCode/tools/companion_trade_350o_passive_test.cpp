#include "gameplay/companions/v2/CompanionTradeInventorySnapshot.h"
#include "gameplay/companions/v2/CompanionTradeItemMove.h"
#include "gameplay/companions/v2/CompanionTradeRevision.h"
#include "gameplay/companions/v2/CompanionTradeBufferedPayloadCandidate.h"
#include "gameplay/companions/v2/CompanionTradeRecordImageBuilder.h"
#include <cassert>
#include <cstring>
#include <vector>
using namespace gameplay::companions::v2;
static std::vector<std::uint8_t> Make(std::initializer_list<unsigned char> ids) {
    std::vector<std::uint8_t> v;
    for(auto token:ids) {
        const unsigned char rec[] = {8,0,0,0,token,3,4,5};
        v.insert(v.end(),rec,rec+sizeof(rec));
    }
    return v;
}
static CompanionTradeInventorySnapshot Parse(std::vector<std::uint8_t> raw,int count) {
    CompanionTradeInventorySnapshot out;
    const auto decode=[](const std::uint8_t* p,std::size_t n,CompanionTradeDecodedItem& item) {
        if(n!=8)return false;
        item.position=0;item.x=p[4]*3;item.y=0;
        item.code=p[4]==1?0x02000011u:0x01000022u;
        item.key=p[4]==1?0:222;item.checksum=p[4]==1?0:333;
        return true;
    };
    assert(ReadCompanionTradeInventory(raw.data(),raw.size(),count,count,10,decode,out)==
           CompanionTradeInventoryReadCode::Ok);
    return out;
}
static std::vector<std::uint8_t> Image(const CompanionTradeInventorySnapshot& snap) {
    std::vector<std::uint8_t> bytes(20,0);
    for(const auto& item:snap.records)
        bytes.insert(bytes.end(),item.encodedRecord.begin(),item.encodedRecord.end());
    const std::int32_t size=static_cast<std::int32_t>(bytes.size());
    const std::int32_t count=static_cast<std::int32_t>(snap.records.size());
    const std::int32_t data=size-20;
    std::memcpy(bytes.data(),&size,4);
    std::memcpy(bytes.data()+4,&count,4);
    std::memcpy(bytes.data()+8,&count,4);
    std::memcpy(bytes.data()+12,&data,4);
    return bytes;
}
int main() {
    const auto original=Parse(Make({1,2}),2);
    CompanionTradeInventorySnapshot companion;
    assert(original.hasMissingInstanceIdentity && original.TradeEligible());
    assert(CompanionTradePreviewRevision(original)!=0);
    assert(TradeSnapshotStrict(original,TradeLimits(10,1000,2)));
    assert(TradeSnapshotStrict(companion,TradeLimits(10,1000,2)));
    const auto relocate=[](const CompanionTradeItemRecord& source,bool secondary,int x,int y,
                           CompanionTradeItemRecord& out) {
        out=source;out.inSecondaryBag=secondary;out.x=x;out.y=y;return true;
    };
    TradePlan planned;
    const auto limits=TradeLimits(10,1000,2);
    const TradeMove move{TradeSide::Player,1,original.records[1].itemCode,222,333,false,12,0};
    assert(PlanItemMoves(original,companion,{move},limits,relocate,planned)==TradePlanResult::Ready);
    assert(planned.playerAfter.records.size()==1 && planned.companionAfter.records.size()==1);
    assert(TradePassiveStacksUnchanged350N(original,planned.playerAfter));
    assert(planned.playerAfter.records[0].encodedRecord==original.records[0].encodedRecord);
    TradeRecordLayout layout{0,4,8,12,20,1000,20};
    std::vector<std::uint8_t> ownerAfter,companionAfter;
    assert(BuildTradeRecordImage(Image(original),original,planned.playerAfter,layout,10,ownerAfter)==TradeImageResult::Ready);
    assert(BuildTradeRecordImage(Image(companion),companion,planned.companionAfter,layout,10,companionAfter)==TradeImageResult::Ready);
    assert(std::memcmp(ownerAfter.data()+20,original.records[0].encodedRecord.data(),8)==0);
    // The anonymous record itself cannot be selected or replayed.
    const TradeMove anonymous{TradeSide::Player,0,original.records[0].itemCode,0,0,false,0,0};
    assert(PlanItemMoves(original,companion,{anonymous},limits,relocate,planned)!=TradePlanResult::Ready);
    // Changing one passive byte before commit must be detected.
    auto damaged=planned.playerAfter;
    damaged.records[0].encodedRecord[5]^=1;
    assert(!TradePassiveStacksUnchanged350N(original,damaged));
    // A complete instance returns, while the anonymous record stays untouched.
    const TradeMove back{TradeSide::Companion,0,move.code,move.key,move.checksum,false,15,0};
    TradePlan reverse;
    assert(PlanItemMoves(planned.playerAfter,planned.companionAfter,{back},limits,relocate,reverse)==TradePlanResult::Ready);
    assert(reverse.playerAfter.records.size()==2 && reverse.companionAfter.records.empty());
    assert(TradePassiveStacksUnchanged350N(original,reverse.playerAfter));
    auto duplicate=Parse(Make({2,2}),2);
    assert(duplicate.hasDuplicateInstanceIdentity && !duplicate.TradeEligible());
    auto cursor=original;cursor.hasCursorItem=true;
    assert(!cursor.TradeEligible());
}
