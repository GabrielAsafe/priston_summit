#pragma once

#include "gameplay/world/v2/IInteractionService.h"

namespace gameplay
{
namespace world
{
namespace legacy
{
class LegacyInteractionAdapter final : public IInteractionService
{
public:
    InteractionResult Interact(const InteractionRequest& request) override;
};

LegacyInteractionAdapter& ServerLegacyInteractionAdapter();
}
}
}
