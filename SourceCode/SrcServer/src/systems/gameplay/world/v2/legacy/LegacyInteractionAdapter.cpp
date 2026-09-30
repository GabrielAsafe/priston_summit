#include "systems/gameplay/world/v2/legacy/LegacyInteractionAdapter.h"

#include "systems/gameplay/events/hells_gate/cHellsGate.h"
#include "systems/gameplay/world/v2/legacy/LegacyNpcAdapter.h"

namespace gameplay
{
namespace world
{
namespace legacy
{
InteractionResult LegacyInteractionAdapter::Interact(const InteractionRequest& request)
{
    if (!request.player.Valid())
        return InteractionResult(InteractionStatus::InvalidPlayer, 0u);
    if (!request.target.Valid())
        return InteractionResult(InteractionStatus::InvalidTarget, 0u);

    NpcSnapshot npc;
    if (!ServerLegacyNpcAdapter().TryGetNpc(request.target, npc))
        return InteractionResult(InteractionStatus::InvalidTarget, 0u);

    // Preserve the only modernized NPC interaction currently routed through
    // this boundary. Other legacy NPC behaviors continue through their
    // existing paths until migrated individually.
    if (npc.eventCode == 988u)
    {
        cHg.SendOpenNpc(request.player);
        return InteractionResult(InteractionStatus::Handled, npc.eventCode);
    }

    return InteractionResult(InteractionStatus::Unhandled, npc.eventCode);
}

LegacyInteractionAdapter& ServerLegacyInteractionAdapter()
{
    static LegacyInteractionAdapter adapter;
    return adapter;
}
}
}
}
