#include "systems/gameplay/inventory/v2/legacy/LegacyPotionStackAdapter.h"

#include <limits>

#include "systems/network/transport/legacy/smwsock.h"
#include "systems/rendering/legacy_3d/smd3d.h"
#include "systems/gameplay/character/core/character.h"
#include "core/server/legacy_runtime/onserver.h"
#include "systems/inventory/legacy/sinItem.h"

namespace gameplay
{
namespace inventory
{
namespace legacy
{
namespace
{
bool IsSupportedPotion(std::uint32_t code)
{
    switch(code)
    {
        case (sinPL1 | sin01): case (sinPL1 | sin02): case (sinPL1 | sin03): case (sinPL1 | sin04): case (sinPL1 | sin05):
        case (sinPM1 | sin01): case (sinPM1 | sin02): case (sinPM1 | sin03): case (sinPM1 | sin04): case (sinPM1 | sin05):
        case (sinPS1 | sin01): case (sinPS1 | sin02): case (sinPS1 | sin03): case (sinPS1 | sin04): case (sinPS1 | sin05):
            return true;
    }
    return false;
}

bool RestorePotionCount(rsPLAYINFO* player, DWORD code, int target)
{
    const int current = rsGetServerPotion(player, code);
    if(current == target)
        return true;
    const long long delta = static_cast<long long>(target) - static_cast<long long>(current);
    if(delta < static_cast<long long>(std::numeric_limits<int>::min()) ||
       delta > static_cast<long long>(std::numeric_limits<int>::max()))
        return false;
    rsAddServerPotion(player, code, static_cast<int>(delta));
    return rsGetServerPotion(player, code) == target;
}

PotionStackMutationResult MutatePotionStack(
    const player::legacy::LegacyOnlinePlayerRegistry& registry,
    const PotionStackMutationRequest& request,
    bool add)
{
    PotionStackMutationResult result;
    result.request = request;
    if(!request.player.Valid() || !request.potion.Valid() || !IsSupportedPotion(request.potion.value) ||
       request.quantity <= 0 ||
       request.quantity > static_cast<PotionQuantity>(std::numeric_limits<int>::max()))
    {
        result.code = PotionStackMutationCode::InvalidRequest;
        return result;
    }

    rsPLAYINFO* legacyPlayer = registry.Resolve(request.player);
    if(!legacyPlayer)
    {
        result.code = PotionStackMutationCode::PlayerNotFound;
        return result;
    }

    const DWORD code = static_cast<DWORD>(request.potion.value);
    const int before = rsGetServerPotion(legacyPlayer, code);
    result.before = before;
    result.after = before;
    if(before < 0)
    {
        result.code = PotionStackMutationCode::MutationRejected;
        return result;
    }

    if(add)
    {
        if(request.quantity > static_cast<PotionQuantity>(std::numeric_limits<int>::max()) - before)
        {
            result.code = PotionStackMutationCode::CapacityExceeded;
            return result;
        }
    }
    else if(before < request.quantity)
    {
        result.code = PotionStackMutationCode::InsufficientQuantity;
        return result;
    }

    const int delta = static_cast<int>(request.quantity) * (add ? 1 : -1);
    const int expected = before + delta;
    rsAddServerPotion(legacyPlayer, code, delta);
    result.after = rsGetServerPotion(legacyPlayer, code);
    if(result.after != expected)
    {
        const bool restored = RestorePotionCount(legacyPlayer, code, before);
        result.after = rsGetServerPotion(legacyPlayer, code);
        result.code = restored ? PotionStackMutationCode::MutationRejected
                               : PotionStackMutationCode::RollbackFailed;
        return result;
    }

    result.code = PotionStackMutationCode::Applied;
    return result;
}
}

LegacyPotionStackAdapter::LegacyPotionStackAdapter(
    const player::legacy::LegacyOnlinePlayerRegistry& registry)
    : registry_(registry)
{
}

bool LegacyPotionStackAdapter::TryGetStack(PlayerId player, ItemId potion, PotionStackSnapshot& out) const
{
    if(!potion.Valid() || !IsSupportedPotion(potion.value))
        return false;
    rsPLAYINFO* legacyPlayer = registry_.Resolve(player);
    if(!legacyPlayer)
        return false;
    const int quantity = rsGetServerPotion(legacyPlayer, static_cast<DWORD>(potion.value));
    if(quantity < 0)
        return false;
    out = PotionStackSnapshot(player, potion, static_cast<PotionQuantity>(quantity));
    return true;
}

PotionStackMutationResult LegacyPotionStackAdapter::AddStack(
    const PotionStackMutationRequest& request)
{
    return MutatePotionStack(registry_, request, true);
}

PotionStackMutationResult LegacyPotionStackAdapter::RemoveStack(
    const PotionStackMutationRequest& request)
{
    return MutatePotionStack(registry_, request, false);
}

PotionStackTransferResult LegacyPotionStackAdapter::TransferStack(
    const PotionStackTransferRequest& request)
{
    PotionStackTransferResult result;
    result.request = request;
    if(!request.source.Valid() || !request.destination.Valid() || request.source == request.destination ||
       !request.potion.Valid() || !IsSupportedPotion(request.potion.value) || request.quantity <= 0 ||
       request.quantity > static_cast<PotionQuantity>(std::numeric_limits<int>::max()))
    {
        result.code = PotionStackTransferCode::InvalidRequest;
        return result;
    }

    rsPLAYINFO* source = registry_.Resolve(request.source);
    if(!source)
    {
        result.code = PotionStackTransferCode::SourcePlayerNotFound;
        return result;
    }
    rsPLAYINFO* destination = registry_.Resolve(request.destination);
    if(!destination)
    {
        result.code = PotionStackTransferCode::DestinationPlayerNotFound;
        return result;
    }

    const DWORD code = static_cast<DWORD>(request.potion.value);
    const int sourceBefore = rsGetServerPotion(source, code);
    const int destinationBefore = rsGetServerPotion(destination, code);
    result.sourceBefore = sourceBefore;
    result.sourceAfter = sourceBefore;
    result.destinationBefore = destinationBefore;
    result.destinationAfter = destinationBefore;

    if(sourceBefore < request.quantity)
    {
        result.code = PotionStackTransferCode::InsufficientQuantity;
        return result;
    }
    if(destinationBefore < 0 ||
       request.quantity > static_cast<PotionQuantity>(std::numeric_limits<int>::max()) - destinationBefore)
    {
        result.code = PotionStackTransferCode::DestinationCapacityExceeded;
        return result;
    }

    rsAddServerPotion(source, code, -static_cast<int>(request.quantity));
    result.sourceAfter = rsGetServerPotion(source, code);
    if(result.sourceAfter != sourceBefore - request.quantity)
    {
        const bool restored = RestorePotionCount(source, code, sourceBefore);
        result.sourceAfter = rsGetServerPotion(source, code);
        result.code = restored ? PotionStackTransferCode::SourceMutationRejected
                               : PotionStackTransferCode::RollbackFailed;
        return result;
    }

    rsAddServerPotion(destination, code, static_cast<int>(request.quantity));
    result.destinationAfter = rsGetServerPotion(destination, code);
    if(result.destinationAfter != destinationBefore + request.quantity)
    {
        const bool destinationRestored = RestorePotionCount(destination, code, destinationBefore);
        const bool sourceRestored = RestorePotionCount(source, code, sourceBefore);
        result.sourceAfter = rsGetServerPotion(source, code);
        result.destinationAfter = rsGetServerPotion(destination, code);
        result.code = destinationRestored && sourceRestored
                          ? PotionStackTransferCode::DestinationMutationRejected
                          : PotionStackTransferCode::RollbackFailed;
        return result;
    }

    result.code = PotionStackTransferCode::Applied;
    return result;
}

LegacyPotionStackAdapter& ServerLegacyPotionStackAdapter()
{
    static LegacyPotionStackAdapter adapter(player::legacy::ServerLegacyOnlinePlayerRegistry());
    return adapter;
}
}
}
}
