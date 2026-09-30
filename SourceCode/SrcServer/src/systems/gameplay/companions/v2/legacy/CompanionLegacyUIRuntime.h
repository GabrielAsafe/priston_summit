#pragma once

#include "gameplay/companions/CompanionProtocol.h"

class rsPLAYINFO;

namespace gameplay
{
namespace companions
{
void SendUIState(rsPLAYINFO* owner, int lastResult = 0);
void HandleUIAction(rsPLAYINFO* owner, const protocol::CompanionUIActionPacket* packet);
}
}
