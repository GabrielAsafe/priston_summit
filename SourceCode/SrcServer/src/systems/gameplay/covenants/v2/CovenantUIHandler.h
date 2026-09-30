#pragma once

#include "gameplay/core/v2/GameplayIds.h"
#include "gameplay/covenants/v2/CovenantUIProtocol.h"

namespace gameplay
{
namespace covenants
{
void HandleCovenantUIAction(PlayerId owner,
                            const protocol::CovenantUIActionPacket& packet);
}
}
