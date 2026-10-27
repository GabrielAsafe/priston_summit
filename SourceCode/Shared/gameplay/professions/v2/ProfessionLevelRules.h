#pragma once
// GENERATED from SourceCode/Content/canonical/professions_v2/levels.experimental.json
// PROFESSION_V2_LEVEL_POLICY_EXPERIMENTAL_V1: independent of legacy JOBS.
#include <cstdint>
#include <cstddef>

namespace gameplay
{
namespace professions
{
inline int ProfessionLevelForExperience(std::uint64_t totalExperience)
{
    static const std::uint64_t thresholds[] = {
        0ULL,
        10ULL,
        30ULL,
        60ULL,
        100ULL,
        150ULL,
        210ULL,
        280ULL,
        360ULL,
        450ULL,
        550ULL,
        660ULL,
        780ULL,
        910ULL,
        1050ULL,
        1200ULL,
        1360ULL,
        1530ULL,
        1710ULL,
        1900ULL,
        2100ULL,
        2310ULL,
        2530ULL,
        2760ULL,
        3000ULL,
        3250ULL,
        3510ULL,
        3780ULL,
        4060ULL,
        4350ULL,
        4650ULL,
        4960ULL,
        5280ULL,
        5610ULL,
        5950ULL,
        6300ULL,
        6660ULL,
        7030ULL,
        7410ULL,
        7800ULL,
        8200ULL,
        8610ULL,
        9030ULL,
        9460ULL,
        9900ULL,
        10350ULL,
        10810ULL,
        11280ULL,
        11760ULL,
        12250ULL,
        12750ULL
    };
    int level = 0;
    for (std::size_t i = 1; i < sizeof(thresholds) / sizeof(thresholds[0]); ++i)
    {
        if (totalExperience < thresholds[i])
            break;
        level = static_cast<int>(i);
    }
    return level;
}
}
}
