#include <cassert>
#include <map>
#include <set>
#include <utility>

#include "gameplay/commerce/v2/TradeTransactionService.h"

using namespace gameplay;
using namespace gameplay::commerce;
using namespace gameplay::inventory;

namespace
{
struct Wallets final : IWalletQuery, IWalletCommands, ICoinWalletQuery, ICoinWalletCommands
{
    std::map<std::uint32_t, CurrencyAmount> gold;
    std::map<std::uint32_t, CoinAmount> coins;
    bool rejectReceiverCoinCredit = false;

    bool TryGetWallet(PlayerId player, WalletSnapshot& out) const override
    {
        std::map<std::uint32_t, CurrencyAmount>::const_iterator it = gold.find(player.value);
        if(it == gold.end()) return false;
        out = WalletSnapshot(player, it->second); return true;
    }
    WalletMutationResult Credit(PlayerId player, CurrencyAmount amount, WalletReason) override
    {
        WalletMutationResult r; r.player=player; r.requested=amount;
        std::map<std::uint32_t, CurrencyAmount>::iterator it=gold.find(player.value);
        if(it==gold.end()) { r.code=WalletMutationCode::PlayerNotFound; return r; }
        r.before=it->second; it->second+=amount; r.after=it->second; r.code=WalletMutationCode::Ok; return r;
    }
    WalletMutationResult Debit(PlayerId player, CurrencyAmount amount, WalletReason) override
    {
        WalletMutationResult r; r.player=player; r.requested=amount;
        std::map<std::uint32_t, CurrencyAmount>::iterator it=gold.find(player.value);
        if(it==gold.end()) { r.code=WalletMutationCode::PlayerNotFound; return r; }
        r.before=it->second;
        if(it->second<amount) { r.after=it->second; r.code=WalletMutationCode::InsufficientFunds; return r; }
        it->second-=amount; r.after=it->second; r.code=WalletMutationCode::Ok; return r;
    }
    bool TryGetCoinWallet(PlayerId player, CoinWalletSnapshot& out) const override
    {
        std::map<std::uint32_t, CoinAmount>::const_iterator it = coins.find(player.value);
        if(it == coins.end()) return false;
        out = CoinWalletSnapshot(player, it->second); return true;
    }
    CoinMutationResult CreditCoins(PlayerId player, CoinAmount amount, CoinReason) override
    {
        CoinMutationResult r; r.player=player; r.requested=amount;
        std::map<std::uint32_t, CoinAmount>::iterator it=coins.find(player.value);
        if(it==coins.end()) { r.code=CoinMutationCode::PlayerNotFound; return r; }
        r.before=it->second;
        if(rejectReceiverCoinCredit && player.value==2u) { r.after=it->second; r.code=CoinMutationCode::LegacyRejected; return r; }
        it->second+=amount; r.after=it->second; r.code=CoinMutationCode::Ok; return r;
    }
    CoinMutationResult DebitCoins(PlayerId player, CoinAmount amount, CoinReason) override
    {
        CoinMutationResult r; r.player=player; r.requested=amount;
        std::map<std::uint32_t, CoinAmount>::iterator it=coins.find(player.value);
        if(it==coins.end()) { r.code=CoinMutationCode::PlayerNotFound; return r; }
        r.before=it->second;
        if(it->second<amount) { r.after=it->second; r.code=CoinMutationCode::InsufficientFunds; return r; }
        it->second-=amount; r.after=it->second; r.code=CoinMutationCode::Ok; return r;
    }
};

struct Inventory final : IInventoryCommands
{
    std::map<std::uint32_t, std::set<std::uint32_t> > owned;
    InventoryMutationResult Add(PlayerId player, ItemInstanceId item) override
    { InventoryMutationResult r; r.player=player; r.item=item; owned[player.value].insert(item.key); r.code=InventoryMutationCode::Applied; return r; }
    InventoryMutationResult Remove(PlayerId player, ItemInstanceId item) override
    { InventoryMutationResult r; r.player=player; r.item=item; r.code=owned[player.value].erase(item.key)?InventoryMutationCode::Applied:InventoryMutationCode::ItemNotFound; return r; }
    InventoryTransferResult Transfer(PlayerId source, PlayerId destination, ItemInstanceId item) override
    {
        InventoryTransferResult r; r.source=source; r.destination=destination; r.item=item;
        if(!owned[source.value].erase(item.key)) { r.code=InventoryTransferCode::SourceItemNotFound; return r; }
        owned[destination.value].insert(item.key); r.code=InventoryTransferCode::Applied; return r;
    }
    InventoryGrantResult Grant(const ItemGrant&) override { return InventoryGrantResult(); }
};

struct Reservations final : IInventoryReservationService
{
    bool active=false; int releases=0;
    InventoryReservationResult Reserve(const InventoryReservationRequest& req) override
    { InventoryReservationResult r; if(!req.owner.Valid()||req.items.empty()) return r; active=true; r.code=InventoryReservationCode::Reserved; r.token=InventoryReservationToken(1); return r; }
    bool Release(InventoryReservationToken token) override { if(token.Valid()&&active){active=false;++releases;return true;} return false; }
    std::size_t ReleasePlayer(PlayerId) override { return 0; }
    bool IsReserved(PlayerId, ItemInstanceId) const override { return active; }
};

struct Potions final : IPotionStackCommands
{
    PotionStackMutationResult AddStack(const PotionStackMutationRequest&) override { return PotionStackMutationResult(); }
    PotionStackMutationResult RemoveStack(const PotionStackMutationRequest&) override { return PotionStackMutationResult(); }
    PotionStackTransferResult TransferStack(const PotionStackTransferRequest& req) override
    { PotionStackTransferResult r; r.request=req; r.code=PotionStackTransferCode::Applied; return r; }
};
}

int main()
{
    Wallets wallets; Inventory inventory; Reservations reservations; Potions potions;
    wallets.gold[1]=1000; wallets.gold[2]=100;
    wallets.coins[1]=50; wallets.coins[2]=5;
    ItemInstanceId sword(ItemId(10), 77, 88); inventory.owned[1].insert(sword.key);
    TradeTransactionService service(wallets, wallets, inventory, reservations, wallets, wallets, potions);

    TradeExactCommitRequest ok; ok.sender=PlayerId(1); ok.receiver=PlayerId(2); ok.gold=200; ok.coins=10; ok.items[0]=sword;
    TradeExactCommitResult result=service.CommitExactTrade(ok);
    assert(result.Committed());
    assert(inventory.owned[2].count(sword.key)==1); assert(wallets.gold[1]==800 && wallets.gold[2]==300);
    assert(wallets.coins[1]==40 && wallets.coins[2]==15); assert(!reservations.active && reservations.releases==1);

    // Late coin failure must compensate item and gold before returning.
    inventory.owned[2].erase(sword.key); inventory.owned[1].insert(sword.key);
    wallets.gold[1]=1000; wallets.gold[2]=100; wallets.coins[1]=50; wallets.coins[2]=5;
    wallets.rejectReceiverCoinCredit=true;
    result=service.CommitExactTrade(ok);
    assert(result.code==TradeExactCommitCode::CoinRejected);
    assert(inventory.owned[1].count(sword.key)==1 && inventory.owned[2].count(sword.key)==0);
    assert(wallets.gold[1]==1000 && wallets.gold[2]==100);
    assert(wallets.coins[1]==50 && wallets.coins[2]==5);
    assert(!reservations.active && reservations.releases==2);
    return 0;
}
