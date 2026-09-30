#include "systems/gameplay/combat/v2/legacy/LegacyHealingAdapter.h"

#include "systems/network/protocol/server/opcodes.h"

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
HealingResult MakeResult(HealingCode code, int requested = 0, int before = 0,
                         int after = 0, int maximum = 0, bool delivered = false)
{
    HealingResult result;
    result.code = code;
    result.requested = requested;
    result.before = before;
    result.after = after;
    result.maximum = maximum;
    result.delivered = delivered;
    result.applied = after > before;
    return result;
}

void PrepareHealingCommand(smTRANS_COMMAND& command, EntityId source, std::uint32_t target, int amount)
{
    ZeroMemory(&command, sizeof(command));
    command.size = sizeof(command);
    command.code = OpCode::OPCODE_HEALING;
    command.WParam = amount;
    command.LParam = 0;
    command.SParam = static_cast<DWORD>(target);
    command.EParam = static_cast<DWORD>(source.value);
}
}

HealingResult LegacyHealingAdapter::Heal(EntityId source, EntityId target, int amount)
{
    if(!source.Valid() || !target.Valid() || amount <= 0)
        return MakeResult(HealingCode::InvalidRequest, amount);

    smCHAR* entity = srFindCharFromSerial(static_cast<DWORD>(target.value));
    if(!entity)
        return MakeResult(HealingCode::TargetNotFound, amount);
    if(!entity->Flag)
        return MakeResult(HealingCode::TargetInactive, amount);

    const int before = entity->smCharInfo.Life[0];
    const int maximum = entity->smCharInfo.Life[1];
    if(before <= 0)
        return MakeResult(HealingCode::TargetDead, amount, before, before, maximum);
    if(maximum <= 0)
        return MakeResult(HealingCode::InvalidState, amount, before, before, maximum);

    smTRANS_COMMAND command;
    PrepareHealingCommand(command, source, target.value, amount);
    if(!rsPlayHealing(&command))
        return MakeResult(HealingCode::LegacyRejected, amount, before, before, maximum);

    const int after = entity->smCharInfo.Life[0];
    return MakeResult(HealingCode::Ok, amount, before, after, maximum, true);
}

HealingResult LegacyHealingAdapter::Heal(EntityId source, PlayerId target, int amount)
{
    if(!source.Valid() || !target.Valid() || amount <= 0)
        return MakeResult(HealingCode::InvalidRequest, amount);

    rsPLAYINFO* player = player::legacy::ServerLegacyOnlinePlayerRegistry().Resolve(target);
    if(!player)
        return MakeResult(HealingCode::TargetNotFound, amount);

    const int before = player->smCharInfo.Life[0];
    const int maximum = player->smCharInfo.Life[1];
    if(before <= 0)
        return MakeResult(HealingCode::TargetDead, amount, before, before, maximum);
    if(maximum <= 0)
        return MakeResult(HealingCode::InvalidState, amount, before, before, maximum);

    smTRANS_COMMAND command;
    PrepareHealingCommand(command, source, target.value, amount);
    if(!rsPlayHealing(&command))
        return MakeResult(HealingCode::LegacyRejected, amount, before, before, maximum);

    // rsPlayHealing sends player-target healing to the client rather than
    // mutating the server snapshot. Preserve the Companion runtime's existing
    // mirror update so target selection observes the heal immediately.
    const int after = std::min(maximum, before + amount);
    player->smCharInfo.Life[0] = static_cast<short>(after);
    return MakeResult(HealingCode::Ok, amount, before, after, maximum, true);
}

LegacyHealingAdapter& ServerLegacyHealingAdapter()
{
    static LegacyHealingAdapter adapter;
    return adapter;
}
}
}
}
