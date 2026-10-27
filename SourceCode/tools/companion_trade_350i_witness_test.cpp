#include "gameplay/companions/v2/CompanionTradeAcceptedRecord350E.h"
#include <cassert>
#include <cstring>
#include <vector>
#include <iostream>
using namespace gameplay::companions::v2;
int main() {
    const SaveSessionKey key(701,3);
    SaveActivityFence fence;
    AcceptedRecordBridge350E bridge;
    std::vector<std::uint8_t> incoming(128,0x5a);
    const std::int32_t length=128;
    std::memcpy(incoming.data(), &length, sizeof length);
    assert(fence.BeginReceive(key,0,1));
    assert(bridge.Publish(key,incoming.data(),incoming.size(),128));
    fence.EndReceive(key,true,true);
    std::uint64_t accepted=0;
    assert(fence.TryAcceptedSaveSequence(key,accepted));
    assert(accepted!=0);
    std::vector<std::uint8_t> persisted=incoming;
    persisted[32]=0x22;persisted[39]=0x33; // only GameSaveInfo area may change
    assert(fence.BeginFlush(key));
    std::uint64_t flushed=0;
    assert(fence.TryActiveFlushAcceptedSequence(key,flushed));
    assert(flushed==accepted);
    assert(bridge.PublishPersisted(key,flushed,persisted.data(),persisted.size(),32,8));
    fence.EndFlush(key);
    AcceptedRecordEvidence350E result;
    assert(bridge.Copy(key,result));
    assert(result.persistedSequence==accepted);
    assert(result.bytes!=result.persistedBytes);
    assert(result.persistedBytes==persisted);
    assert(result.persistedDigest==TradeSha256::Hash(persisted.data(),persisted.size()));
    // A later, accepted save invalidates the old persisted witness.
    incoming[45]=0x77;
    assert(fence.BeginReceive(key,0,1));
    assert(bridge.Publish(key,incoming.data(),incoming.size(),128));
    fence.EndReceive(key,true,true);
    std::uint64_t newer=0;
    assert(fence.TryAcceptedSaveSequence(key,newer) && newer!=accepted);
    assert(bridge.Copy(key,result) && !result.persistedSequence);
    // Do not attest a DAT changing any protected byte outside GameSaveInfo.
    auto invalid=incoming;invalid[55]^=1;
    assert(!bridge.PublishPersisted(key,newer,invalid.data(),invalid.size(),32,8));
    assert(bridge.Copy(key,result) && !result.persistedSequence);
    // If the record is now exclusively reserved, an overlapping write can't
    // manufacture a fresh witness. This only proves an in-process fence.
    assert(fence.BeginFlush(key));
    assert(bridge.PublishPersisted(key,newer,incoming.data(),incoming.size(),32,8));
    fence.EndFlush(key);
    std::uint64_t token=0;
    assert(fence.TryReserve(key,token) && token);
    assert(!fence.TryActiveFlushAcceptedSequence(key,flushed));
    assert(fence.TryLeasedAcceptedSaveSequence(key,token,flushed) && flushed==newer);
    assert(bridge.Copy(key,result) && result.persistedSequence==flushed);
    assert(fence.ReleaseReservation(key,token));
    std::cout<<"SUMMIT350I_WITNESS=PASS (real write witness, stale receipt, changed bytes, lease)\n";
}
