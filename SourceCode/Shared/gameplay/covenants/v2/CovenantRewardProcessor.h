#pragma once

#include <string>
#include <vector>

#include "gameplay/covenants/v2/CovenantService.h"
#include "gameplay/rewards/v2/Reward.h"

namespace gameplay
{
namespace covenants
{
class CovenantRewardProcessor
{
public:
    CovenantRewardProcessor(ICovenantRepository& repository,
                            CovenantService& covenantService,
                            rewards::RewardService& rewardService);

    bool ProcessPlayer(const std::string& playerKey);
    bool ProcessMembership(const std::string& playerKey,
                           const CovenantMembershipState& membership);

    CovenantEffectReceiptLoadResult GetReceiptStatus(
        const std::string& playerKey,
        CovenantId covenantId,
        const std::string& effectKey,
        CovenantEffectReceiptStatus& status);
    bool ListPendingEffects(const std::string& playerKey,
                            std::vector<CovenantEffectReceiptEntry>& receipts);
    bool AcknowledgePendingEffect(const std::string& playerKey,
                                  CovenantId covenantId,
                                  const std::string& effectKey);

private:
    bool ApplyReward(const std::string& playerKey,
                     CovenantId covenantId,
                     const CovenantRewardDefinition& definition);
    std::string SourceKey(CovenantId covenantId,
                          const std::string& effectKey) const;

    ICovenantRepository& repository_;
    CovenantService& covenantService_;
    rewards::RewardService& rewardService_;
};
}
}
