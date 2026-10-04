#pragma once

#include <vector>

#include "gameplay/inventory/v2/InventoryReservation.h"
#include "systems/inventory/v2/legacy/LegacyInventoryReservationRuntime.h"

namespace gameplay
{
namespace commerce
{
namespace legacy
{
class LegacyCommerceReservationGuard
{
public:
    LegacyCommerceReservationGuard(PlayerId owner,
                                   const std::vector<inventory::ItemInstanceId>& items)
        : service_(inventory::legacy::ServerInventoryReservationService()), token_(), accepted_(items.empty())
    {
        if(items.empty())
            return;

        inventory::InventoryReservationRequest request;
        request.owner = owner;
        request.items = items;
        const inventory::InventoryReservationResult result = service_.Reserve(request);
        token_ = result.token;
        accepted_ = result.Reserved();
    }

    ~LegacyCommerceReservationGuard()
    {
        if(token_.Valid())
            service_.Release(token_);
    }

    bool Accepted() const { return accepted_; }

private:
    LegacyCommerceReservationGuard(const LegacyCommerceReservationGuard&);
    LegacyCommerceReservationGuard& operator=(const LegacyCommerceReservationGuard&);

    inventory::IInventoryReservationService& service_;
    inventory::InventoryReservationToken token_;
    bool accepted_;
};

inline void AppendLegacyReservationItem(std::vector<inventory::ItemInstanceId>& items,
                                        std::uint32_t code,
                                        std::uint32_t key,
                                        std::uint32_t checksum)
{
    if(!code)
        return;
    items.push_back(inventory::ItemInstanceId(ItemId(code), key, checksum));
}
}
}
}
