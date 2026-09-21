#pragma once

#include "systems/ui/v2/domain/ICharacterStatsGateway.h"

namespace ui::v2::legacy
{
class LegacyCharacterStatsGateway final : public ICharacterStatsGateway
{
public:
    [[nodiscard]] CharacterStatsSnapshot GetSnapshot() const override;
    bool AddAttribute(CharacterAttribute attribute, int amount) override;
    bool UndoLastAttributePoint() override;

private:
    static void Recalculate();
};
}
