#include "systems/gameplay/world/v2/legacy/LegacyMovementAdapter.h"

#include "systems/network/transport/legacy/smwsock.h"
#include "systems/rendering/legacy_3d/smd3d.h"
#include "systems/gameplay/character/core/character.h"
#include "core/server/legacy_runtime/onserver.h"

namespace gameplay
{
namespace world
{
namespace legacy
{
namespace
{
MovementCommandResult Result(MovementCommandCode code, bool moved = false)
{
    MovementCommandResult result;
    result.code = code;
    result.moved = moved;
    return result;
}
}

MovementCommandResult LegacyMovementAdapter::PushAway(EntityId target, EntityId source, int distance)
{
    if(!target.Valid() || !source.Valid() || distance <= 0)
        return Result(MovementCommandCode::InvalidRequest);

    smCHAR* resolvedTarget = srFindCharFromSerial(static_cast<DWORD>(target.value));
    if(!resolvedTarget)
        return Result(MovementCommandCode::TargetNotFound);
    smCHAR* resolvedSource = srFindCharFromSerial(static_cast<DWORD>(source.value));
    if(!resolvedSource)
        return Result(MovementCommandCode::SourceNotFound);

    const int oldAngle = resolvedTarget->Angle.y;
    const int towardSource = GetRadian2D(resolvedTarget->pX, resolvedTarget->pZ,
                                         resolvedSource->pX, resolvedSource->pZ);
    resolvedTarget->Angle.y = (towardSource + ANGLE_180) & ANGCLIP;
    resolvedTarget->OverLapPosi2 = FALSE;
    resolvedTarget->MoveAngle(distance);
    resolvedTarget->Angle.y = oldAngle;
    resolvedTarget->dwNextMotionCode = CHRMOTION_STATE_STAND;
    return Result(MovementCommandCode::Ok, true);
}

LegacyMovementAdapter& ServerLegacyMovementAdapter()
{
    static LegacyMovementAdapter adapter;
    return adapter;
}
}
}
}
