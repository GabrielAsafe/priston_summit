#include "systems/gameplay/events/v2/legacy/LegacyWorldEventRewardGateway.h"

#include <cstring>

#include "systems/network/transport/legacy/smwsock.h"
#include "systems/rendering/legacy_3d/smd3d.h"
#include "systems/gameplay/character/core/character.h"
#include "core/server/legacy_runtime/onserver.h"

extern int spJobCode;
int CreateCommandItem(rsPLAYINFO* lpPlayInfo, rsPLAYINFO* lpPlayInfo2, char* szItem);

namespace gameplay
{
namespace events
{
namespace legacy
{
LegacyWorldEventRewardGateway::LegacyWorldEventRewardGateway(
    const player::legacy::LegacyOnlinePlayerRegistry& players)
    : players_(players)
{
}

bool LegacyWorldEventRewardGateway::Supports(rewards::RewardType type) const
{
    return type == rewards::RewardType::Item;
}

bool LegacyWorldEventRewardGateway::Validate(const rewards::RewardDefinition& reward) const
{
    return reward.type == rewards::RewardType::Item &&
           !reward.key.empty() && reward.amount == 1;
}

bool LegacyWorldEventRewardGateway::IsRetrySafe(rewards::RewardType) const
{
    // The underlying legacy command-item grant is not idempotent.
    return false;
}

rewards::RewardResult LegacyWorldEventRewardGateway::Grant(
    const std::string& playerKey,
    const rewards::RewardDefinition& reward,
    const std::string&)
{
    if(!Validate(reward))
        return Supports(reward.type) ? rewards::RewardResult::Rejected
                                     : rewards::RewardResult::Unsupported;

    rsPLAYINFO* player = players_.ResolveByPlayerKey(playerKey);
    if(!player || !player->lpsmSock)
        return rewards::RewardResult::Failed;

    char contentKey[64] = {};
    if(reward.key.size() >= sizeof(contentKey))
        return rewards::RewardResult::Rejected;
    std::memcpy(contentKey, reward.key.c_str(), reward.key.size());

    spJobCode = player->smCharInfo.JOB_CODE;
    const int granted = CreateCommandItem(player, player, contentKey);
    spJobCode = 0;

    return granted ? rewards::RewardResult::Granted : rewards::RewardResult::Failed;
}

rewards::RewardService& ServerWorldEventRewardService()
{
    static LegacyWorldEventRewardGateway gateway(
        player::legacy::ServerLegacyOnlinePlayerRegistry());
    static rewards::RewardService service(gateway);
    return service;
}
}
}
}
