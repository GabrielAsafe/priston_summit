#pragma once

#include "gameplay/core/v2/GameplayIds.h"
#include "presentation/v2/PresentationIds.h"

namespace presentation
{
class IAnimationService
{
public:
    virtual ~IAnimationService() {}
    virtual bool Play(gameplay::EntityId entity, const AnimationId& animation) = 0;
};
}
