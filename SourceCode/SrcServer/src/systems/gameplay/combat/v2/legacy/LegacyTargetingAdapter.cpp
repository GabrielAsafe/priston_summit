#include "systems/gameplay/combat/v2/legacy/LegacyTargetingAdapter.h"

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
namespace
{
TargetingResult Result(TargetingCode code, bool changed = false)
{
    TargetingResult result;
    result.code = code;
    result.changed = changed;
    return result;
}

smCHAR* Resolve(EntityId id)
{
    return id.Valid() ? srFindCharFromSerial(static_cast<DWORD>(id.value)) : 0;
}
}

TargetingResult LegacyTargetingAdapter::ClearAcquiredTarget(EntityId actor)
{
    if(!actor.Valid())
        return Result(TargetingCode::InvalidRequest);
    smCHAR* resolved = Resolve(actor);
    if(!resolved)
        return Result(TargetingCode::ActorNotFound);

    const bool changed = resolved->lpTargetChar != 0 || resolved->lpTargetPlayInfo != 0 ||
        resolved->TargetMoveCount != 0 || resolved->AutoMoveStep != 0;
    resolved->lpTargetChar = 0;
    resolved->lpTargetPlayInfo = 0;
    resolved->TargetMoveCount = 0;
    resolved->AutoMoveStep = 0;
    return Result(TargetingCode::Ok, changed);
}

TargetingResult LegacyTargetingAdapter::ClearPlayerTarget(EntityId actor)
{
    if(!actor.Valid())
        return Result(TargetingCode::InvalidRequest);
    smCHAR* resolved = Resolve(actor);
    if(!resolved)
        return Result(TargetingCode::ActorNotFound);

    const bool changed = resolved->lpTargetPlayInfo != 0;
    resolved->lpTargetPlayInfo = 0;
    return Result(TargetingCode::Ok, changed);
}

TargetingResult LegacyTargetingAdapter::ReleaseTarget(EntityId actor, EntityId expectedTarget,
                                                        TargetReleaseMode mode)
{
    if(!actor.Valid() || !expectedTarget.Valid())
        return Result(TargetingCode::InvalidRequest);
    smCHAR* resolvedActor = Resolve(actor);
    if(!resolvedActor)
        return Result(TargetingCode::ActorNotFound);
    smCHAR* resolvedTarget = Resolve(expectedTarget);
    if(!resolvedTarget)
        return Result(TargetingCode::TargetNotFound);

    bool changed = false;
    if(resolvedActor->lpTargetChar == resolvedTarget)
    {
        resolvedActor->lpTargetChar = 0;
        changed = true;
    }
    if(mode == TargetReleaseMode::AcquiredAndForced && resolvedActor->lpCompulsionChar == resolvedTarget)
    {
        resolvedActor->lpCompulsionChar = 0;
        resolvedActor->dwCompulsionTime = 0;
        changed = true;
    }
    return Result(TargetingCode::Ok, changed);
}

TargetingResult LegacyTargetingAdapter::ForceTarget(EntityId actor, EntityId target,
                                                      std::uint32_t expiresAt)
{
    if(!actor.Valid() || !target.Valid() || expiresAt == 0u)
        return Result(TargetingCode::InvalidRequest);
    smCHAR* resolvedActor = Resolve(actor);
    if(!resolvedActor)
        return Result(TargetingCode::ActorNotFound);
    smCHAR* resolvedTarget = Resolve(target);
    if(!resolvedTarget)
        return Result(TargetingCode::TargetNotFound);
    if(!resolvedActor->Flag)
        return Result(TargetingCode::ActorInactive);
    if(!resolvedTarget->Flag)
        return Result(TargetingCode::TargetInactive);

    resolvedActor->lpTargetPlayInfo = 0;
    resolvedActor->lpTargetChar = resolvedTarget;
    resolvedActor->lpCompulsionPlayInfo = 0;
    resolvedActor->lpCompulsionChar = resolvedTarget;
    resolvedActor->dwCompulsionTime = static_cast<DWORD>(expiresAt);
    resolvedActor->SetNextTarget(resolvedTarget->pX, resolvedTarget->pY, resolvedTarget->pZ);
    return Result(TargetingCode::Ok, true);
}

LegacyTargetingAdapter& ServerLegacyTargetingAdapter()
{
    static LegacyTargetingAdapter adapter;
    return adapter;
}
}
}
}
