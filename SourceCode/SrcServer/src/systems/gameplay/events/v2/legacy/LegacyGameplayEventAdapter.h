#pragma once

class rsPLAYINFO;
class smCHAR;

namespace gameplay
{
namespace events
{
namespace legacy
{
class LegacyGameplayEventAdapter
{
public:
    void PublishMonsterKilled(rsPLAYINFO* player, smCHAR* monster, bool partyCredit);
};

LegacyGameplayEventAdapter& ServerLegacyGameplayEventAdapter();
}
}
}
