#pragma once

#include <algorithm>

#include "gameplay/inventory/v2/Inventory.h"

namespace gameplay
{
namespace inventory
{
enum class InventorySnapshotRollbackCode
{
    Restored,
    SnapshotUnavailable,
    CurrentSnapshotUnavailable,
    RemoveFailed,
    AddFailed
};

struct InventorySnapshotRollbackResult
{
    InventorySnapshotRollbackCode code;
    ItemInstanceId failingItem;
    std::size_t removed;
    std::size_t added;

    InventorySnapshotRollbackResult()
        : code(InventorySnapshotRollbackCode::SnapshotUnavailable), failingItem(), removed(0u), added(0u) {}

    bool Restored() const { return code == InventorySnapshotRollbackCode::Restored; }
};

class InventorySnapshotRollback
{
public:
    InventorySnapshotRollback(const IInventoryQuery& query,
                              IInventoryCommands& commands,
                              PlayerId player)
        : query_(query), commands_(commands), player_(player), before_(player), captured_(false)
    {
        captured_ = player_.Valid() && query_.TryGetSnapshot(player_, before_);
    }

    bool Captured() const { return captured_; }
    const InventorySnapshot& Before() const { return before_; }

    InventorySnapshotRollbackResult Restore()
    {
        InventorySnapshotRollbackResult result;
        if(!captured_)
            return result;

        InventorySnapshot current(player_);
        if(!query_.TryGetSnapshot(player_, current))
        {
            result.code = InventorySnapshotRollbackCode::CurrentSnapshotUnavailable;
            return result;
        }

        // Remove post-transaction instances first so replacement items do not
        // consume capacity needed to restore the original instances.
        for(std::vector<InventorySlot>::const_iterator it = current.slots.begin(); it != current.slots.end(); ++it)
        {
            if(Contains(before_, it->item))
                continue;
            const InventoryMutationResult removed = commands_.Remove(player_, it->item);
            if(!removed.Applied())
            {
                result.code = InventorySnapshotRollbackCode::RemoveFailed;
                result.failingItem = it->item;
                return result;
            }
            ++result.removed;
        }

        InventorySnapshot afterRemoval(player_);
        if(!query_.TryGetSnapshot(player_, afterRemoval))
        {
            result.code = InventorySnapshotRollbackCode::CurrentSnapshotUnavailable;
            return result;
        }

        for(std::vector<InventorySlot>::const_iterator it = before_.slots.begin(); it != before_.slots.end(); ++it)
        {
            if(Contains(afterRemoval, it->item))
                continue;
            const InventoryMutationResult added = commands_.Add(player_, it->item);
            if(!added.Applied())
            {
                result.code = InventorySnapshotRollbackCode::AddFailed;
                result.failingItem = it->item;
                return result;
            }
            ++result.added;
        }

        result.code = InventorySnapshotRollbackCode::Restored;
        return result;
    }

private:
    static bool Contains(const InventorySnapshot& snapshot, ItemInstanceId item)
    {
        for(std::vector<InventorySlot>::const_iterator it = snapshot.slots.begin(); it != snapshot.slots.end(); ++it)
            if(it->item == item)
                return true;
        return false;
    }

    const IInventoryQuery& query_;
    IInventoryCommands& commands_;
    PlayerId player_;
    InventorySnapshot before_;
    bool captured_;
};
}
}
