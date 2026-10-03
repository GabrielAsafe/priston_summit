#include <cassert>

#include "gameplay/commerce/v2/TradeSessionService.h"

int main()
{
    using namespace gameplay;
    using namespace gameplay::commerce;

    TradeReplayGuardService guard(8000u);
    const PlayerHandle first(PlayerId(100u), 7u);
    const PlayerHandle reconnect(PlayerId(100u), 8u);

    TradeCommitFingerprint fingerprint;
    fingerprint.sender = first.id;
    fingerprint.receiver = PlayerId(200u);
    fingerprint.encodedMoney = 1234;
    fingerprint.items[0].itemCode = 10u;
    fingerprint.items[0].itemKey = 20u;
    fingerprint.items[0].itemChecksum = 30u;

    TradeCommitGuardResult result = guard.Check(first, fingerprint, 1000u);
    assert(result.Fresh());
    const TradeId id = result.tradeId;

    guard.MarkCommitted(first, fingerprint, 1000u);
    result = guard.Check(first, fingerprint, 8999u);
    assert(result.Duplicate());
    assert(result.tradeId == id);

    result = guard.Check(first, fingerprint, 9000u);
    assert(result.Fresh());

    // Same recyclable serial with a new connection generation is a fresh
    // session immediately, even inside the old connection's duplicate window.
    result = guard.Check(reconnect, fingerprint, 1001u);
    assert(result.Fresh());
    guard.MarkCommitted(reconnect, fingerprint, 1001u);
    assert(guard.Check(reconnect, fingerprint, 1002u).Duplicate());
    assert(guard.Clear(first) == 0u); // stale generation was pruned on mark.
    assert(guard.Clear(reconnect) == 1u);

    return 0;
}
