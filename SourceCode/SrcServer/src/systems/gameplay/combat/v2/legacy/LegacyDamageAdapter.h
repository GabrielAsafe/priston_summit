#pragma once

#include "gameplay/combat/v2/DamageContracts.h"

namespace gameplay
{
namespace combat
{
namespace legacy
{
class LegacyDamageAdapter final : public IDamageService
{
public:
    DamageResult Apply(const DamageRequest& request) override;
};

LegacyDamageAdapter& ServerLegacyDamageAdapter();
}
}
}
