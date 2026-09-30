#pragma once

#include "gameplay/inventory/v2/PotionService.h"

namespace gameplay
{
namespace inventory
{
namespace legacy
{
class LegacyPotionAdapter final : public IPotionService
{
public:
    PotionUseResult ConsumeAndApply(const PotionUseRequest& request) override;
};

LegacyPotionAdapter& ServerLegacyPotionAdapter();
}
}
}
