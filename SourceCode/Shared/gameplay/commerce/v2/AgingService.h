#pragma once

#include <cstdint>
#include <vector>

#include "gameplay/core/v2/GameplayIds.h"

namespace gameplay
{
namespace commerce
{
enum class AgingOperation
{
    Apply,
    Upgrade
};

enum class AgingCode
{
    Processed,
    InvalidPlayer,
    InvalidPayload,
    Rejected
};

struct AgingRequest
{
    PlayerId player;
    AgingOperation operation;
    std::vector<std::uint8_t> requestBytes;

    AgingRequest() : player(), operation(AgingOperation::Apply), requestBytes() {}
};

struct AgingResult
{
    AgingCode code;
    PlayerId player;
    AgingOperation operation;

    AgingResult() : code(AgingCode::InvalidPlayer), player(), operation(AgingOperation::Apply) {}
    bool Processed() const { return code == AgingCode::Processed; }
};

class IAgingService
{
public:
    virtual ~IAgingService() {}
    virtual AgingResult Process(const AgingRequest& request) = 0;
};
}
}
