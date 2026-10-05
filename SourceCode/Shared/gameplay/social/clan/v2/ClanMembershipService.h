#pragma once

#include <cstdint>

#include "gameplay/core/v2/GameplayIds.h"

namespace gameplay
{
namespace social
{
namespace clan
{
enum class ClanMembershipMutationCode
{
    Applied = 0,
    InvalidRequest,
    PlayerNotFound,
    LegacyRejected
};

struct ClanMembershipMutationRequest
{
    PlayerId player;
    std::int32_t mark;

    ClanMembershipMutationRequest(PlayerId playerValue = PlayerId(), std::int32_t markValue = 0)
        : player(playerValue), mark(markValue) {}
};

struct ClanMembershipMutationResult
{
    ClanMembershipMutationCode code;
    PlayerId player;
    std::int32_t mark;

    ClanMembershipMutationResult()
        : code(ClanMembershipMutationCode::LegacyRejected), player(), mark(0) {}

    bool Applied() const { return code == ClanMembershipMutationCode::Applied; }
};

class IClanMembershipMutationPort
{
public:
    virtual ~IClanMembershipMutationPort() {}
    virtual ClanMembershipMutationResult SynchronizeMark(
        const ClanMembershipMutationRequest& request) = 0;
};

class ClanMembershipService final
{
public:
    static ClanMembershipMutationResult SynchronizeMark(
        IClanMembershipMutationPort& port,
        const ClanMembershipMutationRequest& request)
    {
        if(!request.player.Valid() || request.mark < 0)
        {
            ClanMembershipMutationResult result;
            result.code = ClanMembershipMutationCode::InvalidRequest;
            result.player = request.player;
            result.mark = request.mark;
            return result;
        }
        return port.SynchronizeMark(request);
    }
};
}
}
}
