#pragma once

#include "gameplay/covenants/v2/CovenantUIProtocol.h"

class rsPLAYINFO;

namespace gameplay
{
namespace covenants
{
void SendCovenantUIState(rsPLAYINFO* owner,
                         int selectedCovenantId = 0,
                         int lastResult = protocol::COVENANT_UI_RESULT_NONE,
                         const char* lastMessage = 0);
void HandleCovenantUIAction(rsPLAYINFO* owner,
                            const protocol::CovenantUIActionPacket* packet);
}
}
