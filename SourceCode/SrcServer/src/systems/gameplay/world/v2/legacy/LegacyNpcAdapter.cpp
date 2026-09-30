#include "systems/gameplay/world/v2/legacy/LegacyNpcAdapter.h"

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
bool LegacyNpcAdapter::TryGetNpc(EntityId entity, NpcSnapshot& out) const
{
    if (!entity.Valid())
        return false;

    smCHAR* character = srFindCharFromSerial(static_cast<DWORD>(entity.value));
    if (!character)
        return false;

    // Preserve the historical interaction classifier: callers used EventCode
    // directly on the target smCHAR without an additional State == NPC gate.
    out = NpcSnapshot(entity, static_cast<std::uint32_t>(character->smMonsterInfo.EventCode));
    return true;
}

const LegacyNpcAdapter& ServerLegacyNpcAdapter()
{
    static LegacyNpcAdapter adapter;
    return adapter;
}
}
}
}
