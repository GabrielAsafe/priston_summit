#include "systems/gameplay/covenants/v2/CovenantUIHandler.h"

#include "systems/gameplay/covenants/v2/legacy/LegacyCovenantUIIngressAdapter.h"

namespace gameplay
{
namespace covenants
{
void HandleCovenantUIAction(PlayerId owner,
                            const protocol::CovenantUIActionPacket& packet)
{
    legacy::ServerLegacyCovenantUIIngressAdapter().HandleAction(owner, packet);
}
}
}
