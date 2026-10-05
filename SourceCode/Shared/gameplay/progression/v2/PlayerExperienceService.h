#pragma once

#include <cstdint>

#include "gameplay/core/v2/GameplayIds.h"

namespace gameplay
{
namespace progression
{
using PlayerExperienceAmount = std::int64_t;

enum class PlayerExperienceSource
{
    MonsterSolo = 0,
    MonsterParty,
    ProgressionReward
};

enum class PlayerExperienceGrantCode
{
    Granted = 0,
    InvalidRequest,
    PlayerNotFound,
    LegacyRejected
};

struct PlayerExperienceGrantRequest
{
    PlayerId player;
    PlayerExperienceAmount amount;
    EntityId sourceEntity;
    bool party;
    PlayerExperienceSource source;

    PlayerExperienceGrantRequest(
        PlayerId playerValue = PlayerId(),
        PlayerExperienceAmount amountValue = 0,
        EntityId sourceEntityValue = EntityId(),
        bool partyValue = false,
        PlayerExperienceSource sourceValue = PlayerExperienceSource::ProgressionReward)
        : player(playerValue), amount(amountValue), sourceEntity(sourceEntityValue),
          party(partyValue), source(sourceValue) {}
};

struct PlayerExperienceGrantResult
{
    PlayerExperienceGrantCode code;
    PlayerId player;
    PlayerExperienceAmount amount;

    PlayerExperienceGrantResult()
        : code(PlayerExperienceGrantCode::LegacyRejected), player(), amount(0) {}

    bool Granted() const { return code == PlayerExperienceGrantCode::Granted; }
};

class IPlayerExperienceMutationPort
{
public:
    virtual ~IPlayerExperienceMutationPort() {}
    virtual PlayerExperienceGrantResult Grant(const PlayerExperienceGrantRequest& request) = 0;
};

class PlayerExperienceService final
{
public:
    static PlayerExperienceGrantResult Grant(
        IPlayerExperienceMutationPort& port,
        const PlayerExperienceGrantRequest& request)
    {
        if(!request.player.Valid() || request.amount <= 0)
        {
            PlayerExperienceGrantResult result;
            result.code = PlayerExperienceGrantCode::InvalidRequest;
            result.player = request.player;
            result.amount = request.amount;
            return result;
        }
        return port.Grant(request);
    }
};
}
}
