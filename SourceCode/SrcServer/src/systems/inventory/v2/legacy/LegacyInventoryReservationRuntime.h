#pragma once

#include "gameplay/inventory/v2/InventoryReservationService.h"

namespace gameplay
{
namespace inventory
{
namespace legacy
{
// Server-wide reservation kernel shared by commerce adapters. Keeping one
// instance is essential: a Trade reservation must conflict with a Personal
// Shop reservation for the same legacy item instance, and vice versa.
inline IInventoryReservationService& ServerInventoryReservationService()
{
    static InventoryReservationService service;
    return service;
}
}
}
}
