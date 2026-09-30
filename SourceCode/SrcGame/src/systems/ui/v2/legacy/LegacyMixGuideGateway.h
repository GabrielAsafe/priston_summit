#pragma once
#include "systems/ui/v2/domain/IMixGuideGateway.h"
namespace ui::v2
{
class LegacyMixGuideGateway final : public IMixGuideGateway
{
public:
    [[nodiscard]] MixGuideSnapshot GetSnapshot() const override;
};
}
