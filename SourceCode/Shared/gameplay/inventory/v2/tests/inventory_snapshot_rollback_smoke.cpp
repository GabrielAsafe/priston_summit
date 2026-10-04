#include <algorithm>
#include <cassert>
#include <vector>
#include <iostream>

#include "gameplay/inventory/v2/InventorySnapshotRollback.h"

using namespace gameplay;
using namespace gameplay::inventory;

class FakeInventory final : public IInventoryQuery, public IInventoryCommands
{
public:
    explicit FakeInventory(PlayerId player) : player_(player), failAdd_(false), failRemove_(false) {}

    InventoryPresenceResult Contains(PlayerId player, ItemInstanceId item) const override
    {
        InventoryPresenceResult r; r.player=player; r.item=item;
        r.code = player != player_ ? InventoryPresenceCode::PlayerNotFound :
            (Has(item) ? InventoryPresenceCode::Present : InventoryPresenceCode::Absent);
        return r;
    }
    bool TryGetSnapshot(PlayerId player, InventorySnapshot& out) const override
    {
        if(player != player_) return false;
        out = InventorySnapshot(player);
        for(std::size_t i=0;i<items_.size();++i) out.slots.push_back(InventorySlot((std::uint32_t)i,items_[i]));
        return true;
    }
    InventoryMutationResult Add(PlayerId player, ItemInstanceId item) override
    {
        InventoryMutationResult r; r.player=player; r.item=item;
        if(failAdd_){ r.code=InventoryMutationCode::CapacityExceeded; return r; }
        if(Has(item)){ r.code=InventoryMutationCode::AlreadySatisfied; return r; }
        items_.push_back(item); r.code=InventoryMutationCode::Applied; return r;
    }
    InventoryMutationResult Remove(PlayerId player, ItemInstanceId item) override
    {
        InventoryMutationResult r; r.player=player; r.item=item;
        if(failRemove_){ r.code=InventoryMutationCode::LegacyRejected; return r; }
        std::vector<ItemInstanceId>::iterator it=std::find(items_.begin(),items_.end(),item);
        if(it==items_.end()){r.code=InventoryMutationCode::ItemNotFound;return r;}
        items_.erase(it);r.code=InventoryMutationCode::Applied;return r;
    }
    InventoryTransferResult Transfer(PlayerId,PlayerId,ItemInstanceId) override { return InventoryTransferResult(); }
    InventoryGrantResult Grant(const ItemGrant&) override { return InventoryGrantResult(); }
    bool Has(ItemInstanceId item) const { return std::find(items_.begin(),items_.end(),item)!=items_.end(); }
    void Put(ItemInstanceId item){items_.push_back(item);} void SetFailAdd(bool v){failAdd_=v;} void SetFailRemove(bool v){failRemove_=v;}
private:
    PlayerId player_; std::vector<ItemInstanceId> items_; bool failAdd_; bool failRemove_;
};

int main()
{
    const PlayerId player(7); const ItemInstanceId original(ItemId(100),1,11); const ItemInstanceId reagent(ItemId(200),2,22); const ItemInstanceId output(ItemId(300),3,33);
    FakeInventory inv(player); inv.Put(original); inv.Put(reagent);
    InventorySnapshotRollback tx(inv,inv,player); assert(tx.Captured());
    assert(inv.Remove(player,original).Applied()); assert(inv.Remove(player,reagent).Applied()); assert(inv.Add(player,output).Applied());
    const InventorySnapshotRollbackResult restored=tx.Restore(); assert(restored.Restored()); assert(restored.removed==1); assert(restored.added==2); assert(inv.Has(original)); assert(inv.Has(reagent)); assert(!inv.Has(output));

    FakeInventory brokenAdd(player); brokenAdd.Put(original); InventorySnapshotRollback failedAddTx(brokenAdd,brokenAdd,player); assert(brokenAdd.Remove(player,original).Applied()); brokenAdd.SetFailAdd(true); const InventorySnapshotRollbackResult failedAdd=failedAddTx.Restore(); assert(failedAdd.code==InventorySnapshotRollbackCode::AddFailed);

    FakeInventory brokenRemove(player); brokenRemove.Put(original); InventorySnapshotRollback failedRemoveTx(brokenRemove,brokenRemove,player); assert(brokenRemove.Add(player,output).Applied()); brokenRemove.SetFailRemove(true); const InventorySnapshotRollbackResult failedRemove=failedRemoveTx.Restore(); assert(failedRemove.code==InventorySnapshotRollbackCode::RemoveFailed);
    std::cout << "inventory snapshot rollback smoke: PASS\n";
    return 0;
}
