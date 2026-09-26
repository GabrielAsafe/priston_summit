#pragma once

#include <string>
#include "systems/ui/v2/domain/CovenantUIModels.h"

namespace ui::v2
{
class ICovenantGateway
{
public:
    virtual ~ICovenantGateway() = default;
    [[nodiscard]] virtual const CovenantUiSnapshot& Snapshot() const = 0;
    virtual void SelectCovenant(int covenantId) = 0;
    virtual void RequestRefresh() = 0;
    virtual void Execute(CovenantUiAction action, int value = 0, const std::string& key = {}) = 0;
};
}
