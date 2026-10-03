#include "systems/gameplay/combat/v2/legacy/LegacyLifeAdapter.h"

#include <algorithm>

#include "systems/network/transport/legacy/smwsock.h"
#include "systems/rendering/legacy_3d/smd3d.h"
#include "systems/gameplay/character/core/character.h"
#include "core/server/legacy_runtime/onserver.h"
#include "systems/gameplay/player/v2/legacy/LegacyOnlinePlayerRegistry.h"

namespace gameplay
{
namespace combat
{
namespace legacy
{
namespace
{
LifeCommandResult InvalidResult(LifeCommandCode code = LifeCommandCode::InvalidRequest)
{
    LifeCommandResult result;
    result.code = code;
    return result;
}

LifeCommandResult SnapshotResult(LifeCommandCode code, int before, int after, int maximum)
{
    LifeCommandResult result;
    result.code = code;
    result.before = before;
    result.after = after;
    result.maximum = maximum;
    return result;
}

smCHAR* ResolveEntity(EntityId target)
{
    if(!target.Valid())
        return 0;
    return srFindCharFromSerial(static_cast<DWORD>(target.value));
}
}

LifeCommandResult LegacyLifeAdapter::Restore(EntityId target, int amount)
{
    if(!target.Valid() || amount <= 0)
        return InvalidResult();
    smCHAR* entity = ResolveEntity(target);
    if(!entity)
        return InvalidResult(LifeCommandCode::NotFound);
    if(!entity->Flag)
        return InvalidResult(LifeCommandCode::Inactive);

    const int before = entity->smCharInfo.Life[0];
    const int maximum = entity->smCharInfo.Life[1];
    if(before <= 0 || maximum <= 0)
        return SnapshotResult(LifeCommandCode::InvalidState, before, before, maximum);

    const int after = std::min(maximum, before + amount);
    entity->smCharInfo.Life[0] = static_cast<short>(after);
    return SnapshotResult(LifeCommandCode::Ok, before, after, maximum);
}

LifeCommandResult LegacyLifeAdapter::Restore(PlayerId target, int amount)
{
    if(!target.Valid() || amount <= 0)
        return InvalidResult();
    rsPLAYINFO* player = player::legacy::ServerLegacyOnlinePlayerRegistry().Resolve(target);
    if(!player)
        return InvalidResult(LifeCommandCode::NotFound);

    const int before = player->smCharInfo.Life[0];
    const int maximum = player->smCharInfo.Life[1];
    if(before <= 0 || maximum <= 0)
        return SnapshotResult(LifeCommandCode::InvalidState, before, before, maximum);

    const int after = std::min(maximum, before + amount);
    player->smCharInfo.Life[0] = static_cast<short>(after);
    return SnapshotResult(LifeCommandCode::Ok, before, after, maximum);
}

LifeCommandResult LegacyLifeAdapter::ConsumeNonLethal(EntityId target, int amount)
{
    if(!target.Valid() || amount <= 0)
        return InvalidResult();
    smCHAR* entity = ResolveEntity(target);
    if(!entity)
        return InvalidResult(LifeCommandCode::NotFound);
    if(!entity->Flag)
        return InvalidResult(LifeCommandCode::Inactive);

    const int before = entity->smCharInfo.Life[0];
    const int maximum = entity->smCharInfo.Life[1];
    if(before <= 0 || maximum <= 0)
        return SnapshotResult(LifeCommandCode::InvalidState, before, before, maximum);
    if(before - amount <= 0)
        return SnapshotResult(LifeCommandCode::WouldKill, before, before, maximum);

    const int after = before - amount;
    entity->smCharInfo.Life[0] = static_cast<short>(after);
    return SnapshotResult(LifeCommandCode::Ok, before, after, maximum);
}

LifeCommandResult LegacyLifeAdapter::ConsumeLethal(EntityId target, int amount)
{
    if(!target.Valid() || amount <= 0)
        return InvalidResult();
    smCHAR* entity = ResolveEntity(target);
    if(!entity)
        return InvalidResult(LifeCommandCode::NotFound);
    if(!entity->Flag)
        return InvalidResult(LifeCommandCode::Inactive);

    const int before = entity->smCharInfo.Life[0];
    const int maximum = entity->smCharInfo.Life[1];
    if(before <= 0 || maximum <= 0)
        return SnapshotResult(LifeCommandCode::InvalidState, before, before, maximum);

    const int after = std::max(0, before - amount);
    entity->smCharInfo.Life[0] = static_cast<short>(after);
    if(after <= 0)
        entity->SetMotionFromCode(CHRMOTION_STATE_DEAD);
    return SnapshotResult(LifeCommandCode::Ok, before, after, maximum);
}

LifeCommandResult LegacyLifeAdapter::Revive(EntityId target, int percentOfMaximum)
{
    if(!target.Valid() || percentOfMaximum < 1 || percentOfMaximum > 100)
        return InvalidResult();
    smCHAR* entity = ResolveEntity(target);
    if(!entity)
        return InvalidResult(LifeCommandCode::NotFound);
    if(!entity->Flag)
        return InvalidResult(LifeCommandCode::Inactive);

    const int before = entity->smCharInfo.Life[0];
    const int maximum = entity->smCharInfo.Life[1];
    if(before > 0 || maximum <= 0)
        return SnapshotResult(LifeCommandCode::InvalidState, before, before, maximum);

    const int after = std::max(1, (maximum * percentOfMaximum) / 100);
    entity->smCharInfo.Life[0] = static_cast<short>(after);
    entity->SetMotionFromCode(CHRMOTION_STATE_STAND);
    return SnapshotResult(LifeCommandCode::Ok, before, after, maximum);
}

LegacyLifeAdapter& ServerLegacyLifeAdapter()
{
    static LegacyLifeAdapter adapter;
    return adapter;
}
}
}
}
