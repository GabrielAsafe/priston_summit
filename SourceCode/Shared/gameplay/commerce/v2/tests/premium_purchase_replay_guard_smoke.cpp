#include <cassert>
#include <iostream>

#include "gameplay/commerce/v2/PremiumPurchaseReplayGuard.h"
#include "gameplay/commerce/v2/PremiumPurchaseRequestSequence.h"

using namespace gameplay::commerce;

static PremiumPurchaseOperation Op(const char* account,
                                   PremiumPurchaseScope scope,
                                   unsigned id,
                                   const char* item)
{
    PremiumPurchaseOperation op;
    op.purchaserKey = account;
    op.scope = scope;
    op.requestId = id;
    op.itemCode = item;
    return op;
}

int main()
{
    PremiumPurchaseRequestSequence sequence;
    assert(sequence.Next() == 1u);
    assert(sequence.Next() == 2u);

    PremiumPurchaseReplayGuardService guard(4u);
    const PremiumPurchaseOperation first = Op("alice", PremiumPurchaseScope::CoinShop, 7u, "BI101");
    PremiumPurchaseReplayResult result = guard.Reserve(first);
    assert(result.Reserved());
    assert(guard.Reserve(first).code == PremiumPurchaseReplayCode::DuplicateInFlight);
    assert(guard.Commit(first));
    assert(guard.Reserve(first).code == PremiumPurchaseReplayCode::DuplicateCommitted);

    const PremiumPurchaseOperation conflict = Op("alice", PremiumPurchaseScope::CoinShop, 7u, "BI102");
    assert(guard.Reserve(conflict).code == PremiumPurchaseReplayCode::RequestConflict);

    // The same request id in the other premium store is a different operation.
    const PremiumPurchaseOperation time = Op("alice", PremiumPurchaseScope::TimeShop, 7u, "BI102");
    assert(guard.Reserve(time).Reserved());
    assert(guard.Abort(time));
    assert(guard.Reserve(time).Reserved());

    // Account identity survives runtime player-serial regeneration/reconnect.
    assert(guard.Reserve(first).code == PremiumPurchaseReplayCode::DuplicateCommitted);

    const PremiumPurchaseOperation legacy = Op("legacy", PremiumPurchaseScope::CoinShop, 0u, "BI101");
    assert(guard.Reserve(legacy).code == PremiumPurchaseReplayCode::LegacyUntracked);

    std::cout << "premium purchase replay guard smoke: PASS\n";
    return 0;
}
