#pragma once

#include "presentation/v2/VisualEffectService.h"

namespace presentation
{
namespace legacy
{
class LegacyVisualEffectAdapter final : public IVisualEffectService
{
public:
    bool PlayAt(const EffectId& effect, const PresentationTarget& target) override;
};

LegacyVisualEffectAdapter& ClientLegacyVisualEffectAdapter();
}
}
