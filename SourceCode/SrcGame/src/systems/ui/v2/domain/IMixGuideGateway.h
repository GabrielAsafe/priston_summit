#pragma once
#include "systems/ui/v2/domain/MixGuideModels.h"
namespace ui::v2
{
class IMixGuideGateway
{
public:
    virtual ~IMixGuideGateway() = default;
    [[nodiscard]] virtual MixGuideSnapshot GetSnapshot() const = 0;
};
}
