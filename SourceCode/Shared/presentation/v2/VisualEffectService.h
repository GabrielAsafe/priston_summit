#pragma once

#include "presentation/v2/PresentationIds.h"
#include "presentation/v2/PresentationTarget.h"

namespace presentation
{

class IVisualEffectService
{
public:
    virtual ~IVisualEffectService() {}
    virtual bool PlayAt(const EffectId& effect, const PresentationTarget& target) = 0;
};
}
