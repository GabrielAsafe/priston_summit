#include <cassert>
#include <map>
#include <set>

#include "gameplay/commerce/v2/PersonalShopTransactionService.h"

using namespace gameplay;
using namespace gameplay::commerce;
using namespace gameplay::inventory;

namespace
{
struct Wallets final : IWalletQuery, IWalletCommands, ICoinWalletQuery, ICoinWalletCommands
{
    std::map<std::uint32_t, CurrencyAmount> gold; std::map<std::uint32_t, CoinAmount> coins;
    bool rejectSellerCoin=false;
    bool TryGetWallet(PlayerId p, WalletSnapshot& o) const override { auto i=gold.find(p.value); if(i==gold.end()) return false; o=WalletSnapshot(p,i->second); return true; }
    WalletMutationResult Credit(PlayerId p,CurrencyAmount a,WalletReason) override { WalletMutationResult r; r.player=p;r.requested=a;auto i=gold.find(p.value);if(i==gold.end()){r.code=WalletMutationCode::PlayerNotFound;return r;}r.before=i->second;i->second+=a;r.after=i->second;r.code=WalletMutationCode::Ok;return r; }
    WalletMutationResult Debit(PlayerId p,CurrencyAmount a,WalletReason) override { WalletMutationResult r; r.player=p;r.requested=a;auto i=gold.find(p.value);if(i==gold.end()){r.code=WalletMutationCode::PlayerNotFound;return r;}r.before=i->second;if(i->second<a){r.after=i->second;r.code=WalletMutationCode::InsufficientFunds;return r;}i->second-=a;r.after=i->second;r.code=WalletMutationCode::Ok;return r; }
    bool TryGetCoinWallet(PlayerId p,CoinWalletSnapshot& o) const override { auto i=coins.find(p.value);if(i==coins.end())return false;o=CoinWalletSnapshot(p,i->second);return true; }
    CoinMutationResult CreditCoins(PlayerId p,CoinAmount a,CoinReason) override { CoinMutationResult r;r.player=p;r.requested=a;auto i=coins.find(p.value);if(i==coins.end()){r.code=CoinMutationCode::PlayerNotFound;return r;}r.before=i->second;if(rejectSellerCoin&&p.value==1u){r.after=i->second;r.code=CoinMutationCode::LegacyRejected;return r;}i->second+=a;r.after=i->second;r.code=CoinMutationCode::Ok;return r; }
    CoinMutationResult DebitCoins(PlayerId p,CoinAmount a,CoinReason) override { CoinMutationResult r;r.player=p;r.requested=a;auto i=coins.find(p.value);if(i==coins.end()){r.code=CoinMutationCode::PlayerNotFound;return r;}r.before=i->second;if(i->second<a){r.after=i->second;r.code=CoinMutationCode::InsufficientFunds;return r;}i->second-=a;r.after=i->second;r.code=CoinMutationCode::Ok;return r; }
};
struct Inventory final : IInventoryQuery, IInventoryCommands
{
    std::map<std::uint32_t,std::set<std::uint32_t> > owned;
    InventoryPresenceResult Contains(PlayerId p,ItemInstanceId item) const override { InventoryPresenceResult r;r.player=p;r.item=item;auto i=owned.find(p.value);r.code=i!=owned.end()&&i->second.count(item.key)?InventoryPresenceCode::Present:InventoryPresenceCode::Absent;return r; }
    bool TryGetSnapshot(PlayerId,InventorySnapshot&) const override{return false;}
    InventoryMutationResult Add(PlayerId p,ItemInstanceId item) override {InventoryMutationResult r;r.player=p;r.item=item;if(owned[p.value].count(item.key)){r.code=InventoryMutationCode::AlreadySatisfied;return r;}owned[p.value].insert(item.key);r.code=InventoryMutationCode::Applied;return r;}
    InventoryMutationResult Remove(PlayerId p,ItemInstanceId item) override {InventoryMutationResult r;r.player=p;r.item=item;r.code=owned[p.value].erase(item.key)?InventoryMutationCode::Applied:InventoryMutationCode::ItemNotFound;return r;}
    InventoryTransferResult Transfer(PlayerId,PlayerId,ItemInstanceId) override{return InventoryTransferResult();}
    InventoryGrantResult Grant(const ItemGrant&) override{return InventoryGrantResult();}
};
struct Reservations final:IInventoryReservationService{bool active=false;int releases=0;InventoryReservationResult Reserve(const InventoryReservationRequest& q) override{InventoryReservationResult r;if(q.items.empty())return r;active=true;r.code=InventoryReservationCode::Reserved;r.token=InventoryReservationToken(1);return r;}bool Release(InventoryReservationToken t) override{if(active&&t.Valid()){active=false;++releases;return true;}return false;}std::size_t ReleasePlayer(PlayerId)override{return 0;}bool IsReserved(PlayerId,ItemInstanceId)const override{return active;}};
struct Potions final:IPotionStackCommands{PotionStackMutationResult AddStack(const PotionStackMutationRequest&)override{return PotionStackMutationResult();}PotionStackMutationResult RemoveStack(const PotionStackMutationRequest&)override{return PotionStackMutationResult();}PotionStackTransferResult TransferStack(const PotionStackTransferRequest& q)override{PotionStackTransferResult r;r.request=q;r.code=PotionStackTransferCode::Applied;return r;}};
}
int main(){
 Wallets w;Inventory inv;Reservations res;Potions pot;w.gold[1]=100;w.gold[2]=1000;w.coins[1]=5;w.coins[2]=50;ItemInstanceId item(ItemId(99),7,8);inv.owned[1].insert(item.key);
 PersonalShopTransactionService svc(w,w,inv,inv,res,w,w,pot);PersonalShopTradeRequest q;q.seller=PlayerId(1);q.buyer=PlayerId(2);q.itemCode=99;q.head=7;q.checksum=8;q.quantity=1;q.goldPrice=200;q.coinPrice=10;
 PersonalShopTransactionResult r=svc.Commit(PersonalShopTransactionRequest(q,false));assert(r.trade.Committed());assert(inv.owned[2].count(7));assert(!inv.owned[1].count(7));assert(w.gold[1]==300&&w.gold[2]==800);assert(w.coins[1]==15&&w.coins[2]==40);assert(res.releases==1);
 // Late seller coin-credit failure compensates inventory + gold + buyer coin debit.
 inv.owned[2].erase(7);inv.owned[1].insert(7);w.gold[1]=100;w.gold[2]=1000;w.coins[1]=5;w.coins[2]=50;w.rejectSellerCoin=true;
 r=svc.Commit(PersonalShopTransactionRequest(q,false));assert(r.trade.code==PersonalShopTradeCode::CoinWalletRejected);assert(inv.owned[1].count(7)&&!inv.owned[2].count(7));assert(w.gold[1]==100&&w.gold[2]==1000);assert(w.coins[1]==5&&w.coins[2]==50);assert(res.releases==2);
 return 0;
}
