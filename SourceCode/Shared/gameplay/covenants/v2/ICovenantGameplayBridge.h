#pragma once

#include <string>

#include "gameplay/covenants/v2/CovenantModifierEvaluation.h"

namespace gameplay
{
namespace covenants
{
// Narrow integration boundary used by gameplay domains that consume Covenant
// effects. Domain services depend on this interface instead of the server
// ProgressionRuntime, keeping them portable/testable and preventing SQL/runtime
// dependencies from leaking into Mining/Gathering/etc.
class ICovenantGameplayBridge
{
public:
    virtual ~ICovenantGameplayBridge() {}

    virtual double ApplyModifierValue(const std::string& playerKey,
                                      const std::string& statKey,
                                      const std::string& scopeKey,
                                      double baseValue,
                                      bool includeGlobalScope = true,
                                      unsigned int contextTargetSerial = 0) = 0;

    virtual bool GetModifierAggregate(const std::string& playerKey,
                                      const std::string& statKey,
                                      const std::string& scopeKey,
                                      CovenantModifierAggregate& aggregate,
                                      bool includeGlobalScope = true,
                                      unsigned int contextTargetSerial = 0) = 0;

    virtual bool AddInventoryItem(const std::string& playerKey,
                                  int covenantId,
                                  const std::string& itemKey,
                                  int amount) = 0;

    virtual bool RecordActivity(const std::string& playerKey,
                                const std::string& activityKey,
                                int amount = 1) = 0;
};
}
}
