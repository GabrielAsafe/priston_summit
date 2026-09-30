#pragma once

#include "gameplay/rewards/v2/Reward.h"
#include "systems/gameplay/player/v2/legacy/LegacyOnlinePlayerRegistry.h"

namespace gameplay
{
namespace events
{
namespace legacy
{
class LegacyWorldEventRewardGateway final : public rewards::IRewardGateway
{
public:
    explicit LegacyWorldEventRewardGateway(
        const player::legacy::LegacyOnlinePlayerRegistry& players);

    bool Supports(rewards::RewardType type) const override;
    bool Validate(const rewards::RewardDefinition& reward) const override;
    bool IsRetrySafe(rewards::RewardType type) const override;
    rewards::RewardResult Grant(const std::string& playerKey,
                                const rewards::RewardDefinition& reward,
                                const std::string& sourceKey) override;

private:
    const player::legacy::LegacyOnlinePlayerRegistry& players_;
};

rewards::RewardService& ServerWorldEventRewardService();
}
}
}
