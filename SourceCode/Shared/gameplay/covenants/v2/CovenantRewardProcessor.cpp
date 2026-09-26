#include "gameplay/covenants/v2/CovenantRewardProcessor.h"

#include <sstream>

namespace gameplay
{
namespace covenants
{
CovenantRewardProcessor::CovenantRewardProcessor(ICovenantRepository& repository,
                                                   CovenantService& covenantService,
                                                   rewards::RewardService& rewardService)
    : repository_(repository), covenantService_(covenantService), rewardService_(rewardService)
{
}

std::string CovenantRewardProcessor::SourceKey(CovenantId covenantId,
                                                 const std::string& effectKey) const
{
    std::ostringstream source;
    source << "covenant:" << covenantId << ":reward:" << effectKey;
    return source.str();
}

bool CovenantRewardProcessor::ApplyReward(const std::string& playerKey,
                                           CovenantId covenantId,
                                           const CovenantRewardDefinition& definition)
{
    if (playerKey.empty() || covenantId <= 0 || definition.key.empty() ||
        definition.key.size() > kCovenantRewardKeyMaxLength || !rewardService_.Supports(definition.reward) ||
        !rewardService_.Validate(definition.reward))
        return false;

    const CovenantEffectClaimResult claim =
        repository_.ClaimEffect(playerKey, covenantId, definition.key);
    if (claim == CovenantEffectClaimResult::AlreadyApplied)
        return true;
    if (claim == CovenantEffectClaimResult::Pending && !rewardService_.IsRetrySafe(definition.reward))
        return false;
    if (claim != CovenantEffectClaimResult::Claimed && claim != CovenantEffectClaimResult::Pending)
        return false;

    if (rewardService_.Grant(playerKey, definition.reward,
                             SourceKey(covenantId, definition.key)) != rewards::RewardResult::Granted)
        return false;
    return repository_.MarkEffectApplied(playerKey, covenantId, definition.key);
}

bool CovenantRewardProcessor::ProcessMembership(const std::string& playerKey,
                                                  const CovenantMembershipState& membership)
{
    // Join/rank rewards belong to durable membership progress, not to the
    // currently active specialization. An inactive Covenant can still have
    // already-earned milestones that need idempotent delivery.
    if (playerKey.empty() || membership.covenantId <= 0)
        return false;

    std::vector<CovenantRewardDefinition> definitions;
    if (!covenantService_.GetEarnedRewards(membership.covenantId, membership.rankId, definitions))
        return false;

    for (std::vector<CovenantRewardDefinition>::const_iterator it = definitions.begin();
         it != definitions.end(); ++it)
        if (!ApplyReward(playerKey, membership.covenantId, *it))
            return false;
    return true;
}

bool CovenantRewardProcessor::ProcessPlayer(const std::string& playerKey)
{
    if (playerKey.empty())
        return false;

    std::vector<CovenantMembershipState> memberships;
    if (!repository_.LoadAll(playerKey, memberships))
        return false;
    for (std::vector<CovenantMembershipState>::const_iterator it = memberships.begin();
         it != memberships.end(); ++it)
    {
        if (!ProcessMembership(playerKey, *it))
            return false;
    }
    return true;
}

CovenantEffectReceiptLoadResult CovenantRewardProcessor::GetReceiptStatus(
    const std::string& playerKey,
    CovenantId covenantId,
    const std::string& effectKey,
    CovenantEffectReceiptStatus& status)
{
    return repository_.GetEffectReceiptStatus(playerKey, covenantId, effectKey, status);
}

bool CovenantRewardProcessor::ListPendingEffects(
    const std::string& playerKey,
    std::vector<CovenantEffectReceiptEntry>& receipts)
{
    return repository_.ListPendingEffects(playerKey, receipts);
}

bool CovenantRewardProcessor::AcknowledgePendingEffect(const std::string& playerKey,
                                                        CovenantId covenantId,
                                                        const std::string& effectKey)
{
    return repository_.AcknowledgePendingEffect(playerKey, covenantId, effectKey);
}
}
}
