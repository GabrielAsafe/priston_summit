#include "gameplay/companions/v2/CompanionTradeLiveHandoff350Q.h"
#include "gameplay/companions/v2/CompanionTradeDeliveryWire.h"
#include <cassert>
#include <cstring>
#include <cstdint>
#include <iostream>
#include <vector>
using namespace gameplay::companions::v2;
static void put32(std::vector<std::uint8_t>& x,std::size_t pos,std::uint32_t v) {
    assert(pos+4<=x.size());std::memcpy(&x[pos],&v,4);
}
int main() {
    // Simulated complete record with only the inventory changing.
    const BufferedTradeEnvelopeLayout layout(0,4,8,12,16,32,48,512);
    std::vector<std::uint8_t> before(88,0),after(88,0);
    put32(before,0,88);put32(before,4,1);put32(before,8,1);put32(before,12,40);
    std::memcpy(&before[16],"Owner",6);
    for(unsigned i=0;i<40;++i) before[48+i]=static_cast<std::uint8_t>(i+1);
    after=before;after[55]^=0x3a;
    TradeSha256Digest expected{};
    assert(TradeCanonicalInventoryDigest::Compute(after,layout,expected));
    TradeLiveFirstSave350Q gate;
    SaveSessionKey session(7,0);
    assert(gate.Arm(session,expected));
    assert(!gate.Arm(session,expected));
    assert(gate.Pending(session));
    assert(!gate.CheckOrReject(session,before.data(),before.size(),layout));
    assert(gate.Pending(session));
    assert(gate.CheckOrReject(session,after.data(),after.size(),layout));
    assert(gate.Pending(session)); // unsuccessful legacy validation cannot disarm replay barrier
    gate.MarkAccepted(session);
    assert(!gate.Pending(session));
    assert(gate.CheckOrReject(session,before.data(),before.size(),layout));

    SaveActivityFence fence;
    assert(fence.BeginReceive(session,0,1));fence.EndReceive(session,true,true);
    std::uint64_t token=0;
    assert(fence.TryReserve(session,token) && token);
    assert(fence.SealCommittedForReconnect(session,token));
    bool tracked=false;
    assert(!fence.BeginMutation(session,tracked));
    assert(fence.ReleaseAfterVerifiedLiveApply(session,token));
    assert(!fence.IsSealedForReconnect(session));
    assert(!fence.ReleaseAfterVerifiedLiveApply(session,token));
    assert(fence.BeginMutation(session,tracked));fence.EndMutation(session,tracked);

    std::vector<std::uint8_t> payload(6400);
    for(std::size_t i=0;i<payload.size();++i) payload[i]=static_cast<std::uint8_t>((i*37u)%251);
    const std::int32_t opcode=0x48479C18,version=1;
    const auto beforeDigest=TradeSha256::Hash(before.data(),before.size());
    TradeDeliveryReceiver receiver;
    std::vector<std::uint8_t> result;
    std::size_t chunks=0;
    assert(SendCommittedImage(payload,opcode,version,123,456,beforeDigest,"Owner","Companion",
        [&](const TradeDeliveryChunk& chunk) {
            ++chunks;
            const auto status=receiver.Accept(chunk,opcode,version,123,"Owner","Companion",result);
            return status==TradeDeliveryPacketStatus::Incomplete ||
                   status==TradeDeliveryPacketStatus::Complete;
        }));
    assert(chunks==3 && result==payload && receiver.TransactionId()==456);
    std::cout << "SUMMIT350Q_NATIVE=PASS (first-save replay, generation zero, lease, chunk delivery)\n";
}
