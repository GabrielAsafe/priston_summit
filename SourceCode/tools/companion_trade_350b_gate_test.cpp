#include "gameplay/companions/v2/CompanionTradeParticipantGate350B.h"
#include <cassert>
#include <iostream>
#include <atomic>
#include <thread>
#include <vector>
using namespace gameplay::companions::v2;
int main() {
    TradeParticipantGate350B gate;
    assert(!gate.TryReserve(4,4));
    assert(!gate.TryReserve(0,4));
    assert(gate.TryReserve(4,5));
    assert(!gate.TryReserve(5,6));
    assert(!gate.TryReserve(6,4));
    assert(gate.IsReserved(4));
    assert(gate.IsReserved(5));
    {
        TradeParticipantGate350B::LoadGuard bad(gate,4);
        assert(!bad.Allowed());
    }
    assert(gate.ReleaseVerified(4,5));
    assert(!gate.IsReserved(4));
    {
        TradeParticipantGate350B::LoadGuard guard(gate,4);
        assert(guard.Allowed());
        assert(!gate.TryReserve(4,5));
        {
            TradeParticipantGate350B::LoadGuard parallel(gate,4);
            assert(parallel.Allowed());
        }
        assert(!gate.TryReserve(4,5));
    }
    assert(gate.TryReserve(4,5));
    assert(gate.ReleaseVerified(4,5));
    std::atomic<int> accepted{0};
    std::vector<std::thread> competitors;
    for(int i=0;i<20;++i) competitors.emplace_back([&]{
        if(gate.TryReserve(22,33)) ++accepted;
    });
    for(auto& t:competitors)t.join();
    assert(accepted==1);
    assert(gate.ReleaseVerified(22,33));
    std::cout << "SUMMIT350B_GATE=PASS (login exclusion, duplicate, orientation, threads)\n";
}
