#pragma once

#include <cstdint>

namespace gameplay::progression
{
enum class RankingMetric : std::uint8_t
{
    Level = 1,
    Kills = 2
};

struct ClassRankQuery
{
    RankingMetric metric = RankingMetric::Level;
    int classCode = 0;

    bool IsGlobal() const noexcept { return classCode == 0; }
};

class ClassRankQueryService
{
public:
    static bool TryFromLegacy(int type, int classCode, ClassRankQuery& query) noexcept
    {
        switch (type)
        {
        case 1:
            query = {RankingMetric::Level, classCode};
            return true;
        case 2:
            query = {RankingMetric::Kills, classCode};
            return true;
        default:
            return false;
        }
    }
};
}
