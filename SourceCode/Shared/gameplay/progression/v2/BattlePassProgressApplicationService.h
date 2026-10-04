#pragma once

#include <string>

#include "gameplay/progression/v2/BattlePassProgressService.h"

namespace gameplay::progression
{
class IBattlePassProgressMutationPort
{
public:
    virtual ~IBattlePassProgressMutationPort() = default;
    virtual bool AddExperience(const std::string& accountId,
                               int amount,
                               BattlePassProgressResult& result) = 0;
};

class BattlePassProgressApplicationService final
{
public:
    static bool AddExperience(IBattlePassProgressMutationPort& port,
                              const std::string& accountId,
                              int amount,
                              BattlePassProgressResult& result)
    {
        result = BattlePassProgressResult{};
        if(accountId.empty())
            return false;
        return port.AddExperience(accountId, amount, result);
    }
};
}
