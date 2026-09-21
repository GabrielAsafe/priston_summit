#pragma once

#include "systems/ui/v2/domain/ICharacterStatsGateway.h"

namespace ui::v2
{
class AttributesViewModel
{
public:
    explicit AttributesViewModel(ICharacterStatsGateway& gateway) : gateway_(gateway) {}

    void Refresh() { snapshot_ = gateway_.GetSnapshot(); }
    [[nodiscard]] const CharacterStatsSnapshot& Snapshot() const noexcept { return snapshot_; }

    bool Add(CharacterAttribute attribute, int amount = 1)
    {
        if (!gateway_.AddAttribute(attribute, amount))
            return false;
        Refresh();
        return true;
    }

    bool Undo()
    {
        if (!gateway_.UndoLastAttributePoint())
            return false;
        Refresh();
        return true;
    }

private:
    ICharacterStatsGateway& gateway_;
    CharacterStatsSnapshot snapshot_{};
};
}
