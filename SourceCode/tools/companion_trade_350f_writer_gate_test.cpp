#include "gameplay/companions/v2/CompanionTradeParticipantGate350B.h"
#include "gameplay/companions/v2/CompanionTradeParticipantIdentity.h"
#include <atomic>
#include <cassert>
#include <thread>
#include <iostream>
using namespace gameplay::companions::v2;
int main() {
    TradeParticipantGate350B gate;
    const auto player=TradeParticipantId("account", "Online");
    const auto companion=TradeParticipantId("account", "Offline");
    const auto other=TradeParticipantId("account", "Third");
    assert(player && companion && other && player!=companion);
    assert(!gate.TryReserve(player, player));
    {
        TradeParticipantGate350B::WriteGuard w(gate, companion);
        assert(w.Allowed());
        assert(!gate.TryReserve(player, companion)); // writer in progress
        assert(gate.TryReserve(player,other)); // unrelated pair can progress
        assert(gate.ReleaseVerified(player,other));
        TradeParticipantGate350B::WriteGuard nested(gate,companion);
        assert(nested.Allowed());
    }
    assert(gate.TryReserve(player,companion));
    {
        TradeParticipantGate350B::WriteGuard blocked(gate,companion);
        assert(!blocked.Allowed()); // no blocking under world section
        TradeParticipantGate350B::WriteGuard unrelated(gate,other);
        assert(unrelated.Allowed());
    }
    assert(gate.ReleaseVerified(player,companion));
    {
        TradeParticipantGate350B::WriteGuard resumed(gate,companion);
        assert(resumed.Allowed());
    }
    // A queued save retains admission until its actual disk write, not just
    // until the producer has returned.
    assert(gate.BeginQueuedWrite(companion));
    assert(!gate.TryReserve(player,companion));
    assert(gate.FinishQueuedWrite(companion,true));
    assert(gate.TryReserve(player,companion));
    assert(!gate.BeginQueuedWrite(companion));
    assert(gate.ReleaseVerified(player,companion));
    assert(gate.BeginQueuedWrite(0)); // unknown owner blocks all pair commits
    assert(!gate.TryReserve(player,companion));
    assert(!gate.TryReserve(player,other));
    assert(gate.FinishQueuedWrite(0,true));
    assert(gate.TryReserve(player,companion));
    assert(gate.ReleaseVerified(player,companion));
    {
        TradeParticipantGate350B failed;
        assert(failed.BeginQueuedWrite(companion));
        assert(failed.FinishQueuedWrite(companion,false));
        assert(!failed.TryReserve(player,companion)); // bad disk flush quarantined
        assert(failed.TryReserve(player,other));
        assert(failed.ReleaseVerified(player,other));
        assert(failed.BeginQueuedWrite(0));
        assert(failed.FinishQueuedWrite(0,false));
        assert(!failed.TryReserve(player,other)); // unknown failed flush is global
    }
    // Race admission: reserve succeeds iff no writer has been admitted.
    for(int n=0;n<400;++n) {
        std::atomic<bool> go{false};
        std::atomic<bool> writerAllowed{false};
        std::atomic<bool> pairAllowed{false};
        std::thread writer([&] {
            while(!go.load()) std::this_thread::yield();
            TradeParticipantGate350B::WriteGuard w(gate,companion);
            writerAllowed.store(w.Allowed());
            if(w.Allowed()) std::this_thread::yield();
        });
        std::thread trade([&] {
            while(!go.load()) std::this_thread::yield();
            bool ok=gate.TryReserve(player,companion);
            pairAllowed.store(ok);
            if(ok) {
                TradeParticipantGate350B::WriteGuard denied(gate,companion);
                assert(!denied.Allowed());
                assert(gate.ReleaseVerified(player,companion));
            }
        });
        go.store(true);
        writer.join(); trade.join();
        assert(writerAllowed.load() || pairAllowed.load());
        assert(!gate.IsReserved(player));
    }
    std::cout<<"SUMMIT350F_GATE=PASS (online/offline writers, queued flushes, failed flushes, races)\n";
}
