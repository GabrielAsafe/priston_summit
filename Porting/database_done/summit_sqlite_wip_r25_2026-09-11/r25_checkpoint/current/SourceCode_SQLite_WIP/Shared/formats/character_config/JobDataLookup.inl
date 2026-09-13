#pragma once

namespace summit {
namespace formats {
namespace character_config {

template <typename JobData, typename BaseJobType>
JobData *FindJobData(
    BaseJobType baseJob,
    int requestedRank,
    JobData *baseJobs,
    JobData *rank5Jobs,
    int firstRank,
    int rank5)
{
    int jobCount = 0;
    int resolvedRank = requestedRank;
    const int jobCode = static_cast<int>(baseJob & static_cast<BaseJobType>(0xFFFF));

    if(!requestedRank)
        resolvedRank = static_cast<int>(baseJob >> 24);

    if(resolvedRank < firstRank || resolvedRank > rank5)
        return nullptr;

    if(resolvedRank == rank5)
    {
        for(int index = 0; rank5Jobs[index].szName[0]; ++index)
        {
            if(jobCode == static_cast<int>(rank5Jobs[index].JobCode))
                return &rank5Jobs[index];
        }
        return nullptr;
    }

    for(int index = 0; baseJobs[index].szName[0]; ++index)
    {
        if(jobCode != static_cast<int>(baseJobs[index].JobCode))
            continue;

        if(resolvedRank == jobCount)
            return &baseJobs[index];

        ++jobCount;
    }

    return nullptr;
}

} // namespace character_config
} // namespace formats
} // namespace summit
