#pragma once

#include <cstdint>
#include <vector>

#include "gameplay/core/v2/GameplayIds.h"

namespace gameplay
{
namespace commerce
{
enum class MixingOperation
{
    Reset
};

enum class MixingCode
{
    Processed,
    InvalidPlayer,
    InvalidPayload,
    InventoryReservationRejected,
    RollbackFailed,
    Rejected
};

struct MixingRequest
{
    PlayerId player;
    MixingOperation operation;
    std::vector<std::uint8_t> requestBytes;

    MixingRequest() : player(), operation(MixingOperation::Reset), requestBytes() {}
};

struct MixingResult
{
    MixingCode code;
    PlayerId player;
    MixingOperation operation;

    MixingResult() : code(MixingCode::InvalidPlayer), player(), operation(MixingOperation::Reset) {}
    bool Processed() const { return code == MixingCode::Processed; }
};

class IMixingService
{
public:
    virtual ~IMixingService() {}
    virtual MixingResult Process(const MixingRequest& request) = 0;
};
}
}
