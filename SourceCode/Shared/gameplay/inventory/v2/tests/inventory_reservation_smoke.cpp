#include <cassert>
#include <vector>

#include "gameplay/inventory/v2/InventoryReservationService.h"

int main()
{
    using namespace gameplay;
    inventory::InventoryReservationService service;
    const PlayerId player(7u);
    const inventory::ItemInstanceId sword(ItemId(100u), 11u, 22u);
    const inventory::ItemInstanceId shield(ItemId(101u), 33u, 44u);

    inventory::InventoryReservationRequest first;
    first.owner = player;
    first.items.push_back(sword);
    first.items.push_back(shield);
    const inventory::InventoryReservationResult reserved = service.Reserve(first);
    assert(reserved.Reserved());
    assert(reserved.token.Valid());
    assert(service.IsReserved(player, sword));
    assert(service.IsReserved(player, shield));

    inventory::InventoryReservationRequest conflict;
    conflict.owner = player;
    conflict.items.push_back(sword);
    const inventory::InventoryReservationResult blocked = service.Reserve(conflict);
    assert(blocked.code == inventory::InventoryReservationCode::Conflict);
    assert(blocked.conflictingItem == sword);

    assert(service.Release(reserved.token));
    assert(!service.IsReserved(player, sword));
    const inventory::InventoryReservationResult second = service.Reserve(conflict);
    assert(second.Reserved());
    assert(service.ReleasePlayer(player) == 1u);
    assert(!service.IsReserved(player, sword));

    inventory::InventoryReservationRequest duplicate;
    duplicate.owner = player;
    duplicate.items.push_back(sword);
    duplicate.items.push_back(sword);
    assert(service.Reserve(duplicate).code == inventory::InventoryReservationCode::InvalidRequest);
    return 0;
}
