#pragma once

#include <algorithm>
#include <limits>
#include <vector>

#include "gameplay/inventory/v2/InventoryReservation.h"

namespace gameplay
{
namespace inventory
{
class InventoryReservationService final : public IInventoryReservationService
{
public:
    InventoryReservationService() : nextToken_(1u) {}

    InventoryReservationResult Reserve(const InventoryReservationRequest& request) override
    {
        InventoryReservationResult result;
        if(!request.owner.Valid() || request.items.empty())
            return result;

        std::vector<ItemInstanceId> normalized;
        normalized.reserve(request.items.size());
        for(std::vector<ItemInstanceId>::const_iterator it = request.items.begin(); it != request.items.end(); ++it)
        {
            if(!it->Valid())
                return result;
            if(std::find(normalized.begin(), normalized.end(), *it) != normalized.end())
                return result;
            if(IsReserved(request.owner, *it))
            {
                result.code = InventoryReservationCode::Conflict;
                result.conflictingItem = *it;
                return result;
            }
            normalized.push_back(*it);
        }

        if(nextToken_ == 0u)
            nextToken_ = 1u;
        const InventoryReservationToken token(nextToken_++);
        Entry entry;
        entry.token = token;
        entry.owner = request.owner;
        entry.items.swap(normalized);
        entries_.push_back(entry);
        result.code = InventoryReservationCode::Reserved;
        result.token = token;
        return result;
    }

    bool Release(InventoryReservationToken token) override
    {
        if(!token.Valid())
            return false;
        for(std::vector<Entry>::iterator it = entries_.begin(); it != entries_.end(); ++it)
        {
            if(it->token == token)
            {
                entries_.erase(it);
                return true;
            }
        }
        return false;
    }

    std::size_t ReleasePlayer(PlayerId player) override
    {
        const std::size_t before = entries_.size();
        entries_.erase(std::remove_if(entries_.begin(), entries_.end(),
            [player](const Entry& entry) { return entry.owner == player; }), entries_.end());
        return before - entries_.size();
    }

    bool IsReserved(PlayerId player, ItemInstanceId item) const override
    {
        if(!player.Valid() || !item.Valid())
            return false;
        for(std::vector<Entry>::const_iterator entry = entries_.begin(); entry != entries_.end(); ++entry)
        {
            if(entry->owner != player)
                continue;
            if(std::find(entry->items.begin(), entry->items.end(), item) != entry->items.end())
                return true;
        }
        return false;
    }

private:
    struct Entry
    {
        InventoryReservationToken token;
        PlayerId owner;
        std::vector<ItemInstanceId> items;
    };

    std::uint64_t nextToken_;
    std::vector<Entry> entries_;
};
}
}
