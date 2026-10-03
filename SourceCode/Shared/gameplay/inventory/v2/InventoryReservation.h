#pragma once

#include <cstdint>
#include <vector>

#include "gameplay/inventory/v2/Inventory.h"

namespace gameplay
{
namespace inventory
{
struct InventoryReservationToken
{
    std::uint64_t value;
    explicit constexpr InventoryReservationToken(std::uint64_t v = 0u) : value(v) {}
    constexpr bool Valid() const { return value != 0u; }
};

inline constexpr bool operator==(InventoryReservationToken a, InventoryReservationToken b)
{
    return a.value == b.value;
}

enum class InventoryReservationCode
{
    Reserved,
    InvalidRequest,
    Conflict
};

struct InventoryReservationRequest
{
    PlayerId owner;
    std::vector<ItemInstanceId> items;
};

struct InventoryReservationResult
{
    InventoryReservationCode code;
    InventoryReservationToken token;
    ItemInstanceId conflictingItem;

    InventoryReservationResult()
        : code(InventoryReservationCode::InvalidRequest), token(), conflictingItem() {}
    bool Reserved() const { return code == InventoryReservationCode::Reserved; }
};

class IInventoryReservationService
{
public:
    virtual ~IInventoryReservationService() {}
    virtual InventoryReservationResult Reserve(const InventoryReservationRequest& request) = 0;
    virtual bool Release(InventoryReservationToken token) = 0;
    virtual std::size_t ReleasePlayer(PlayerId player) = 0;
    virtual bool IsReserved(PlayerId player, ItemInstanceId item) const = 0;
};
}
}
