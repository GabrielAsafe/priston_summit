#pragma once

#include "gameplay/events/v2/GameplayEventHub.h"

class rsPLAYINFO;
class smCHAR;

namespace gameplay
{
namespace events
{
GameplayEventHub& ServerGameplayEventHub();
void PublishMonsterKilled(rsPLAYINFO* player, smCHAR* monster, bool partyCredit);
}
}
