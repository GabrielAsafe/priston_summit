#include "gameplay/companions/v2/CompanionTradePreviewPolicy350P.h"
#include <cassert>
#include <cstdio>
using namespace gameplay::companions::v2;
int main() {
    CompanionTradeInventorySnapshot s;
    s.primaryCount=2;
    CompanionTradeItemRecord passive;
    passive.recordIndex=0; passive.itemCode=0x10000002; passive.itemKey=0;
    passive.itemChecksum=0; passive.position=0; passive.encodedRecord={5,0,0,0,7};
    CompanionTradeItemRecord equipped;
    equipped.recordIndex=1;equipped.itemCode=0x10000003; equipped.itemKey=123;
    equipped.itemChecksum=456;equipped.position=5;equipped.encodedRecord={5,0,0,0,7};
    s.records={passive,equipped};s.hasMissingInstanceIdentity=true;
    assert(CanListTradePreview350P(CompanionTradeInventoryReadCode::Ok,s));
    assert(CompanionTradePreviewRevision(s)!=0);
    TradePreviewPositionIndex350P positions(&s);
    assert(positions.PositionOrBag350P(0x10000003,123,456)==5);
    assert(positions.PositionOrBag350P(0x10000004,124,457)==-2);
    assert(positions.PositionOrBag350P(0x10000002,0,0)==-2);
    TradePreviewPositionIndex350P empty(nullptr);
    assert(empty.PositionOrBag350P(0x10000003,123,456)==0);
    s.hasCursorItem=true;
    assert(!CanListTradePreview350P(CompanionTradeInventoryReadCode::Ok,s));
    assert(!CanListTradePreview350P(CompanionTradeInventoryReadCode::CodecRejected,s));
    s.hasCursorItem=false;
    s.hasDuplicateInstanceIdentity=true;
    assert(!CanListTradePreview350P(CompanionTradeInventoryReadCode::Ok,s));
    std::puts("SUMMIT350P_PREVIEW=PASS (passive shown; equipped/unknown disabled; unsafe blocked)");
}
