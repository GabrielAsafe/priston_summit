#include "gameplay/commerce/v2/PremiumPurchaseTransactionService.h"
#include <cassert>
#include <iostream>

class FakeWallet final : public gameplay::commerce::ICoinWalletQuery,
                         public gameplay::commerce::ICoinWalletCommands
{
public:
    gameplay::commerce::CoinAmount balance = 100;
    bool rejectDebit = false;
    bool rejectCredit = false;
    bool TryGetCoinWallet(gameplay::PlayerId p, gameplay::commerce::CoinWalletSnapshot& out) const override
    { out = {p, balance}; return true; }
    gameplay::commerce::CoinMutationResult CreditCoins(gameplay::PlayerId p, gameplay::commerce::CoinAmount a, gameplay::commerce::CoinReason) override
    { gameplay::commerce::CoinMutationResult r; r.player=p;r.requested=a;r.before=balance;r.after=balance; if(rejectCredit){r.code=gameplay::commerce::CoinMutationCode::LegacyRejected;return r;} balance+=a;r.after=balance;r.code=gameplay::commerce::CoinMutationCode::Ok;return r; }
    gameplay::commerce::CoinMutationResult DebitCoins(gameplay::PlayerId p, gameplay::commerce::CoinAmount a, gameplay::commerce::CoinReason) override
    { gameplay::commerce::CoinMutationResult r; r.player=p;r.requested=a;r.before=balance;r.after=balance; if(rejectDebit){r.code=gameplay::commerce::CoinMutationCode::LegacyRejected;return r;} balance-=a;r.after=balance;r.code=gameplay::commerce::CoinMutationCode::Ok;return r; }
};
class FakeGrant final : public gameplay::commerce::IPremiumPurchaseGrant { public: bool ok=true; bool Grant(const gameplay::commerce::PremiumPurchaseRequest&) override { return ok; } };
int main(){ using namespace gameplay::commerce; FakeWallet w; FakeGrant g; PremiumPurchaseTransactionService s(w,w,g); PremiumPurchaseRequest q{gameplay::PlayerId(7),30,"item"}; auto r=s.Purchase(q); assert(r.Purchased()&&w.balance==70); w.balance=100;g.ok=false;r=s.Purchase(q);assert(r.code==PremiumPurchaseCode::GrantRejected&&w.balance==100); w.balance=100;w.rejectCredit=true;r=s.Purchase(q);assert(r.code==PremiumPurchaseCode::RollbackFailed&&w.balance==70); std::cout<<"premium purchase transaction smoke: PASS\n"; }
