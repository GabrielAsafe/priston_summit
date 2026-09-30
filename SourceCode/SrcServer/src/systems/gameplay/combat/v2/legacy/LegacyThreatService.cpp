#include "systems/gameplay/combat/v2/legacy/LegacyThreatService.h"

#include "systems/network/transport/legacy/smwsock.h"
#include "systems/rendering/legacy_3d/smd3d.h"
#include "systems/gameplay/character/core/character.h"
#include "core/server/legacy_runtime/onserver.h"

namespace gameplay
{
namespace combat
{
namespace legacy
{
bool LegacyThreatService::TryGetThreat(EntityId actor, ThreatSnapshot& out) const
{
    if(!actor.Valid())
        return false;

    smCHAR* resolved = srFindCharFromSerial(static_cast<DWORD>(actor.value));
    if(!resolved)
        return false;

    ThreatSnapshot snapshot;
    snapshot.actor = actor;
    snapshot.active = resolved->Flag != 0;
    if(resolved->lpTargetPlayInfo)
        snapshot.targetPlayer = PlayerId(static_cast<std::uint32_t>(resolved->lpTargetPlayInfo->dwObjectSerial));
    if(resolved->lpTargetChar)
        snapshot.targetEntity = EntityId(static_cast<std::uint32_t>(resolved->lpTargetChar->dwObjectSerial));
    out = snapshot;
    return true;
}

LegacyThreatService& ServerLegacyThreatService()
{
    static LegacyThreatService service;
    return service;
}
}
}
}
