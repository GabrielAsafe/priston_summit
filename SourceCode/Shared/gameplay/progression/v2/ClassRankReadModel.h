#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "gameplay/progression/v2/ClassRankQuery.h"

namespace gameplay::progression
{
struct ClassRankEntry
{
    std::string name;
    int classCode = 0;
    std::int64_t level = 0;
    int kills = 0;
};

class IClassRankRepository
{
public:
    virtual ~IClassRankRepository() = default;
    virtual bool Load(const ClassRankQuery& query, std::vector<ClassRankEntry>& entries) = 0;
};

class ClassRankReadService final
{
public:
    static constexpr std::size_t LegacyResultLimit = 51u;

    static bool LoadTop(IClassRankRepository& repository,
                        const ClassRankQuery& query,
                        std::vector<ClassRankEntry>& entries,
                        std::size_t limit = LegacyResultLimit)
    {
        std::vector<ClassRankEntry> loaded;
        entries.clear();
        if(!repository.Load(query, loaded))
            return false;
        if(loaded.size() > limit)
            loaded.resize(limit);
        entries = std::move(loaded);
        return true;
    }
};
}
