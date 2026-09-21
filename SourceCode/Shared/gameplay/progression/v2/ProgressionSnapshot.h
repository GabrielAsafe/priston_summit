#pragma once

#include <vector>

#include "gameplay/covenants/v2/CovenantTypes.h"
#include "gameplay/reputation/v2/ReputationTypes.h"
#include "gameplay/story/v2/StoryTypes.h"

namespace gameplay
{
namespace progression
{
struct ProgressionPlayerSnapshot
{
    std::vector<covenants::CovenantSnapshot> activeCovenants;
    std::vector<reputation::ReputationSnapshot> reputations;
    std::vector<story::StorySnapshot> startedStories;
    int pendingStoryEffectCount;
    int pendingCovenantEffectCount;

    ProgressionPlayerSnapshot()
        : pendingStoryEffectCount(0), pendingCovenantEffectCount(0)
    {
    }
};
}
}
