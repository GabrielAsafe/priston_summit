#pragma once

#include "presentation/v2/LightingService.h"

namespace presentation
{
namespace legacy
{
class LegacyLightingAdapter final : public ILightingService
{
public:
    bool PulseAt(const LightId& light, const PresentationTarget& target) override;
};

LegacyLightingAdapter& ClientLegacyLightingAdapter();
}
}
