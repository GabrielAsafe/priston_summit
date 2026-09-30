#pragma once

#include <cstdint>
#include <vector>

#include "gameplay/core/v2/GameplayIds.h"

namespace gameplay
{
namespace inventory
{
struct ItemInstanceId
{
    ItemId item;
    std::uint32_t key;
    std::uint32_t checksum;

    constexpr ItemInstanceId(ItemId itemValue = ItemId(),
                             std::uint32_t keyValue = 0u,
                             std::uint32_t checksumValue = 0u)
        : item(itemValue), key(keyValue), checksum(checksumValue) {}

    constexpr bool Valid() const { return item.Valid(); }
};

inline constexpr bool operator==(const ItemInstanceId& lhs, const ItemInstanceId& rhs)
{
    return lhs.item.value == rhs.item.value && lhs.key == rhs.key && lhs.checksum == rhs.checksum;
}

inline constexpr bool operator!=(const ItemInstanceId& lhs, const ItemInstanceId& rhs)
{
    return !(lhs == rhs);
}

struct InventorySlot
{
    std::uint32_t index;
    ItemInstanceId item;

    InventorySlot(std::uint32_t indexValue = 0u, ItemInstanceId itemValue = ItemInstanceId())
        : index(indexValue), item(itemValue) {}
};

struct InventorySnapshot
{
    PlayerId player;
    std::vector<InventorySlot> slots;

    InventorySnapshot() : player(), slots() {}
    explicit InventorySnapshot(PlayerId playerValue) : player(playerValue), slots() {}
};

enum class InventoryPresenceCode
{
    Present,
    Absent,
    PlayerNotFound,
    InvalidItem
};

struct InventoryPresenceResult
{
    InventoryPresenceCode code;
    PlayerId player;
    ItemInstanceId item;

    InventoryPresenceResult()
        : code(InventoryPresenceCode::InvalidItem), player(), item() {}

    bool Present() const { return code == InventoryPresenceCode::Present; }
};

enum class InventoryMutationCode
{
    Applied,
    AlreadySatisfied,
    PlayerNotFound,
    InvalidItem,
    ItemNotFound,
    CapacityExceeded,
    LegacyRejected
};

struct InventoryMutationResult
{
    InventoryMutationCode code;
    PlayerId player;
    ItemInstanceId item;

    InventoryMutationResult()
        : code(InventoryMutationCode::LegacyRejected), player(), item() {}

    bool Applied() const
    {
        return code == InventoryMutationCode::Applied ||
               code == InventoryMutationCode::AlreadySatisfied;
    }
};


enum class InventoryTransferCode
{
    Applied,
    SamePlayer,
    InvalidItem,
    SourcePlayerNotFound,
    DestinationPlayerNotFound,
    SourceItemNotFound,
    DestinationAlreadyContains,
    DestinationCapacityExceeded,
    SourceRemoveRejected,
    RollbackFailed
};

struct InventoryTransferResult
{
    InventoryTransferCode code;
    PlayerId source;
    PlayerId destination;
    ItemInstanceId item;

    InventoryTransferResult()
        : code(InventoryTransferCode::SourceRemoveRejected), source(), destination(), item() {}

    bool Applied() const { return code == InventoryTransferCode::Applied; }
};

struct ItemGrant
{
    PlayerId player;
    ItemId item;
    std::uint32_t quantity;

    ItemGrant(PlayerId playerValue = PlayerId(),
              ItemId itemValue = ItemId(),
              std::uint32_t quantityValue = 1u)
        : player(playerValue), item(itemValue), quantity(quantityValue) {}
};

enum class InventoryGrantCode
{
    Granted,
    PlayerNotFound,
    InvalidItem,
    InvalidQuantity,
    LegacyRejected
};

struct InventoryGrantResult
{
    InventoryGrantCode code;
    ItemGrant grant;

    InventoryGrantResult()
        : code(InventoryGrantCode::LegacyRejected), grant() {}

    bool Granted() const { return code == InventoryGrantCode::Granted; }
};

class IInventoryQuery
{
public:
    virtual ~IInventoryQuery() {}
    virtual InventoryPresenceResult Contains(PlayerId player, ItemInstanceId item) const = 0;
    virtual bool TryGetSnapshot(PlayerId player, InventorySnapshot& out) const = 0;
};

class IInventoryCommands
{
public:
    virtual ~IInventoryCommands() {}
    virtual InventoryMutationResult Add(PlayerId player, ItemInstanceId item) = 0;
    virtual InventoryMutationResult Remove(PlayerId player, ItemInstanceId item) = 0;
    virtual InventoryTransferResult Transfer(PlayerId source, PlayerId destination, ItemInstanceId item) = 0;
    virtual InventoryGrantResult Grant(const ItemGrant& grant) = 0;
};
}
}
