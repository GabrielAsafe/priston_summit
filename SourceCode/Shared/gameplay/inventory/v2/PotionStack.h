#pragma once

#include <cstdint>

#include "gameplay/core/v2/GameplayIds.h"

namespace gameplay
{
namespace inventory
{
using PotionQuantity = std::int64_t;

struct PotionStackSnapshot
{
    PlayerId player;
    ItemId potion;
    PotionQuantity quantity;

    PotionStackSnapshot() : player(), potion(), quantity(0) {}
    PotionStackSnapshot(PlayerId playerValue, ItemId potionValue, PotionQuantity quantityValue)
        : player(playerValue), potion(potionValue), quantity(quantityValue) {}
};

enum class PotionStackMutationCode
{
    Applied,
    InvalidRequest,
    PlayerNotFound,
    InsufficientQuantity,
    CapacityExceeded,
    MutationRejected,
    RollbackFailed
};

struct PotionStackMutationRequest
{
    PlayerId player;
    ItemId potion;
    PotionQuantity quantity;

    PotionStackMutationRequest(PlayerId playerValue = PlayerId(),
                               ItemId potionValue = ItemId(),
                               PotionQuantity quantityValue = 0)
        : player(playerValue), potion(potionValue), quantity(quantityValue) {}
};

struct PotionStackMutationResult
{
    PotionStackMutationCode code;
    PotionStackMutationRequest request;
    PotionQuantity before;
    PotionQuantity after;

    PotionStackMutationResult()
        : code(PotionStackMutationCode::InvalidRequest), request(), before(0), after(0) {}

    bool Applied() const { return code == PotionStackMutationCode::Applied; }
};

enum class PotionStackTransferCode
{
    Applied,
    InvalidRequest,
    SourcePlayerNotFound,
    DestinationPlayerNotFound,
    InsufficientQuantity,
    DestinationCapacityExceeded,
    SourceMutationRejected,
    DestinationMutationRejected,
    RollbackFailed
};

struct PotionStackTransferRequest
{
    PlayerId source;
    PlayerId destination;
    ItemId potion;
    PotionQuantity quantity;

    PotionStackTransferRequest(PlayerId sourceValue = PlayerId(),
                               PlayerId destinationValue = PlayerId(),
                               ItemId potionValue = ItemId(),
                               PotionQuantity quantityValue = 0)
        : source(sourceValue), destination(destinationValue), potion(potionValue), quantity(quantityValue) {}
};

struct PotionStackTransferResult
{
    PotionStackTransferCode code;
    PotionStackTransferRequest request;
    PotionQuantity sourceBefore;
    PotionQuantity sourceAfter;
    PotionQuantity destinationBefore;
    PotionQuantity destinationAfter;

    PotionStackTransferResult()
        : code(PotionStackTransferCode::InvalidRequest), request(), sourceBefore(0), sourceAfter(0),
          destinationBefore(0), destinationAfter(0) {}

    bool Applied() const { return code == PotionStackTransferCode::Applied; }
};

class IPotionStackQuery
{
public:
    virtual ~IPotionStackQuery() {}
    virtual bool TryGetStack(PlayerId player, ItemId potion, PotionStackSnapshot& out) const = 0;
};

class IPotionStackCommands
{
public:
    virtual ~IPotionStackCommands() {}
    virtual PotionStackMutationResult AddStack(const PotionStackMutationRequest& request) = 0;
    virtual PotionStackMutationResult RemoveStack(const PotionStackMutationRequest& request) = 0;
    virtual PotionStackTransferResult TransferStack(const PotionStackTransferRequest& request) = 0;
};
}
}
