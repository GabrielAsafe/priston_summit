#include "gameplay/inventory/v2/Inventory.h"

#include <cassert>
#include <iostream>
#include <map>

namespace
{
class FakeInventory final : public gameplay::inventory::IInventoryQuery,
                            public gameplay::inventory::IInventoryCommands
{
public:
    gameplay::inventory::InventoryPresenceResult Contains(
        gameplay::PlayerId player,
        gameplay::inventory::ItemInstanceId item) const override
    {
        gameplay::inventory::InventoryPresenceResult result;
        result.player = player;
        result.item = item;
        if(!player.Valid())
            result.code = gameplay::inventory::InventoryPresenceCode::PlayerNotFound;
        else if(!item.Valid())
            result.code = gameplay::inventory::InventoryPresenceCode::InvalidItem;
        else
            result.code = items.count(Key(player, item))
                              ? gameplay::inventory::InventoryPresenceCode::Present
                              : gameplay::inventory::InventoryPresenceCode::Absent;
        return result;
    }

    bool TryGetSnapshot(gameplay::PlayerId player, gameplay::inventory::InventorySnapshot& out) const override
    {
        if(!player.Valid()) return false;
        out = gameplay::inventory::InventorySnapshot(player);
        for(const auto& pair : items)
            if(static_cast<std::uint32_t>(pair.first >> 32u) == player.value)
                out.slots.push_back(gameplay::inventory::InventorySlot(
                    static_cast<std::uint32_t>(out.slots.size()), pair.second));
        return true;
    }

    gameplay::inventory::InventoryMutationResult Add(
        gameplay::PlayerId player,
        gameplay::inventory::ItemInstanceId item) override
    {
        gameplay::inventory::InventoryMutationResult result;
        result.player = player;
        result.item = item;
        if(!player.Valid())
            result.code = gameplay::inventory::InventoryMutationCode::PlayerNotFound;
        else if(!item.Valid())
            result.code = gameplay::inventory::InventoryMutationCode::InvalidItem;
        else if(items.count(Key(player, item)))
            result.code = gameplay::inventory::InventoryMutationCode::AlreadySatisfied;
        else
        {
            items[Key(player, item)] = item;
            result.code = gameplay::inventory::InventoryMutationCode::Applied;
        }
        return result;
    }

    gameplay::inventory::InventoryMutationResult Remove(
        gameplay::PlayerId player,
        gameplay::inventory::ItemInstanceId item) override
    {
        gameplay::inventory::InventoryMutationResult result;
        result.player = player;
        result.item = item;
        const auto key = Key(player, item);
        if(items.erase(key))
            result.code = gameplay::inventory::InventoryMutationCode::Applied;
        else
            result.code = gameplay::inventory::InventoryMutationCode::ItemNotFound;
        return result;
    }

    gameplay::inventory::InventoryTransferResult Transfer(
        gameplay::PlayerId source,
        gameplay::PlayerId destination,
        gameplay::inventory::ItemInstanceId item) override
    {
        gameplay::inventory::InventoryTransferResult result;
        result.source = source;
        result.destination = destination;
        result.item = item;
        if(source == destination)
            result.code = gameplay::inventory::InventoryTransferCode::SamePlayer;
        else if(!item.Valid())
            result.code = gameplay::inventory::InventoryTransferCode::InvalidItem;
        else if(!items.count(Key(source, item)))
            result.code = gameplay::inventory::InventoryTransferCode::SourceItemNotFound;
        else if(items.count(Key(destination, item)))
            result.code = gameplay::inventory::InventoryTransferCode::DestinationAlreadyContains;
        else
        {
            items[Key(destination, item)] = item;
            if(!items.erase(Key(source, item)))
            {
                items.erase(Key(destination, item));
                result.code = gameplay::inventory::InventoryTransferCode::SourceRemoveRejected;
            }
            else
                result.code = gameplay::inventory::InventoryTransferCode::Applied;
        }
        return result;
    }

    gameplay::inventory::InventoryGrantResult Grant(
        const gameplay::inventory::ItemGrant& grant) override
    {
        gameplay::inventory::InventoryGrantResult result;
        result.grant = grant;
        if(!grant.player.Valid())
            result.code = gameplay::inventory::InventoryGrantCode::PlayerNotFound;
        else if(!grant.item.Valid())
            result.code = gameplay::inventory::InventoryGrantCode::InvalidItem;
        else if(grant.quantity != 1u)
            result.code = gameplay::inventory::InventoryGrantCode::InvalidQuantity;
        else
            result.code = gameplay::inventory::InventoryGrantCode::Granted;
        return result;
    }

private:
    static std::uint64_t Key(gameplay::PlayerId player, gameplay::inventory::ItemInstanceId item)
    {
        return (static_cast<std::uint64_t>(player.value) << 32u) ^
               static_cast<std::uint64_t>(item.item.value ^ item.key ^ item.checksum);
    }

    std::map<std::uint64_t, gameplay::inventory::ItemInstanceId> items;
};
}

int main()
{
    using namespace gameplay;
    using namespace gameplay::inventory;

    FakeInventory inventory;
    const PlayerId player(7u);
    const ItemInstanceId item(ItemId(0x12345678u), 11u, 22u);

    assert(!inventory.Contains(player, item).Present());
    assert(inventory.Add(player, item).code == InventoryMutationCode::Applied);
    assert(inventory.Contains(player, item).Present());
    assert(inventory.Add(player, item).code == InventoryMutationCode::AlreadySatisfied);
    assert(inventory.Remove(player, item).code == InventoryMutationCode::Applied);
    assert(!inventory.Contains(player, item).Present());
    assert(inventory.Remove(player, item).code == InventoryMutationCode::ItemNotFound);

    const PlayerId seller(8u);
    const PlayerId buyer(9u);
    assert(inventory.Add(seller, item).Applied());
    assert(inventory.Transfer(seller, buyer, item).code == InventoryTransferCode::Applied);
    assert(!inventory.Contains(seller, item).Present());
    assert(inventory.Contains(buyer, item).Present());
    assert(inventory.Transfer(seller, buyer, item).code == InventoryTransferCode::SourceItemNotFound);
    assert(inventory.Transfer(buyer, buyer, item).code == InventoryTransferCode::SamePlayer);

    InventorySnapshot snapshot;
    assert(inventory.TryGetSnapshot(buyer, snapshot));
    assert(snapshot.player == buyer);
    assert(!snapshot.slots.empty());

    assert(inventory.Grant(ItemGrant(buyer, ItemId(0xABCDEF01u), 1u)).Granted());
    assert(inventory.Grant(ItemGrant(buyer, ItemId(0xABCDEF01u), 2u)).code == InventoryGrantCode::InvalidQuantity);
    assert(inventory.Grant(ItemGrant(buyer, ItemId(), 1u)).code == InventoryGrantCode::InvalidItem);

    std::cout << "inventory contract smoke: PASS\n";
    return 0;
}
