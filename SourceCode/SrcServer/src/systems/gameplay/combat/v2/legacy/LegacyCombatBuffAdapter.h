#pragma once

#include "gameplay/combat/v2/CombatBuffService.h"

namespace gameplay
{
namespace combat
{
namespace legacy
{
class LegacyCombatBuffAdapter final : public ICombatBuffService
{
public:
    CombatBuffResult Apply(EntityId target, const CombatBuffDelta& delta) override;
    CombatBuffResult Remove(EntityId target, const CombatBuffDelta& delta) override;
};

LegacyCombatBuffAdapter& ServerLegacyCombatBuffAdapter();
}
}
}
