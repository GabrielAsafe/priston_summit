#pragma once

#include "gameplay/core/v2/GameplayIds.h"

namespace gameplay
{
namespace monsters
{
enum class MonsterCommandCode
{
    Ok = 0,
    InvalidRequest,
    OwnerNotFound,
    TargetNotFound,
    TargetInactive,
    TargetDead,
    InvalidState
};

struct FollowerConversionRequest
{
    EntityId target;
    PlayerId owner;
    int durationTicks;
    int lifePercent;

    FollowerConversionRequest(EntityId targetValue = EntityId(),
                              PlayerId ownerValue = PlayerId(),
                              int durationTicksValue = 0,
                              int lifePercentValue = 0)
        : target(targetValue), owner(ownerValue), durationTicks(durationTicksValue),
          lifePercent(lifePercentValue) {}
};

struct FollowerConversionResult
{
    MonsterCommandCode code;
    int previousBrood;
    int previousLife;
    int convertedLife;
    int durationTicks;
    bool converted;

    FollowerConversionResult()
        : code(MonsterCommandCode::InvalidRequest), previousBrood(0), previousLife(0),
          convertedLife(0), durationTicks(0), converted(false) {}

    bool Ok() const { return code == MonsterCommandCode::Ok && converted; }
};


struct FollowerReleaseRequest
{
    EntityId target;

    explicit FollowerReleaseRequest(EntityId targetValue = EntityId()) : target(targetValue) {}
};

struct FollowerReleaseResult
{
    MonsterCommandCode code;
    bool released;

    FollowerReleaseResult() : code(MonsterCommandCode::InvalidRequest), released(false) {}
    bool Ok() const { return code == MonsterCommandCode::Ok && released; }
};

class IMonsterCommands
{
public:
    virtual ~IMonsterCommands() {}
    virtual FollowerConversionResult ConvertToFollower(const FollowerConversionRequest& request) = 0;
    virtual FollowerReleaseResult ReleaseFollower(const FollowerReleaseRequest& request) = 0;
};
}
}
