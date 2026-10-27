#include "gameplay/companions/v2/CompanionTradeParticipantGate350B.h"
#include "gameplay/companions/v2/CompanionTradeParticipantIdentity.h"
#include <atomic>
#include <cassert>
#include <thread>
#include <iostream>
using namespace gameplay::companions::v2;
int main() {
    auto owner=TradeParticipantId("account","online");
    auto companion=TradeParticipantId("account","companion");
    assert(owner && companion && owner!=companion);
    TradeParticipantGate350B gate;
    {
        TradeParticipantGate350B::UnknownWriterGuard350G writer(gate);
        assert(writer.Allowed());
        assert(!gate.TryReserve(owner,companion));
        TradeParticipantGate350B::UnknownWriterGuard350G nested(gate);
        assert(nested.Allowed());
    }
    assert(gate.TryReserve(owner,companion));
    {
        TradeParticipantGate350B::UnknownWriterGuard350G blocked(gate);
        assert(!blocked.Allowed());
    }
    assert(gate.ReleaseVerified(owner,companion));
    // An uncertain partial character import fails closed globally.
    {
        TradeParticipantGate350B poisoned;
        {
            TradeParticipantGate350B::UnknownWriterGuard350G writer(poisoned);
            assert(writer.Allowed());
            writer.MarkUncertain();
        }
        assert(!poisoned.TryReserve(owner,companion));
    }
    // Race: unknown writer and reservation cannot be simultaneously held.
    for(int i=0;i<600;++i) {
        std::atomic<bool> go{false};
        std::atomic<bool> writerAllowed{false}, pairAllowed{false};
        std::thread a([&]{
            while(!go.load()) std::this_thread::yield();
            TradeParticipantGate350B::UnknownWriterGuard350G writer(gate);
            writerAllowed.store(writer.Allowed());
            if(writer.Allowed()) assert(!gate.TryReserve(owner,companion));
        });
        std::thread b([&]{
            while(!go.load()) std::this_thread::yield();
            const bool ok=gate.TryReserve(owner,companion);
            pairAllowed.store(ok);
            if(ok) {
                TradeParticipantGate350B::UnknownWriterGuard350G writer(gate);
                assert(!writer.Allowed());
                assert(gate.ReleaseVerified(owner,companion));
            }
        });
        go.store(true);a.join();b.join();
        assert(writerAllowed || pairAllowed);
    }
    std::cout << "SUMMIT350G_NATIVE=PASS (global unknown writer, character lifecycle, races, fail-closed)\n";
}
