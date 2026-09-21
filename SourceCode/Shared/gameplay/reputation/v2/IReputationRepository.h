#pragma once

#include <string>

namespace gameplay
{
namespace reputation
{
enum class ReputationLoadResult
{
    Found,
    NotFound,
    Failed
};

enum class ReputationChangeResult
{
    Applied,
    AlreadyApplied,
    Conflict,
    Failed
};

class IReputationRepository
{
public:
    virtual ~IReputationRepository() {}

    virtual ReputationLoadResult LoadValue(const std::string& playerKey,
                                            const std::string& reputationKey,
                                            int& value) = 0;

    virtual ReputationChangeResult ApplyChange(const std::string& playerKey,
                                                const std::string& reputationKey,
                                                int expectedValue,
                                                int newValue,
                                                int delta,
                                                const std::string& sourceKey) = 0;
};
}
}
