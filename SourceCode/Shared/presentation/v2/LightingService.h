#pragma once

#include "presentation/v2/PresentationIds.h"
#include "presentation/v2/PresentationTarget.h"

namespace presentation
{
class ILightingService
{
public:
    virtual ~ILightingService() {}
    virtual bool PulseAt(const LightId& light, const PresentationTarget& target) = 0;
};
}
