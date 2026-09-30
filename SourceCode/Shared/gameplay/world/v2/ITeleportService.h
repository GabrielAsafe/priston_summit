#pragma once

#include "gameplay/core/v2/GameplayIds.h"
#include "gameplay/world/v2/WorldPosition.h"

namespace gameplay
{
namespace world
{
enum class TeleportStatus
{
    Sent = 0,
    InvalidPlayer,
    InvalidDestination,
    Disconnected,
    TransportFailure
};

struct TeleportResult
{
    TeleportStatus status;

    explicit TeleportResult(TeleportStatus value = TeleportStatus::InvalidDestination)
        : status(value) {}

    bool Succeeded() const { return status == TeleportStatus::Sent; }
};

class ITeleportService
{
public:
    virtual ~ITeleportService() {}

    // Requests the legacy field transition using the server's fixed-point
    // destination coordinates. The historical wire command carries map/x/z;
    // y is intentionally not serialized.
    virtual TeleportResult Teleport(PlayerId player, const WorldPosition& destination) = 0;
};
}
}
