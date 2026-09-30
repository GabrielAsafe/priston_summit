#pragma once

#include <cstdint>

#include "gameplay/core/v2/GameplayIds.h"

namespace gameplay
{
namespace world
{
enum class InteractionStatus
{
    Unhandled = 0,
    Handled,
    InvalidPlayer,
    InvalidTarget
};

struct InteractionRequest
{
    PlayerId player;
    EntityId target;

    InteractionRequest() : player(), target() {}
    InteractionRequest(PlayerId playerValue, EntityId targetValue)
        : player(playerValue), target(targetValue) {}
};

struct InteractionResult
{
    InteractionStatus status;
    std::uint32_t eventCode;

    InteractionResult()
        : status(InteractionStatus::Unhandled), eventCode(0u) {}
    InteractionResult(InteractionStatus statusValue, std::uint32_t eventCodeValue)
        : status(statusValue), eventCode(eventCodeValue) {}

    bool Handled() const { return status == InteractionStatus::Handled; }
};

class IInteractionService
{
public:
    virtual ~IInteractionService() {}
    virtual InteractionResult Interact(const InteractionRequest& request) = 0;
};
}
}
