#pragma once

#include "presentation/v2/AnimationService.h"

namespace presentation
{
namespace legacy
{
class LegacyAnimationAdapter final : public IAnimationService
{
public:
    bool Play(gameplay::EntityId entity, const AnimationId& animation) override;
};

LegacyAnimationAdapter& ClientLegacyAnimationAdapter();
}
}
