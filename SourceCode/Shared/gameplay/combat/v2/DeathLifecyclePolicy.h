#pragma once

namespace gameplay
{
namespace combat
{
class DeathLifecyclePolicy
{
public:
    static bool ShouldResolveChangeJobArenaDeath(bool changeJob4QuestActive, bool inQuestArena)
    {
        return changeJob4QuestActive && inQuestArena;
    }
};
}
}
