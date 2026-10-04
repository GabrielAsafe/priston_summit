#include <cassert>
#include "gameplay/commerce/v2/RestorationTransactionService.h"

using namespace gameplay;
using namespace gameplay::commerce;

class FakeWallet final : public ICoinWalletQuery, public ICoinWalletCommands
{
public:
    CoinAmount balance = 100;
    bool rejectDebit = false;
    bool rejectCredit = false;
    bool TryGetCoinWallet(PlayerId p, CoinWalletSnapshot& out) const override
    { out = CoinWalletSnapshot(p, balance); return true; }
    CoinMutationResult CreditCoins(PlayerId p, CoinAmount a, CoinReason) override
    { CoinMutationResult r; r.player=p;r.requested=a;r.before=balance;r.after=balance; if(rejectCredit){r.code=CoinMutationCode::LegacyRejected;return r;} balance+=a;r.after=balance;r.code=CoinMutationCode::Ok;return r; }
    CoinMutationResult DebitCoins(PlayerId p, CoinAmount a, CoinReason) override
    { CoinMutationResult r; r.player=p;r.requested=a;r.before=balance;r.after=balance; if(rejectDebit){r.code=CoinMutationCode::LegacyRejected;return r;} if(balance<a){r.code=CoinMutationCode::InsufficientFunds;return r;} balance-=a;r.after=balance;r.code=CoinMutationCode::Ok;return r; }
};

class FakeStore final : public IRestorationClaimStore
{
public:
    bool consume = true;
    bool restore = true;
    int consumes = 0;
    int restores = 0;
    bool Consume(const RestorationRecordSnapshot&) override { ++consumes; return consume; }
    bool Restore(const RestorationRecordSnapshot&) override { ++restores; return restore; }
};
class FakeGrant final : public IRestorationGrant
{
public:
    bool ok = true;
    int grants = 0;
    bool Grant(const RestorationRecordSnapshot&) override { ++grants; return ok; }
};

static RestorationTransactionRequest Request()
{
    RestorationTransactionRequest q;
    q.player = PlayerId(7u);
    q.price = 30;
    q.record.account = "acc";
    q.record.character = "char";
    q.record.itemName = "item";
    q.record.itemAging = 10;
    q.record.head = 11u;
    q.record.checksum = 22u;
    return q;
}

int main()
{
    FakeWallet wallet; FakeStore store; FakeGrant grant;
    RestorationTransactionService service(wallet, wallet, store, grant);

    auto r = service.Restore(Request());
    assert(r.Restored()); assert(wallet.balance == 70); assert(store.consumes == 1); assert(grant.grants == 1);

    wallet.balance = 100; store.consume = false; store.consumes = 0; grant.grants = 0;
    r = service.Restore(Request());
    assert(r.code == RestorationTransactionCode::RecordConsumeRejected); assert(wallet.balance == 100); assert(grant.grants == 0);

    wallet.balance = 100; store.consume = true; store.restore = true; grant.ok = false; store.restores = 0;
    r = service.Restore(Request());
    assert(r.code == RestorationTransactionCode::GrantRejected); assert(wallet.balance == 100); assert(store.restores == 1);

    wallet.balance = 100; store.restore = false;
    r = service.Restore(Request());
    assert(r.code == RestorationTransactionCode::RollbackFailed); assert(wallet.balance == 100);

    wallet.balance = 20; store.consume = true; grant.ok = true;
    r = service.Restore(Request());
    assert(r.code == RestorationTransactionCode::InsufficientFunds); assert(store.consumes >= 1); // prior calls only
    return 0;
}
