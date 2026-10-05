#pragma once

#include "gameplay/combat/v2/LifeCommands.h"

namespace gameplay
{
namespace combat
{
namespace legacy
{
class LegacyLifeAdapter final : public ILifeCommands
{
public:
    LifeCommandResult Restore(EntityId target, int amount) override;
    LifeCommandResult Restore(PlayerId target, int amount) override;
    LifeCommandResult ConsumeNonLethal(EntityId target, int amount) override;
    LifeCommandResult ConsumeLethal(EntityId target, int amount) override;
    LifeCommandResult Deplete(EntityId target) override;
    LifeCommandResult EnsureMinimum(EntityId target, int minimumLife) override;
    LifeCommandResult ClampToValidRange(EntityId target) override;
    LifeCommandResult RestoreToMaximum(EntityId target) override;
    LifeCommandResult Revive(EntityId target, int percentOfMaximum) override;
};

LegacyLifeAdapter& ServerLegacyLifeAdapter();
}
}
}
