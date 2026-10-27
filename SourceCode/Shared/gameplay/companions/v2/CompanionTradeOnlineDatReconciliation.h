#pragma once
// SUMMIT325: read-only reconciliation of an online identity ledger against a
// persisted DAT's full encoded item records. This NEVER authenticates live
// item payloads, locks a session, or authorizes a trade.
#include <cstddef>
#include <cstdint>
#include <set>
#include <tuple>
#include <vector>

#include "gameplay/companions/v2/CompanionTradeOnlineIdentitySnapshot.h"
#include "gameplay/companions/v2/CompanionTradeInventorySnapshot.h"

namespace gameplay { namespace companions { namespace v2 {

enum class OnlineTradePersistedCode {
    SessionUnavailable, OnlineUnavailable, PersistedUnavailable,
    SessionChanged, PersistedUnsafe, IdentityMismatch, MatchReadOnly
};

struct OnlineTradePersistedCheck {
    OnlineTradePersistedCode code;
    std::uint64_t onlineRevision;
    std::uint32_t itemsCompared;
    OnlineTradePersistedCheck()
        : code(OnlineTradePersistedCode::OnlineUnavailable),
          onlineRevision(0u), itemsCompared(0u) {}
    // A saved DAT is not proof of live item attributes. A later save can race
    // this read, even when the identities match at the time of comparison.
    bool HasAuthenticatedLivePayloads() const { return false; }
    bool CanCommitTransfer() const { return false; }
};

struct OnlineTradePersistedPolicy {
    using Identity = std::tuple<std::uint32_t, std::uint32_t, std::uint32_t>;
    static OnlineTradePersistedCheck Compare(
        const OnlineTradeIdentitySnapshot& online,
        const CompanionTradeInventorySnapshot& persisted,
        std::uint32_t onlineCapacity)
    {
        OnlineTradePersistedCheck result;
        if (!online.session.Valid() || online.revision == 0u ||
            onlineCapacity == 0u)
            return result;
        OnlineTradeIdentitySnapshot verified = online;
        if (!OnlineTradeIdentityPolicy::Finalize(verified, onlineCapacity) ||
            !OnlineTradeIdentityPolicy::ExactlyMatches(online, verified))
            return result;
        result.onlineRevision = online.revision;
        if (!persisted.TradeEligible() ||
            persisted.primaryCount + static_cast<std::uint64_t>(persisted.secondaryCount)
                != persisted.records.size()) {
            result.code = OnlineTradePersistedCode::PersistedUnsafe;
            return result;
        }
        // Some legacy DAT records have no instance identity, and are never
        // represented in the ONLINE full-instance ledger. Preserve them as
        // non-transferable records; compare all fully identified instances.
        std::set<std::uint32_t> slots;
        std::set<Identity> live, disk;
        for (const auto& entry : online.entries) {
            const auto& item = entry.identity;
            if (!item.Valid() || !item.key || !item.checksum ||
                !slots.insert(entry.slot).second ||
                !live.insert(Identity(item.item.value, item.key, item.checksum)).second) {
                result.code = OnlineTradePersistedCode::IdentityMismatch;
                return result;
            }
        }
        for (std::size_t i = 0; i < persisted.records.size(); ++i) {
            const CompanionTradeItemRecord& record = persisted.records[i];
            if (!record.itemCode ||
                record.recordIndex != i ||
                record.inSecondaryBag != (i >= persisted.primaryCount) ||
                record.encodedRecord.size() <= sizeof(std::int32_t) ||
                record.position < 0 || record.position > 16) {
                result.code = OnlineTradePersistedCode::PersistedUnsafe;
                return result;
            }
            if (HasCompleteTradeIdentity350O(record.itemCode,record.itemKey,
                                             record.itemChecksum) &&
                !disk.insert(Identity(record.itemCode,record.itemKey,
                                      record.itemChecksum)).second) {
                result.code = OnlineTradePersistedCode::PersistedUnsafe;
                return result;
            }
        }
        result.code = live == disk ? OnlineTradePersistedCode::MatchReadOnly
                                   : OnlineTradePersistedCode::IdentityMismatch;
        if (result.code == OnlineTradePersistedCode::MatchReadOnly)
            result.itemsCompared = static_cast<std::uint32_t>(live.size());
        return result;
    }
};

}}} // namespace gameplay::companions::v2
