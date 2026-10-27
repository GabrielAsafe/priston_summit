#include "systems/inventory/v2/legacy/LegacyInventoryRuntime.h"
#include "systems/gameplay/player/v2/legacy/LegacyCharacterRecordAdapter.h"
#include "gameplay/companions/v2/CompanionTradeSaveActivityFence.h"

#include "systems/network/transport/legacy/smwsock.h"
#include "systems/rendering/legacy_3d/smd3d.h"
#include "systems/gameplay/character/core/character.h"
#include "core/server/legacy_runtime/onserver.h"

int rsFindInvenItem(rsPLAYINFO* lpPlayInfo, DWORD dwCode, DWORD dwHead, DWORD dwChkSum);
int rsAddInvenItem(rsPLAYINFO* lpPlayInfo, DWORD dwCode, DWORD dwHead, DWORD dwChkSum);
int rsDeleteInvenItem(rsPLAYINFO* lpPlayInfo, DWORD dwCode, DWORD dwHead, DWORD dwChkSum);

namespace gameplay
{
namespace inventory
{
namespace legacy
{
namespace
{
InventoryPresenceResult BasePresence(PlayerId player, ItemInstanceId item)
{
    InventoryPresenceResult result;
    result.player = player;
    result.item = item;
    return result;
}

InventoryMutationResult BaseMutation(PlayerId player, ItemInstanceId item)
{
    InventoryMutationResult result;
    result.player = player;
    result.item = item;
    return result;
}
}

LegacyInventoryRuntime::LegacyInventoryRuntime(
    const player::legacy::LegacyOnlinePlayerRegistry& registry)
    : registry_(registry)
{
}

InventoryPresenceResult LegacyInventoryRuntime::Contains(PlayerId player, ItemInstanceId item) const
{
    InventoryPresenceResult result = BasePresence(player, item);
    if(!item.Valid())
    {
        result.code = InventoryPresenceCode::InvalidItem;
        return result;
    }

    rsPLAYINFO* legacyPlayer = registry_.Resolve(player);
    if(!legacyPlayer)
    {
        result.code = InventoryPresenceCode::PlayerNotFound;
        return result;
    }

    const int index = rsFindInvenItem(
        legacyPlayer,
        static_cast<DWORD>(item.item.value),
        static_cast<DWORD>(item.key),
        static_cast<DWORD>(item.checksum));
    result.code = index >= 0 ? InventoryPresenceCode::Present : InventoryPresenceCode::Absent;
    return result;
}

bool LegacyInventoryRuntime::TryGetSnapshot(PlayerId player, InventorySnapshot& out) const
{
    rsPLAYINFO* legacyPlayer = registry_.Resolve(player);
    if(!legacyPlayer || !rsServerConfig.ServerInventoryMode)
        return false;

    InventorySnapshot snapshot(player);
    snapshot.slots.reserve(INVEN_ITEM_INFO_MAX);
    for(int index = 0; index < INVEN_ITEM_INFO_MAX; ++index)
    {
        const sTHROW_ITEM_INFO& legacyItem = legacyPlayer->InvenItemInfo[index];
        if(!legacyItem.dwCode)
            continue;
        snapshot.slots.push_back(InventorySlot(
            static_cast<std::uint32_t>(index),
            ItemInstanceId(
                ItemId(static_cast<std::uint32_t>(legacyItem.dwCode)),
                static_cast<std::uint32_t>(legacyItem.dwKey),
                static_cast<std::uint32_t>(legacyItem.dwSum))));
    }

    out = snapshot;
    return true;
}

bool LegacyInventoryRuntime::TryGetTradeIdentitySnapshot(
    PlayerHandle session, companions::OnlineTradeIdentitySnapshot& out) const
{
    if (!session.Valid() || !rsServerConfig.ServerInventoryMode)
        return false;
    rsPLAYINFO* legacyPlayer = registry_.Resolve(session);
    if (!legacyPlayer)
        return false;

    companions::OnlineTradeIdentitySnapshot candidate;
    candidate.session = session;
    candidate.entries.reserve(INVEN_ITEM_INFO_MAX);
    for (int index = 0; index < INVEN_ITEM_INFO_MAX; ++index)
    {
        const sTHROW_ITEM_INFO& item = legacyPlayer->InvenItemInfo[index];
        if (!item.dwCode)
        {
            // The legacy rsDeleteInvenItem clears only dwCode. dwKey/dwSum
            // may retain the deleted item's identity; the slot is still empty.
            // Never include such tombstones in trade pages or revisions.
            continue;
        }
        candidate.entries.push_back(companions::OnlineTradeIdentityEntry(
            static_cast<std::uint32_t>(index),
            ItemInstanceId(ItemId(static_cast<std::uint32_t>(item.dwCode)),
                           static_cast<std::uint32_t>(item.dwKey),
                           static_cast<std::uint32_t>(item.dwSum))));
    }
    if (!companions::OnlineTradeIdentityPolicy::Finalize(
            candidate, INVEN_ITEM_INFO_MAX) || !registry_.IsCurrent(session))
        return false;
    out = candidate;  // Never change the caller's snapshot on failure.
    return true;
}

bool LegacyInventoryRuntime::MatchesTradeIdentitySnapshot(
    const companions::OnlineTradeIdentitySnapshot& expected) const
{
    companions::OnlineTradeIdentitySnapshot fresh;
    return TryGetTradeIdentitySnapshot(expected.session, fresh) &&
        companions::OnlineTradeIdentityPolicy::ExactlyMatches(expected, fresh);
}

companions::v2::OnlineTradePersistedCheck
LegacyInventoryRuntime::CheckTradePersistedIdentity(PlayerHandle session) const
{
    using namespace companions::v2;
    OnlineTradePersistedCheck result;
    rsPLAYINFO* legacyPlayer = registry_.Resolve(session);
    if (!legacyPlayer) {
        result.code = OnlineTradePersistedCode::SessionUnavailable;
        return result;
    }
    const char* account = legacyPlayer->getAccount();
    const char* character = legacyPlayer->getName();
    if (!account || !account[0] || !character || !character[0]) {
        result.code = OnlineTradePersistedCode::SessionUnavailable;
        return result;
    }
    // Capture trusted, server-side identity before disk I/O. Never trust a
    // caller-provided character name or any client-record payload.
    const std::string accountKey(account), characterName(character);
    // SUMMIT327: read-only observation, not a lock or transfer permission.
    companions::v2::SaveReadOnlyEpoch saveEpoch;
    const companions::v2::SaveSessionKey saveKey(session.id.value, session.generation);
    if (!companions::v2::ServerSaveActivityFence().TryObserve(saveKey, saveEpoch)) {
        result.code = OnlineTradePersistedCode::SessionChanged;
        return result;
    }
    companions::OnlineTradeIdentitySnapshot before, after;
    if (!TryGetTradeIdentitySnapshot(session, before)) {
        result.code = OnlineTradePersistedCode::OnlineUnavailable;
        return result;
    }
    CompanionTradeInventorySnapshot disk;
    const auto readCode = player::legacy::ServerLegacyCharacterRecordAdapter()
        .TryLoadTradeInventorySnapshot(
            player::CharacterRecordKey(accountKey, characterName), disk);
    if (readCode != CompanionTradeInventoryReadCode::Ok) {
        result.code = OnlineTradePersistedCode::PersistedUnavailable;
        return result;
    }
    // Two read-only observations reduce stale results; they are NOT a lock.
    if (!companions::v2::ServerSaveActivityFence().StillStable(saveEpoch)) {
        result.code = OnlineTradePersistedCode::SessionChanged;
        return result;
    }
    if (registry_.Resolve(session) != legacyPlayer ||
        accountKey != (legacyPlayer->getAccount() ? legacyPlayer->getAccount() : "") ||
        characterName != (legacyPlayer->getName() ? legacyPlayer->getName() : "") ||
        !TryGetTradeIdentitySnapshot(session, after) ||
        !companions::OnlineTradeIdentityPolicy::ExactlyMatches(before, after)) {
        result.code = OnlineTradePersistedCode::SessionChanged;
        return result;
    }
    if (!companions::v2::ServerSaveActivityFence().StillStable(saveEpoch)) {
        result.code = OnlineTradePersistedCode::SessionChanged;
        return result;
    }
    return OnlineTradePersistedPolicy::Compare(after, disk, INVEN_ITEM_INFO_MAX);
}

InventoryMutationResult LegacyInventoryRuntime::Add(PlayerId player, ItemInstanceId item)
{
    InventoryMutationResult result = BaseMutation(player, item);
    if(!item.Valid())
    {
        result.code = InventoryMutationCode::InvalidItem;
        return result;
    }

    rsPLAYINFO* legacyPlayer = registry_.Resolve(player);
    if(!legacyPlayer)
    {
        result.code = InventoryMutationCode::PlayerNotFound;
        return result;
    }

    if(rsFindInvenItem(
           legacyPlayer,
           static_cast<DWORD>(item.item.value),
           static_cast<DWORD>(item.key),
           static_cast<DWORD>(item.checksum)) >= 0)
    {
        result.code = InventoryMutationCode::AlreadySatisfied;
        return result;
    }

    const int index = rsAddInvenItem(
        legacyPlayer,
        static_cast<DWORD>(item.item.value),
        static_cast<DWORD>(item.key),
        static_cast<DWORD>(item.checksum));
    result.code = index >= 0 ? InventoryMutationCode::Applied : InventoryMutationCode::CapacityExceeded;
    return result;
}

InventoryMutationResult LegacyInventoryRuntime::Remove(PlayerId player, ItemInstanceId item)
{
    InventoryMutationResult result = BaseMutation(player, item);
    if(!item.Valid())
    {
        result.code = InventoryMutationCode::InvalidItem;
        return result;
    }

    rsPLAYINFO* legacyPlayer = registry_.Resolve(player);
    if(!legacyPlayer)
    {
        result.code = InventoryMutationCode::PlayerNotFound;
        return result;
    }

    const int index = rsDeleteInvenItem(
        legacyPlayer,
        static_cast<DWORD>(item.item.value),
        static_cast<DWORD>(item.key),
        static_cast<DWORD>(item.checksum));
    result.code = index >= 0 ? InventoryMutationCode::Applied : InventoryMutationCode::ItemNotFound;
    return result;
}

InventoryTransferResult LegacyInventoryRuntime::Transfer(
    PlayerId source,
    PlayerId destination,
    ItemInstanceId item)
{
    InventoryTransferResult result;
    result.source = source;
    result.destination = destination;
    result.item = item;

    if(!item.Valid())
    {
        result.code = InventoryTransferCode::InvalidItem;
        return result;
    }

    if(source == destination)
    {
        result.code = InventoryTransferCode::SamePlayer;
        return result;
    }

    const InventoryPresenceResult sourcePresence = Contains(source, item);
    if(sourcePresence.code == InventoryPresenceCode::PlayerNotFound)
    {
        result.code = InventoryTransferCode::SourcePlayerNotFound;
        return result;
    }
    if(!sourcePresence.Present())
    {
        result.code = InventoryTransferCode::SourceItemNotFound;
        return result;
    }

    const InventoryPresenceResult destinationPresence = Contains(destination, item);
    if(destinationPresence.code == InventoryPresenceCode::PlayerNotFound)
    {
        result.code = InventoryTransferCode::DestinationPlayerNotFound;
        return result;
    }
    if(destinationPresence.Present())
    {
        result.code = InventoryTransferCode::DestinationAlreadyContains;
        return result;
    }

    const InventoryMutationResult destinationAdd = Add(destination, item);
    if(!destinationAdd.Applied())
    {
        result.code = destinationAdd.code == InventoryMutationCode::CapacityExceeded
                          ? InventoryTransferCode::DestinationCapacityExceeded
                          : InventoryTransferCode::DestinationPlayerNotFound;
        return result;
    }

    const InventoryMutationResult sourceRemove = Remove(source, item);
    if(!sourceRemove.Applied())
    {
        const InventoryMutationResult rollback = Remove(destination, item);
        result.code = rollback.Applied()
                          ? InventoryTransferCode::SourceRemoveRejected
                          : InventoryTransferCode::RollbackFailed;
        return result;
    }

    result.code = InventoryTransferCode::Applied;
    return result;
}

InventoryGrantResult LegacyInventoryRuntime::Grant(const ItemGrant& grant)
{
    InventoryGrantResult result;
    result.grant = grant;

    if(!grant.item.Valid())
    {
        result.code = InventoryGrantCode::InvalidItem;
        return result;
    }
    if(grant.quantity != 1u)
    {
        result.code = InventoryGrantCode::InvalidQuantity;
        return result;
    }

    rsPLAYINFO* legacyPlayer = registry_.Resolve(grant.player);
    if(!legacyPlayer || !legacyPlayer->lpsmSock)
    {
        result.code = InventoryGrantCode::PlayerNotFound;
        return result;
    }

    result.code = rsPutItemChecked(legacyPlayer, static_cast<DWORD>(grant.item.value))
                      ? InventoryGrantCode::Granted
                      : InventoryGrantCode::LegacyRejected;
    return result;
}

LegacyInventoryRuntime& ServerLegacyInventoryRuntime()
{
    static LegacyInventoryRuntime runtime(player::legacy::ServerLegacyOnlinePlayerRegistry());
    return runtime;
}
}
}
}
