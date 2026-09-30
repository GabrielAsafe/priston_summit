#pragma once

#include <cstdint>
#include <string>

#include "gameplay/core/v2/GameplayIds.h"

namespace gameplay
{
namespace inventory
{
enum class PotionResource
{
    Life = 0,
    Mana = 1,
    Stamina = 2
};

enum class PotionUseCode
{
    Ok = 0,
    InvalidRequest,
    TargetNotFound,
    ConsumeRejected,
    RestoreInvalid
};

struct PotionUseRequest
{
    std::string accountName;
    std::string characterName;
    EntityId target;
    PotionResource resource;

    PotionUseRequest() : accountName(), characterName(), target(), resource(PotionResource::Life) {}
    PotionUseRequest(const std::string& account, const std::string& character,
                     EntityId targetValue, PotionResource resourceValue)
        : accountName(account), characterName(character), target(targetValue), resource(resourceValue) {}
};

struct PotionUseResult
{
    PotionUseCode code;
    bool consumed;
    int legacyStatus;
    int tier;
    int remainingCount;
    int restoreAmount;
    std::uint32_t itemCode;
    int beforeValue;
    int afterValue;
    int maxValue;

    PotionUseResult()
        : code(PotionUseCode::InvalidRequest), consumed(false), legacyStatus(0), tier(-1),
          remainingCount(0), restoreAmount(0), itemCode(0), beforeValue(0), afterValue(0), maxValue(0) {}
    bool Ok() const { return code == PotionUseCode::Ok && consumed; }
};

class IPotionService
{
public:
    virtual ~IPotionService() {}
    virtual PotionUseResult ConsumeAndApply(const PotionUseRequest& request) = 0;
};
}
}
