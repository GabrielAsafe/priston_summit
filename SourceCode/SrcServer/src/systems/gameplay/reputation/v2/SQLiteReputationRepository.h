#pragma once

#include "gameplay/reputation/v2/IReputationRepository.h"

namespace gameplay
{
namespace reputation
{
class SQLiteReputationRepository : public IReputationRepository
{
public:
    bool Ready() { return EnsureSchema(); }

    ReputationLoadResult LoadValue(const std::string& playerKey,
                                   const std::string& reputationKey,
                                   int& value);

    ReputationChangeResult ApplyChange(const std::string& playerKey,
                                       const std::string& reputationKey,
                                       int expectedValue,
                                       int newValue,
                                       int delta,
                                       const std::string& sourceKey);

private:
    bool EnsureSchema();
};
}
}
