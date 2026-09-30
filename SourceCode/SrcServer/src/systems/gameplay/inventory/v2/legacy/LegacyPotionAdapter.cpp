#include "systems/gameplay/inventory/v2/legacy/LegacyPotionAdapter.h"

#include <algorithm>

#include "systems/network/transport/legacy/smwsock.h"
#include "systems/rendering/legacy_3d/smd3d.h"
#include "systems/gameplay/character/core/character.h"
#include "systems/persistence/character_record/record.h"
#include "core/server/legacy_runtime/onserver.h"

namespace gameplay
{
namespace inventory
{
namespace legacy
{
namespace
{
PotionUseResult Failure(PotionUseCode code, int legacyStatus = 0)
{
    PotionUseResult result;
    result.code = code;
    result.legacyStatus = legacyStatus;
    return result;
}

bool ResourceValues(smCHAR* target, PotionResource resource, int& currentValue, int& maxValue)
{
    if(!target)
        return false;
    switch(resource)
    {
        case PotionResource::Life:
            currentValue = target->smCharInfo.Life[0];
            maxValue = target->smCharInfo.Life[1];
            return true;
        case PotionResource::Mana:
            currentValue = target->smCharInfo.Mana[0];
            maxValue = target->smCharInfo.Mana[1];
            return true;
        case PotionResource::Stamina:
            currentValue = target->smCharInfo.Stamina[0];
            maxValue = target->smCharInfo.Stamina[1];
            return true;
    }
    return false;
}

void ApplyRestore(smCHAR* target, PotionResource resource, int restore)
{
    switch(resource)
    {
        case PotionResource::Life:
            target->smCharInfo.Life[0] = static_cast<short>(std::min(
                static_cast<int>(target->smCharInfo.Life[1]),
                static_cast<int>(target->smCharInfo.Life[0]) + restore));
            break;
        case PotionResource::Mana:
            target->smCharInfo.Mana[0] = static_cast<short>(std::min(
                static_cast<int>(target->smCharInfo.Mana[1]),
                static_cast<int>(target->smCharInfo.Mana[0]) + restore));
            break;
        case PotionResource::Stamina:
            target->smCharInfo.Stamina[0] = static_cast<short>(std::min(
                static_cast<int>(target->smCharInfo.Stamina[1]),
                static_cast<int>(target->smCharInfo.Stamina[0]) + restore));
            break;
    }
}
}

PotionUseResult LegacyPotionAdapter::ConsumeAndApply(const PotionUseRequest& request)
{
    if(request.accountName.empty() || request.characterName.empty() || !request.target.Valid())
        return Failure(PotionUseCode::InvalidRequest, -1);

    smCHAR* target = srFindCharFromSerial(static_cast<DWORD>(request.target.value));
    if(!target)
        return Failure(PotionUseCode::TargetNotFound, -1);

    PotionUseResult result;
    if(!ResourceValues(target, request.resource, result.beforeValue, result.maxValue))
        return Failure(PotionUseCode::InvalidRequest, -1);
    result.afterValue = result.beforeValue;

    int tier = -1;
    int remaining = 0;
    int restore = 0;
    int status = 0;
    DWORD itemCode = 0;
    const int type = static_cast<int>(request.resource);
    if(!rsConsumeOfflineCharacterPotion(request.accountName.c_str(), request.characterName.c_str(), type,
        &tier, &remaining, &restore, &itemCode, &status))
    {
        result.code = PotionUseCode::ConsumeRejected;
        result.legacyStatus = status;
        result.tier = tier;
        result.remainingCount = remaining;
        result.restoreAmount = restore;
        result.itemCode = static_cast<std::uint32_t>(itemCode);
        return result;
    }

    result.consumed = true;
    result.legacyStatus = status;
    result.tier = tier;
    result.remainingCount = remaining;
    result.restoreAmount = restore;
    result.itemCode = static_cast<std::uint32_t>(itemCode);
    if(restore <= 0)
    {
        result.code = PotionUseCode::RestoreInvalid;
        result.legacyStatus = -11;
        return result;
    }

    ApplyRestore(target, request.resource, restore);
    ResourceValues(target, request.resource, result.afterValue, result.maxValue);
    result.code = PotionUseCode::Ok;
    return result;
}

LegacyPotionAdapter& ServerLegacyPotionAdapter()
{
    static LegacyPotionAdapter adapter;
    return adapter;
}
}
}
}
