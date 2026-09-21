#pragma once

#include <string>
#include "systems/ui/v2/domain/CompanionUIModels.h"

namespace ui::v2
{
class ICompanionGateway
{
public:
    virtual ~ICompanionGateway() = default;
    virtual const CompanionUiSnapshot& Snapshot() const = 0;
    virtual const CompanionRuntimeSnapshot& RuntimeSnapshot() const = 0;
    virtual void RequestRefresh() = 0;
    virtual void Execute(CompanionUiAction action, const std::string& characterName = {}, int value = 0) = 0;
};
}
