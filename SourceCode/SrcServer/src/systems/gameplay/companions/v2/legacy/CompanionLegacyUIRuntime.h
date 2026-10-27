#pragma once

#include "gameplay/companions/CompanionProtocol.h"

class rsPLAYINFO;

namespace gameplay
{
namespace companions
{
void SendUIState(rsPLAYINFO* owner, int lastResult = 0);
void HandleUIAction(rsPLAYINFO* owner, const protocol::CompanionUIActionPacket* packet);
void HandleTradeConfirm(rsPLAYINFO* owner, const protocol::CompanionTradeConfirmPacket* packet);
void HandleTradeApplyAck350Q(rsPLAYINFO* owner, const protocol::CompanionTradeApplyAckPacket* packet);
}
}
