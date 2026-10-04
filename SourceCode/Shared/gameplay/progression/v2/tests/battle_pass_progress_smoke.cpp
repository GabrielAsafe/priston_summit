#include <cassert>
#include "gameplay/progression/v2/BattlePassProgressService.h"

int main()
{
    using gameplay::progression::BattlePassProgressService;
    using gameplay::progression::BattlePassProgressState;

    auto inactive = BattlePassProgressService::AddExperience({998, 3, 0}, 10);
    assert(!inactive.changed);
    assert(inactive.state.experience == 998 && inactive.state.level == 3);

    auto threshold = BattlePassProgressService::AddExperience({998, 3, 1}, 1);
    assert(threshold.changed);
    assert(threshold.levelsGained == 1);
    assert(threshold.state.level == 4 && threshold.state.experience == 0);

    auto overflow = BattlePassProgressService::AddExperience({0, 1, 1}, 2500);
    assert(overflow.levelsGained == 2);
    assert(overflow.state.level == 3);
    assert(overflow.state.experience == 502);

    auto rollover = BattlePassProgressService::AddExperience({900, 5, 2}, 1200);
    assert(rollover.levelsGained == 2);
    assert(rollover.state.level == 7);
    assert(rollover.state.experience == 102);

    auto capped = BattlePassProgressService::AddExperience({998, 19, 2}, 5000);
    assert(capped.levelsGained == 1);
    assert(capped.state.level == BattlePassProgressService::MaxLevel);
    assert(capped.state.experience == BattlePassProgressService::ExperiencePerLevel);

    auto alreadyMax = BattlePassProgressService::AddExperience({17, 20, 1}, 1);
    assert(alreadyMax.changed);
    assert(alreadyMax.levelsGained == 0);
    assert(alreadyMax.state.level == 20 && alreadyMax.state.experience == 999);

    return 0;
}
