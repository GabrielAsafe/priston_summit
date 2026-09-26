#pragma once

#include "systems/ui/v2/domain/CovenantUIModels.h"

namespace ui::v2
{
// Persistent HUD adapter for the active Covenant. It intentionally reuses the
// already-loaded premium/upkeep icon materials (the same 32x32 path used by
// Phoenix and other upkeep buffs) without inserting synthetic premium items.
class CovenantEffectHud
{
public:
    void Render(const CovenantUiSnapshot& snapshot) const;
};
}
