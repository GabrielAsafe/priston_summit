#include <cassert>

#include "gameplay/commerce/v2/PremiumPurchaseReplayGuard.h"

int main()
{
    using namespace gameplay::commerce;
    PremiumPurchaseReplayGuardService memoryStore;
    IPremiumPurchaseReplayStore& store = memoryStore;

    PremiumPurchaseOperation operation;
    operation.purchaserKey = "account";
    operation.scope = PremiumPurchaseScope::CoinShop;
    operation.requestId = 42u;
    operation.itemCode = "BI101";

    assert(store.Reserve(operation).code == PremiumPurchaseReplayCode::Reserved);
    assert(store.Commit(operation));
    assert(store.Reserve(operation).code == PremiumPurchaseReplayCode::DuplicateCommitted);
    return 0;
}
