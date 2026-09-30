#pragma once

#include <algorithm>
#include <cstdint>

namespace gameplay
{
namespace professions
{
enum class ProfessionType
{
    None = 0,
    Arcanist,
    Blacksmith,
    Lumberjack,
    Miner,
    Artisan,
    Fisher,
    Hunter,
    Custom
};

struct ProfessionProgress
{
    int level;
    std::uint64_t experience;

    ProfessionProgress() : level(0), experience(0) {}
    ProfessionProgress(int levelValue, std::uint64_t experienceValue)
        : level(std::max(0, levelValue)), experience(experienceValue) {}
};
}
}
