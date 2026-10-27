#include "gameplay/companions/v2/CompanionTradeAcceptedRecord350E.h"
#include "gameplay/companions/v2/CompanionTradeNormalizedRecord350D.h"
#include <cassert>
#include <atomic>
#include <cstring>
#include <iostream>
#include <thread>
#include <vector>
using namespace gameplay::companions::v2;
int main() {
    AcceptedRecordBridge350E bridge;
    SaveSessionKey session(123,4);
    std::vector<std::uint8_t> buffer(256,0xAA);
    std::int32_t declared=72;
    std::memcpy(buffer.data(),&declared,4);
    assert(bridge.Publish(session,buffer.data(),buffer.size(),128));
    AcceptedRecordEvidence350E first;
    assert(bridge.Copy(session,first));
    assert(first.bytes.size()==128 && first.declaredBytes==72);
    assert(first.exactWriteBytes==128);
    assert(first.bytes[127]==0xAA);
    buffer[10]=0x22; // server save receive buffer is mutable and must not alias evidence
    AcceptedRecordEvidence350E copy;
    assert(bridge.Copy(session,copy));
    assert(copy.bytes[10]==0xAA && copy.digest==first.digest);
    TradeLegacyRecordLayout350D layout{};
    layout.declaredSizeOffset=0;
    layout.minimumWriteBytes=128;
    layout.gameSaveOffset=16;
    layout.gameSaveBytes=8;
    layout.inventoryEndOffset=32;
    TradeNormalizedRecord350D normalized;
    auto change=[](std::uint8_t* p,std::size_t n){if(n!=8)return false;p[0]=0xEF;return true;};
    assert(BuildNormalizedRecord350D(first.bytes.data(),first.bytes.size(),layout,
                                     change,normalized)==TradeRecordNormalizeCode350D::Ready);
    assert(normalized.bytes[16]==0xEF && first.bytes[16]==0xAA);
    assert(normalized.bytes.size()==128);
    // Rejected new save must destroy stale acceptance evidence.
    declared=260;
    std::memcpy(buffer.data(),&declared,4);
    assert(!bridge.Publish(session,buffer.data(),buffer.size(),128));
    assert(!bridge.Copy(session,copy));
    declared=140;
    std::memcpy(buffer.data(),&declared,4);
    assert(bridge.Publish(session,buffer.data(),buffer.size(),128));
    assert(bridge.Copy(session,copy) && copy.exactWriteBytes==140);
    bridge.Invalidate(session);
    assert(!bridge.Copy(session,copy));
    // SUMMIT350I: seed one valid record before starting competing threads.
    // Without a seed the reader may finish all iterations before the writer
    // is ever scheduled, making seen>0 a scheduling-dependent false failure.
    declared=144;
    std::memcpy(buffer.data(),&declared,4);
    std::fill(buffer.begin()+4,buffer.end(),0xAA);
    assert(bridge.Publish(session,buffer.data(),buffer.size(),128));
    // Concurrent save producers and consumers cannot return torn copies.
    std::atomic<int> seen{0};
    std::thread writer([&]{
        for(int i=0;i<1500;++i) {
            std::vector<std::uint8_t> bytes(256,static_cast<std::uint8_t>(i));
            std::int32_t sz=144;
            std::memcpy(bytes.data(),&sz,4);
            assert(bridge.Publish(session,bytes.data(),bytes.size(),128));
        }
    });
    std::thread reader([&]{
        for(int i=0;i<1500;++i) {
            AcceptedRecordEvidence350E out;
            if(!bridge.Copy(session,out))continue;
            auto digest=TradeSha256::Hash(out.bytes.data(),out.bytes.size());
            assert(digest==out.digest);
            assert(out.bytes.size()==144);
            for(std::size_t j=4;j<out.bytes.size();++j)assert(out.bytes[j]==out.bytes[4]);
            ++seen;
        }
    });
    writer.join();reader.join();
    assert(seen>0);
    // Bounded registry evicts oldest but retains the most recently published.
    for(std::uint32_t i=1;i<300;++i) {
        SaveSessionKey key(i+2000,1);
        assert(bridge.Publish(key,buffer.data(),buffer.size(),128));
    }
    assert(bridge.Size()==AcceptedRecordBridge350E::kMaxSessions);
    AcceptedRecordEvidence350E recent;
    assert(bridge.Copy(SaveSessionKey(2299,1),recent));
    std::cout << "SUMMIT350E_NATIVE=PASS (real receipt evidence, normalized image, concurrency, bound, tamper)\n";
}
