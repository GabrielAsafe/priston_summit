#pragma once

#include <cstdint>

#include "gameplay/core/v2/GameplayIds.h"

namespace gameplay
{
namespace loot
{
struct LootDropHandle
{
    std::uint32_t areaIndex;
    std::uint32_t slotIndex;
    std::uint32_t createdAt;
    ItemId item;
    std::uint32_t checksum;

    LootDropHandle()
        : areaIndex(0xFFFFFFFFu), slotIndex(0xFFFFFFFFu), createdAt(0u), item(), checksum(0u) {}

    LootDropHandle(std::uint32_t areaIndexValue,
                   std::uint32_t slotIndexValue,
                   std::uint32_t createdAtValue,
                   ItemId itemValue,
                   std::uint32_t checksumValue)
        : areaIndex(areaIndexValue), slotIndex(slotIndexValue), createdAt(createdAtValue),
          item(itemValue), checksum(checksumValue) {}

    bool Valid() const
    {
        return areaIndex != 0xFFFFFFFFu && slotIndex != 0xFFFFFFFFu && item.Valid();
    }
};

enum class LootPickupCode
{
    PickedUp,
    PlayerNotFound,
    InvalidDrop,
    DropNotFound,
    StaleDrop,
    LegacyRejected
};

struct LootPickupResult
{
    LootPickupCode code;
    PlayerId player;
    LootDropHandle drop;

    LootPickupResult() : code(LootPickupCode::LegacyRejected), player(), drop() {}
    bool PickedUp() const { return code == LootPickupCode::PickedUp; }
};

class ILootService
{
public:
    virtual ~ILootService() {}
    virtual LootPickupResult Pickup(PlayerId player, LootDropHandle drop) = 0;
};
}
}
