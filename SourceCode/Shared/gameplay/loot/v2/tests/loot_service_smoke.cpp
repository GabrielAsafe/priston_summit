#include "gameplay/loot/v2/LootService.h"

#include <cassert>
#include <iostream>

namespace
{
class FakeLootService final : public gameplay::loot::ILootService
{
public:
    gameplay::loot::LootPickupResult Pickup(
        gameplay::PlayerId player,
        gameplay::loot::LootDropHandle drop) override
    {
        gameplay::loot::LootPickupResult result;
        result.player = player;
        result.drop = drop;
        if(!player.Valid())
            result.code = gameplay::loot::LootPickupCode::PlayerNotFound;
        else if(!drop.Valid())
            result.code = gameplay::loot::LootPickupCode::InvalidDrop;
        else
            result.code = gameplay::loot::LootPickupCode::PickedUp;
        return result;
    }
};
}

int main()
{
    using namespace gameplay;
    using namespace gameplay::loot;
    FakeLootService service;
    const LootDropHandle drop(2u, 7u, 1234u, ItemId(0x01020304u), 99u);
    assert(drop.Valid());
    assert(service.Pickup(PlayerId(44u), drop).PickedUp());
    assert(service.Pickup(PlayerId(), drop).code == LootPickupCode::PlayerNotFound);
    assert(service.Pickup(PlayerId(44u), LootDropHandle()).code == LootPickupCode::InvalidDrop);
    std::cout << "loot service smoke: PASS\n";
    return 0;
}
