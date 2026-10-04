#pragma once

#include <algorithm>

namespace gameplay::progression
{
struct BattlePassProgressState
{
    int experience = 0;
    int level = 0;
    int passType = 0;
};

struct BattlePassProgressResult
{
    BattlePassProgressState state;
    int levelsGained = 0;
    bool changed = false;
};

class BattlePassProgressService final
{
public:
    static constexpr int ExperiencePerLevel = 999;
    static constexpr int MaxLevel = 20;

    static BattlePassProgressResult AddExperience(const BattlePassProgressState& current,
                                                  int amount) noexcept
    {
        BattlePassProgressResult result;
        result.state = current;

        if(current.passType <= 0 || amount <= 0)
            return result;

        result.changed = true;

        if(result.state.level >= MaxLevel)
        {
            result.state.level = MaxLevel;
            result.state.experience = ExperiencePerLevel;
            return result;
        }

        long long accumulated = static_cast<long long>(std::max(0, result.state.experience)) + amount;
        while(accumulated >= ExperiencePerLevel && result.state.level < MaxLevel)
        {
            accumulated -= ExperiencePerLevel;
            ++result.state.level;
            ++result.levelsGained;
        }

        if(result.state.level >= MaxLevel)
            result.state.experience = ExperiencePerLevel;
        else
            result.state.experience = static_cast<int>(accumulated);

        return result;
    }
};
}
