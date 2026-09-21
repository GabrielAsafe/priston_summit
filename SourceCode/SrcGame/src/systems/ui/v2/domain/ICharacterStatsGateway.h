#pragma once

#include "systems/ui/v2/domain/CharacterUIModels.h"

namespace ui::v2
{
class ICharacterStatsGateway
{
public:
    virtual ~ICharacterStatsGateway() = default;
    [[nodiscard]] virtual CharacterStatsSnapshot GetSnapshot() const = 0;
    virtual bool AddAttribute(CharacterAttribute attribute, int amount) = 0;
    virtual bool UndoLastAttributePoint() = 0;
};
}
