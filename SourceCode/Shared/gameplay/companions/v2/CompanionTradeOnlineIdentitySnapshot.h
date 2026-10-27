#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "gameplay/core/v2/GameplayIds.h"
#include "gameplay/inventory/v2/Inventory.h"

namespace gameplay { namespace companions {

// Online server inventory is an anti-duplication identity ledger, NOT an
// authoritative source of full item payloads (upgrades, affixes, etc.).
// This type deliberately cannot authorize a physical transfer by itself.
struct OnlineTradeIdentityEntry
{
    std::uint32_t slot;
    inventory::ItemInstanceId identity;
    OnlineTradeIdentityEntry(std::uint32_t at = 0u,
                             inventory::ItemInstanceId item = inventory::ItemInstanceId())
        : slot(at), identity(item) {}
};

struct OnlineTradeIdentitySnapshot
{
    PlayerHandle session;
    std::vector<OnlineTradeIdentityEntry> entries;
    std::uint64_t revision;
    OnlineTradeIdentitySnapshot() : session(), entries(), revision(0u) {}
    // Explicit fail-closed contract: the legacy ONLINE ledger has no raw item records.
    bool HasFullItemPayloads() const { return false; }
    bool CanCommitItemTransfer() const { return false; }
};

struct OnlineTradeIdentityPolicy
{
    static void Mix(std::uint64_t& hash, std::uint32_t value)
    {
        // Byte-order-independent FNV-1a over four little-endian bytes.
        for (unsigned shift = 0; shift < 32; shift += 8)
        {
            hash ^= static_cast<std::uint8_t>(value >> shift);
            hash *= UINT64_C(1099511628211);
        }
    }

    static bool Finalize(OnlineTradeIdentitySnapshot& snapshot, std::uint32_t capacity)
    {
        if (!snapshot.session.Valid() || capacity == 0u ||
            snapshot.entries.size() > capacity)
            return false;
        std::uint64_t hash = UINT64_C(14695981039346656037);
        Mix(hash, snapshot.session.id.value);
        Mix(hash, snapshot.session.generation);
        Mix(hash, capacity);
        Mix(hash, static_cast<std::uint32_t>(snapshot.entries.size()));
        std::vector<bool> occupied(capacity, false);
        for (std::size_t i = 0; i < snapshot.entries.size(); ++i)
        {
            const OnlineTradeIdentityEntry& entry = snapshot.entries[i];
            if (entry.slot >= capacity || occupied[entry.slot] ||
                !entry.identity.Valid() || !entry.identity.key || !entry.identity.checksum)
                return false;
            occupied[entry.slot] = true;
            for (std::size_t j = 0; j < i; ++j)
            {
                if (entry.identity == snapshot.entries[j].identity)
                    return false;
            }
            Mix(hash, entry.slot);
            Mix(hash, entry.identity.item.value);
            Mix(hash, entry.identity.key);
            Mix(hash, entry.identity.checksum);
        }
        snapshot.revision = hash;
        return true;
    }

    static bool ExactlyMatches(const OnlineTradeIdentitySnapshot& expected,
                               const OnlineTradeIdentitySnapshot& actual)
    {
        if (expected.session.id != actual.session.id ||
            expected.session.generation != actual.session.generation ||
            expected.revision != actual.revision ||
            expected.entries.size() != actual.entries.size())
            return false;
        for (std::size_t i = 0; i < expected.entries.size(); ++i)
        {
            if (expected.entries[i].slot != actual.entries[i].slot ||
                expected.entries[i].identity != actual.entries[i].identity)
                return false;
        }
        return true;
    }
};

} }
