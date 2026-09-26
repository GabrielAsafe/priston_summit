#include "gameplay/covenants/v2/CovenantService.h"

#include <algorithm>
#include <climits>
#include <cmath>
#include <set>

namespace gameplay
{
namespace covenants
{
namespace
{
std::string BenefitSourceKey(const CovenantDefinition& covenant,
                             const CovenantRankDefinition& rank)
{
    return std::string("covenant:") + covenant.key + ":rank:" + rank.key;
}

std::vector<modifiers::GameplayModifier> BenefitsWithSource(
    const CovenantDefinition& covenant,
    const CovenantRankDefinition& rank)
{
    std::vector<modifiers::GameplayModifier> result = rank.benefits;
    const std::string source = BenefitSourceKey(covenant, rank);
    for (std::vector<modifiers::GameplayModifier>::iterator it = result.begin();
         it != result.end(); ++it)
        it->sourceKey = source;
    return result;
}
}

CovenantService::CovenantService(ICovenantRepository& repository,
                                 events::GameplayEventHub* eventHub)
    : repository_(repository), eventHub_(eventHub)
{
}

bool CovenantService::RegisterCovenant(const CovenantDefinition& definition)
{
    if (definition.id <= 0 || definition.key.empty() || definition.key.size() > 255u ||
        definition.displayNameKey.size() > 255u)
        return false;
    if (definition.hasReputationBounds && definition.minReputation > definition.maxReputation)
        return false;
    for (DefinitionMap::const_iterator it = definitions_.begin(); it != definitions_.end(); ++it)
        if (it->second.key == definition.key)
            return false;
    return definitions_.insert(std::make_pair(definition.id, definition)).second;
}

bool CovenantService::RegisterRank(const CovenantRankDefinition& definition)
{
    if (definition.id <= 0 || definition.covenantId <= 0 || definition.key.empty() ||
        definition.key.size() > 255u || definition.displayNameKey.size() > 255u)
        return false;
    if (!FindDefinition(definition.covenantId))
        return false;

    std::vector<CovenantRankDefinition>& rankList = ranks_[definition.covenantId];
    for (std::vector<CovenantRankDefinition>::const_iterator it = rankList.begin();
         it != rankList.end(); ++it)
    {
        if (it->id == definition.id || it->key == definition.key ||
            it->minReputation == definition.minReputation)
            return false;
    }
    rankList.push_back(definition);
    std::sort(rankList.begin(), rankList.end(),
              [](const CovenantRankDefinition& left, const CovenantRankDefinition& right)
              {
                  if (left.minReputation != right.minReputation)
                      return left.minReputation < right.minReputation;
                  return left.id < right.id;
              });
    return true;
}


bool CovenantService::ValidateDefinitions(std::string* error) const
{
    for (DefinitionMap::const_iterator definitionIt = definitions_.begin();
         definitionIt != definitions_.end(); ++definitionIt)
    {
        const CovenantDefinition& definition = definitionIt->second;
        if (!conditions::ConditionService::ValidateAll(definition.joinConditions))
        {
            if (error) *error = "covenant contains an invalid join condition";
            return false;
        }
        std::set<int> relatedQuestIds;
        for (std::vector<int>::const_iterator questId = definition.relatedQuestIds.begin();
             questId != definition.relatedQuestIds.end(); ++questId)
        {
            if (*questId <= 0 || !relatedQuestIds.insert(*questId).second)
            {
                if (error) *error = "covenant related quest ids must be positive and unique";
                return false;
            }
        }
        std::set<int> relatedStoryIds;
        for (std::vector<int>::const_iterator storyId = definition.relatedStoryIds.begin();
             storyId != definition.relatedStoryIds.end(); ++storyId)
        {
            if (*storyId <= 0 || !relatedStoryIds.insert(*storyId).second)
            {
                if (error) *error = "covenant related story ids must be positive and unique";
                return false;
            }
        }
        std::set<std::string> rewardKeys;
        for (std::vector<CovenantRewardDefinition>::const_iterator rewardIt = definition.joinRewards.begin();
             rewardIt != definition.joinRewards.end(); ++rewardIt)
        {
            if (rewardIt->key.empty() || rewardIt->key.size() > kCovenantRewardKeyMaxLength ||
                !rewardKeys.insert(rewardIt->key).second)
            {
                if (error) *error = "covenant reward keys must be non-empty, within source-key limits and unique";
                return false;
            }
        }
        RankMap::const_iterator rankListIt = ranks_.find(definition.id);
        if (rankListIt == ranks_.end() || rankListIt->second.empty())
        {
            if (error) *error = "covenant has no ranks";
            return false;
        }
        int previousRankId = 0;
        bool havePreviousRank = false;
        const CovenantRankDefinition* previousRank = 0;
        for (std::vector<CovenantRankDefinition>::const_iterator rankIt = rankListIt->second.begin();
             rankIt != rankListIt->second.end(); ++rankIt)
        {
            if (havePreviousRank && rankIt->id <= previousRankId)
            {
                if (error) *error = "covenant rank ids must increase with reputation thresholds";
                return false;
            }
            previousRankId = rankIt->id;
            havePreviousRank = true;
            if (definition.hasReputationBounds &&
                (rankIt->minReputation < definition.minReputation ||
                 rankIt->minReputation > definition.maxReputation))
            {
                if (error) *error = "rank threshold is outside covenant reputation bounds";
                return false;
            }
            std::set<std::string> benefitKeys;
            for (std::vector<modifiers::GameplayModifier>::const_iterator benefit = rankIt->benefits.begin();
                 benefit != rankIt->benefits.end(); ++benefit)
            {
                const int operation = static_cast<int>(benefit->operation);
                if (benefit->statKey.empty() || benefit->statKey.size() > 255u ||
                    benefit->scopeKey.size() > 255u || operation < 0 || operation > 2 ||
                    !std::isfinite(benefit->value))
                {
                    if (error) *error = "covenant rank contains an invalid gameplay modifier";
                    return false;
                }
                const std::string benefitKey = benefit->statKey + "\n" + benefit->scopeKey +
                                               "\n" + std::to_string(operation);
                if (!benefitKeys.insert(benefitKey).second)
                {
                    if (error) *error = "covenant rank contains a duplicate gameplay modifier";
                    return false;
                }
            }
            // Covenant content uses cumulative ranks (Option A): every effect key present in
            // a lower rank must remain declared in the next rank. Values may grow/shrink, but
            // the semantic stat/scope/operation contract cannot silently disappear.
            if (previousRank)
            {
                for (std::vector<modifiers::GameplayModifier>::const_iterator previousBenefit =
                         previousRank->benefits.begin();
                     previousBenefit != previousRank->benefits.end(); ++previousBenefit)
                {
                    bool carriedForward = false;
                    for (std::vector<modifiers::GameplayModifier>::const_iterator currentBenefit =
                             rankIt->benefits.begin();
                         currentBenefit != rankIt->benefits.end(); ++currentBenefit)
                    {
                        if (currentBenefit->statKey == previousBenefit->statKey &&
                            currentBenefit->scopeKey == previousBenefit->scopeKey &&
                            currentBenefit->operation == previousBenefit->operation)
                        {
                            carriedForward = true;
                            break;
                        }
                    }
                    if (!carriedForward)
                    {
                        if (error) *error = "covenant cumulative rank benefits must carry forward prior effects";
                        return false;
                    }
                }
            }
            previousRank = &*rankIt;
            for (std::vector<CovenantRewardDefinition>::const_iterator rewardIt = rankIt->entryRewards.begin();
                 rewardIt != rankIt->entryRewards.end(); ++rewardIt)
            {
                if (rewardIt->key.empty() || rewardIt->key.size() > kCovenantRewardKeyMaxLength ||
                    !rewardKeys.insert(rewardIt->key).second)
                {
                    if (error) *error = "covenant reward keys must be non-empty, within source-key limits and unique";
                    return false;
                }
            }
        }
    }
    return true;
}

bool CovenantService::ValidateRewards(const rewards::RewardService& rewardService,
                                        std::string* error) const
{
    for (DefinitionMap::const_iterator definitionIt = definitions_.begin();
         definitionIt != definitions_.end(); ++definitionIt)
    {
        const CovenantDefinition& definition = definitionIt->second;
        for (std::vector<CovenantRewardDefinition>::const_iterator rewardIt = definition.joinRewards.begin();
             rewardIt != definition.joinRewards.end(); ++rewardIt)
        {
            if (!rewardService.Supports(rewardIt->reward) || !rewardService.Validate(rewardIt->reward))
            {
                if (error) *error = "covenant contains an unsupported or malformed join reward";
                return false;
            }
        }
        RankMap::const_iterator rankListIt = ranks_.find(definition.id);
        if (rankListIt == ranks_.end())
            continue;
        for (std::vector<CovenantRankDefinition>::const_iterator rankIt = rankListIt->second.begin();
             rankIt != rankListIt->second.end(); ++rankIt)
        {
            for (std::vector<CovenantRewardDefinition>::const_iterator rewardIt = rankIt->entryRewards.begin();
                 rewardIt != rankIt->entryRewards.end(); ++rewardIt)
            {
                if (!rewardService.Supports(rewardIt->reward) || !rewardService.Validate(rewardIt->reward))
                {
                    if (error) *error = "covenant contains an unsupported or malformed rank reward";
                    return false;
                }
            }
        }
    }
    return true;
}

bool CovenantService::LoadMemberships(const std::string& playerKey,
                                      std::vector<CovenantMembershipState>& memberships)
{
    memberships.clear();
    if (playerKey.empty())
        return false;
    return repository_.LoadAll(playerKey, memberships);
}

const CovenantDefinition* CovenantService::FindDefinition(CovenantId covenantId) const
{
    DefinitionMap::const_iterator it = definitions_.find(covenantId);
    return it == definitions_.end() ? 0 : &it->second;
}

const CovenantRankDefinition* CovenantService::FindRank(CovenantId covenantId,
                                                        CovenantRankId rankId) const
{
    RankMap::const_iterator listIt = ranks_.find(covenantId);
    if (listIt == ranks_.end())
        return 0;
    for (std::vector<CovenantRankDefinition>::const_iterator it = listIt->second.begin();
         it != listIt->second.end(); ++it)
    {
        if (it->id == rankId)
            return &*it;
    }
    return 0;
}

CovenantRankId CovenantService::CalculateRank(CovenantId covenantId, int reputation) const
{
    RankMap::const_iterator listIt = ranks_.find(covenantId);
    if (listIt == ranks_.end())
        return 0;

    CovenantRankId result = 0;
    for (std::vector<CovenantRankDefinition>::const_iterator it = listIt->second.begin();
         it != listIt->second.end(); ++it)
    {
        if (reputation >= it->minReputation)
            result = it->id;
        else
            break;
    }
    return result;
}

int CovenantService::ClampReputation(const CovenantDefinition& definition, long long value) const
{
    long long minimum = INT_MIN;
    long long maximum = INT_MAX;
    if (definition.hasReputationBounds)
    {
        minimum = definition.minReputation;
        maximum = definition.maxReputation;
    }
    if (value < minimum)
        value = minimum;
    if (value > maximum)
        value = maximum;
    return static_cast<int>(value);
}

CovenantJoinEligibility CovenantService::CheckJoinEligibility(
    const std::string& playerKey,
    CovenantId covenantId,
    const conditions::ConditionContext& context)
{
    if (playerKey.empty() || covenantId <= 0)
        return CovenantJoinEligibility::InvalidRequest;

    const CovenantDefinition* definition = FindDefinition(covenantId);
    if (!definition)
        return CovenantJoinEligibility::UnknownCovenant;
    if (!conditions::ConditionService::EvaluateAll(definition->joinConditions, context))
        return CovenantJoinEligibility::ConditionsNotMet;

    std::vector<CovenantMembershipState> memberships;
    if (!LoadMemberships(playerKey, memberships))
        return CovenantJoinEligibility::PersistenceFailure;

    int activeCount = 0;
    for (std::vector<CovenantMembershipState>::const_iterator it = memberships.begin();
         it != memberships.end(); ++it)
    {
        if (it->active)
            ++activeCount;
        if (it->covenantId == covenantId)
            return it->active ? CovenantJoinEligibility::AlreadyActive
                              : CovenantJoinEligibility::AlreadyMember;
        if (!FindDefinition(it->covenantId))
            return CovenantJoinEligibility::PersistenceFailure;
    }

    // A corrupted row-set with more than one active Covenant must fail closed.
    if (activeCount > 1)
        return CovenantJoinEligibility::PersistenceFailure;
    return CovenantJoinEligibility::Eligible;
}

bool CovenantService::CanJoin(const std::string& playerKey,
                              CovenantId covenantId,
                              const conditions::ConditionContext& context)
{
    return CheckJoinEligibility(playerKey, covenantId, context) ==
           CovenantJoinEligibility::Eligible;
}

bool CovenantService::Join(const std::string& playerKey,
                           CovenantId covenantId,
                           const conditions::ConditionContext& context)
{
    if (!CanJoin(playerKey, covenantId, context))
        return false;

    const CovenantDefinition* definition = FindDefinition(covenantId);
    if (!definition)
        return false;

    std::vector<CovenantMembershipState> memberships;
    if (!LoadMemberships(playerKey, memberships))
        return false;

    bool hasActiveMembership = false;
    for (std::vector<CovenantMembershipState>::const_iterator it = memberships.begin();
         it != memberships.end(); ++it)
    {
        if (it->active)
        {
            if (hasActiveMembership)
                return false;
            hasActiveMembership = true;
        }
    }

    CovenantMembershipState membership;
    membership.covenantId = covenantId;
    membership.reputation = 0;
    membership.rankId = CalculateRank(covenantId, membership.reputation);
    // The first Covenant joined becomes active automatically. Further memberships
    // preserve the current specialization until Activate() is explicitly requested.
    membership.active = !hasActiveMembership;

    std::vector<CovenantId> knownCovenantIds;
    std::vector<CovenantId> exclusiveCovenantIds;
    for (DefinitionMap::const_iterator it = definitions_.begin(); it != definitions_.end(); ++it)
    {
        knownCovenantIds.push_back(it->first);
        if (it->second.exclusiveMembership)
            exclusiveCovenantIds.push_back(it->first);
    }
    if (!repository_.TryJoin(playerKey, membership, definition->exclusiveMembership,
                             knownCovenantIds, exclusiveCovenantIds))
        return false;

    if (eventHub_)
    {
        CovenantJoinedEvent event;
        event.playerKey = playerKey;
        event.covenantId = covenantId;
        eventHub_->Publish(event);

        if (membership.active)
        {
            CovenantActivatedEvent activated;
            activated.playerKey = playerKey;
            activated.covenantId = covenantId;
            activated.previousCovenantId = 0;
            eventHub_->Publish(activated);
        }
    }
    return true;
}

CovenantActivationResult CovenantService::Activate(const std::string& playerKey,
                                                    CovenantId covenantId)
{
    if (playerKey.empty() || covenantId <= 0)
        return CovenantActivationResult::InvalidRequest;
    if (!FindDefinition(covenantId))
        return CovenantActivationResult::UnknownCovenant;

    std::vector<CovenantMembershipState> memberships;
    if (!LoadMemberships(playerKey, memberships))
        return CovenantActivationResult::PersistenceFailure;

    bool foundTarget = false;
    CovenantId previousCovenantId = 0;
    int activeCount = 0;
    for (std::vector<CovenantMembershipState>::const_iterator it = memberships.begin();
         it != memberships.end(); ++it)
    {
        if (!FindDefinition(it->covenantId))
            return CovenantActivationResult::PersistenceFailure;
        if (it->covenantId == covenantId)
            foundTarget = true;
        if (it->active)
        {
            ++activeCount;
            previousCovenantId = it->covenantId;
        }
    }
    if (!foundTarget)
        return CovenantActivationResult::NotMember;
    if (activeCount > 1)
        return CovenantActivationResult::PersistenceFailure;
    if (previousCovenantId == covenantId)
        return CovenantActivationResult::AlreadyActive;

    std::vector<CovenantId> knownCovenantIds;
    for (DefinitionMap::const_iterator it = definitions_.begin(); it != definitions_.end(); ++it)
        knownCovenantIds.push_back(it->first);

    if (!repository_.SwitchActive(playerKey, covenantId, knownCovenantIds))
        return CovenantActivationResult::PersistenceFailure;

    if (eventHub_)
    {
        CovenantActivatedEvent activated;
        activated.playerKey = playerKey;
        activated.covenantId = covenantId;
        activated.previousCovenantId = previousCovenantId;
        eventHub_->Publish(activated);
    }
    return CovenantActivationResult::Activated;
}

bool CovenantService::Leave(const std::string& playerKey, CovenantId covenantId)
{
    const int maxAttempts = 4;
    for (int attempt = 0; attempt < maxAttempts; ++attempt)
    {
        CovenantMembershipState expected;
        if (!GetMembership(playerKey, covenantId, expected) || !expected.active)
            return false;

        CovenantMembershipState membership = expected;
        membership.active = false;
        if (!repository_.SaveIfUnchanged(playerKey, expected, membership))
            continue;

        if (eventHub_)
        {
            CovenantLeftEvent event;
            event.playerKey = playerKey;
            event.covenantId = covenantId;
            eventHub_->Publish(event);
        }
        return true;
    }
    return false;
}

bool CovenantService::AddReputation(const std::string& playerKey,
                                    CovenantId covenantId,
                                    int amount,
                                    const std::string& sourceKey)
{
    const CovenantDefinition* definition = FindDefinition(covenantId);
    if (!definition || playerKey.empty() || playerKey.size() > 255u ||
        sourceKey.empty() || sourceKey.size() > 255u || amount == 0)
        return false;

    const int maxAttempts = 4;
    for (int attempt = 0; attempt < maxAttempts; ++attempt)
    {
        CovenantMembershipState expected;
        if (!GetMembership(playerKey, covenantId, expected) || !expected.active)
            return false;

        CovenantMembershipState membership = expected;
        membership.reputation = ClampReputation(*definition,
            static_cast<long long>(expected.reputation) + static_cast<long long>(amount));
        membership.rankId = CalculateRank(covenantId, membership.reputation);

        const CovenantReputationSaveResult saveResult =
            repository_.SaveReputationChangeChecked(playerKey, expected, membership, amount, sourceKey);
        if (saveResult == CovenantReputationSaveResult::Conflict)
            continue;
        if (saveResult == CovenantReputationSaveResult::Failed)
            return false;
        if (saveResult == CovenantReputationSaveResult::AlreadyApplied)
            return true;

        if (eventHub_ && expected.reputation != membership.reputation)
        {
            CovenantReputationChangedEvent reputationEvent;
            reputationEvent.playerKey = playerKey;
            reputationEvent.covenantId = covenantId;
            reputationEvent.oldReputation = expected.reputation;
            reputationEvent.newReputation = membership.reputation;
            reputationEvent.sourceKey = sourceKey;
            eventHub_->Publish(reputationEvent);
        }
        if (eventHub_ && expected.rankId != membership.rankId)
        {
            CovenantRankChangedEvent rankEvent;
            rankEvent.playerKey = playerKey;
            rankEvent.covenantId = covenantId;
            rankEvent.oldRankId = expected.rankId;
            rankEvent.newRankId = membership.rankId;
            eventHub_->Publish(rankEvent);
        }
        return true;
    }
    return false;
}

bool CovenantService::AddRepeatableReputation(const std::string& playerKey,
                                               CovenantId covenantId,
                                               int amount,
                                               const std::string& activityKey)
{
    const CovenantDefinition* definition = FindDefinition(covenantId);
    if (!definition || playerKey.empty() || playerKey.size() > 255u ||
        activityKey.empty() || activityKey.size() > 255u || amount == 0)
        return false;

    const int maxAttempts = 4;
    for (int attempt = 0; attempt < maxAttempts; ++attempt)
    {
        CovenantMembershipState expected;
        if (!GetMembership(playerKey, covenantId, expected) || !expected.active)
            return false;

        CovenantMembershipState membership = expected;
        membership.reputation = ClampReputation(*definition,
            static_cast<long long>(expected.reputation) + static_cast<long long>(amount));
        membership.rankId = CalculateRank(covenantId, membership.reputation);

        const CovenantReputationSaveResult saveResult =
            repository_.SaveRepeatableReputationChangeChecked(playerKey, expected, membership, amount);
        if (saveResult == CovenantReputationSaveResult::Conflict)
            continue;
        if (saveResult != CovenantReputationSaveResult::Applied)
            return false;

        if (eventHub_ && expected.reputation != membership.reputation)
        {
            CovenantReputationChangedEvent reputationEvent;
            reputationEvent.playerKey = playerKey;
            reputationEvent.covenantId = covenantId;
            reputationEvent.oldReputation = expected.reputation;
            reputationEvent.newReputation = membership.reputation;
            reputationEvent.sourceKey = std::string("activity:") + activityKey;
            eventHub_->Publish(reputationEvent);
        }
        if (eventHub_ && expected.rankId != membership.rankId)
        {
            CovenantRankChangedEvent rankEvent;
            rankEvent.playerKey = playerKey;
            rankEvent.covenantId = covenantId;
            rankEvent.oldRankId = expected.rankId;
            rankEvent.newRankId = membership.rankId;
            eventHub_->Publish(rankEvent);
        }
        return true;
    }
    return false;
}

bool CovenantService::ReevaluateRank(const std::string& playerKey, CovenantId covenantId)
{
    const CovenantDefinition* definition = FindDefinition(covenantId);
    if (!definition)
        return false;

    const int maxAttempts = 4;
    for (int attempt = 0; attempt < maxAttempts; ++attempt)
    {
        CovenantMembershipState expected;
        if (!GetMembership(playerKey, covenantId, expected) || !expected.active)
            return false;

        CovenantMembershipState membership = expected;
        membership.reputation = ClampReputation(*definition, expected.reputation);
        membership.rankId = CalculateRank(covenantId, membership.reputation);
        if (membership.rankId == expected.rankId && membership.reputation == expected.reputation)
            return true;
        if (!repository_.SaveIfUnchanged(playerKey, expected, membership))
            continue;

        if (eventHub_ && expected.rankId != membership.rankId)
        {
            CovenantRankChangedEvent rankEvent;
            rankEvent.playerKey = playerKey;
            rankEvent.covenantId = covenantId;
            rankEvent.oldRankId = expected.rankId;
            rankEvent.newRankId = membership.rankId;
            eventHub_->Publish(rankEvent);
        }
        return true;
    }
    return false;
}

bool CovenantService::GetMembership(const std::string& playerKey,
                                    CovenantId covenantId,
                                    CovenantMembershipState& membership)
{
    std::vector<CovenantMembershipState> memberships;
    if (!LoadMemberships(playerKey, memberships))
        return false;
    for (std::vector<CovenantMembershipState>::const_iterator it = memberships.begin();
         it != memberships.end(); ++it)
    {
        if (it->covenantId == covenantId)
        {
            membership = *it;
            return true;
        }
    }
    return false;
}

bool CovenantService::GetActiveMembership(const std::string& playerKey,
                                           CovenantMembershipState& membership)
{
    std::vector<CovenantMembershipState> memberships;
    if (!LoadMemberships(playerKey, memberships))
        return false;
    bool found = false;
    for (std::vector<CovenantMembershipState>::const_iterator it = memberships.begin();
         it != memberships.end(); ++it)
    {
        if (!it->active)
            continue;
        if (found)
            return false;
        membership = *it;
        found = true;
    }
    return found;
}

CovenantRankId CovenantService::GetRank(const std::string& playerKey, CovenantId covenantId)
{
    CovenantMembershipState membership;
    return GetMembership(playerKey, covenantId, membership) ? membership.rankId : 0;
}

bool CovenantService::HasRankAtLeast(const std::string& playerKey,
                                     CovenantId covenantId,
                                     CovenantRankId rankId)
{
    CovenantMembershipState membership;
    if (!GetMembership(playerKey, covenantId, membership))
        return false;

    const CovenantRankDefinition* required = FindRank(covenantId, rankId);
    const CovenantRankDefinition* current = FindRank(covenantId, membership.rankId);
    return required && current && current->minReputation >= required->minReputation;
}

std::vector<modifiers::GameplayModifier> CovenantService::GetActiveBenefits(const std::string& playerKey,
                                                                            CovenantId covenantId)
{
    CovenantMembershipState membership;
    if (!GetMembership(playerKey, covenantId, membership) || !membership.active)
        return std::vector<modifiers::GameplayModifier>();
    if (membership.rankId == 0)
        return std::vector<modifiers::GameplayModifier>();

    const CovenantDefinition* definition = FindDefinition(covenantId);
    const CovenantRankDefinition* rank = FindRank(covenantId, membership.rankId);
    return definition && rank ? BenefitsWithSource(*definition, *rank)
                              : std::vector<modifiers::GameplayModifier>();
}

bool CovenantService::GetAllActiveBenefits(
    const std::string& playerKey,
    std::vector<modifiers::GameplayModifier>& benefits)
{
    benefits.clear();
    std::vector<CovenantMembershipState> memberships;
    if (!LoadMemberships(playerKey, memberships))
        return false;

    int activeCount = 0;
    for (std::vector<CovenantMembershipState>::const_iterator membership = memberships.begin();
         membership != memberships.end(); ++membership)
    {
        if (!membership->active)
            continue;
        ++activeCount;
        if (activeCount > 1)
        {
            benefits.clear();
            return false;
        }
        if (membership->rankId == 0)
            continue;
        const CovenantDefinition* definition = FindDefinition(membership->covenantId);
        const CovenantRankDefinition* rank = FindRank(membership->covenantId, membership->rankId);
        if (!definition || !rank)
            return false;
        const std::vector<modifiers::GameplayModifier> current =
            BenefitsWithSource(*definition, *rank);
        benefits.insert(benefits.end(), current.begin(), current.end());
    }
    return true;
}

bool CovenantService::GetEarnedRewards(
    CovenantId covenantId,
    CovenantRankId rankId,
    std::vector<CovenantRewardDefinition>& rewards) const
{
    rewards.clear();
    const CovenantDefinition* definition = FindDefinition(covenantId);
    if (!definition)
        return false;

    rewards.insert(rewards.end(), definition->joinRewards.begin(), definition->joinRewards.end());
    if (rankId == 0)
        return true;

    const CovenantRankDefinition* currentRank = FindRank(covenantId, rankId);
    RankMap::const_iterator rankListIt = ranks_.find(covenantId);
    if (!currentRank || rankListIt == ranks_.end())
        return false;

    for (std::vector<CovenantRankDefinition>::const_iterator rankIt = rankListIt->second.begin();
         rankIt != rankListIt->second.end(); ++rankIt)
    {
        if (rankIt->minReputation > currentRank->minReputation)
            break;
        rewards.insert(rewards.end(), rankIt->entryRewards.begin(), rankIt->entryRewards.end());
    }
    return true;
}

bool CovenantService::GetSnapshot(const std::string& playerKey,
                                  CovenantId covenantId,
                                  CovenantSnapshot& snapshot)
{
    const CovenantDefinition* definition = FindDefinition(covenantId);
    CovenantMembershipState membership;
    if (!definition || !GetMembership(playerKey, covenantId, membership))
        return false;

    snapshot = CovenantSnapshot();
    snapshot.covenantId = covenantId;
    snapshot.key = definition->key;
    snapshot.displayNameKey = definition->displayNameKey;
    snapshot.membership = membership;
    snapshot.relatedQuestIds = definition->relatedQuestIds;
    snapshot.relatedStoryIds = definition->relatedStoryIds;

    const CovenantRankDefinition* rank = FindRank(covenantId, membership.rankId);
    if (rank)
    {
        snapshot.rankKey = rank->key;
        snapshot.rankDisplayNameKey = rank->displayNameKey;
        if (membership.active && definition)
            snapshot.activeBenefits = BenefitsWithSource(*definition, *rank);
    }
    return true;
}

bool CovenantService::GetActiveSnapshots(const std::string& playerKey,
                                         std::vector<CovenantSnapshot>& snapshots)
{
    snapshots.clear();
    if (playerKey.empty())
        return false;

    std::vector<CovenantMembershipState> memberships;
    if (!LoadMemberships(playerKey, memberships))
        return false;

    int activeCount = 0;
    for (std::vector<CovenantMembershipState>::const_iterator membership = memberships.begin();
         membership != memberships.end(); ++membership)
    {
        if (!membership->active)
            continue;
        ++activeCount;
        if (activeCount > 1)
        {
            snapshots.clear();
            return false;
        }

        const CovenantDefinition* definition = FindDefinition(membership->covenantId);
        if (!definition)
            return false;

        CovenantSnapshot snapshot;
        snapshot.covenantId = membership->covenantId;
        snapshot.key = definition->key;
        snapshot.displayNameKey = definition->displayNameKey;
        snapshot.membership = *membership;
        snapshot.relatedQuestIds = definition->relatedQuestIds;
        snapshot.relatedStoryIds = definition->relatedStoryIds;

        if (membership->rankId != 0)
        {
            const CovenantRankDefinition* rank =
                FindRank(membership->covenantId, membership->rankId);
            if (!rank)
                return false;
            snapshot.rankKey = rank->key;
            snapshot.rankDisplayNameKey = rank->displayNameKey;
            snapshot.activeBenefits = BenefitsWithSource(*definition, *rank);
        }
        snapshots.push_back(snapshot);
    }
    return true;
}

std::vector<CovenantId> CovenantService::CovenantIds() const
{
    std::vector<CovenantId> ids;
    ids.reserve(definitions_.size());
    for (DefinitionMap::const_iterator it = definitions_.begin(); it != definitions_.end(); ++it)
        ids.push_back(it->first);
    return ids;
}

void CovenantService::GetCatalog(std::vector<CovenantCatalogEntry>& entries) const
{
    entries.clear();
    entries.reserve(definitions_.size());
    for (DefinitionMap::const_iterator definitionIt = definitions_.begin();
         definitionIt != definitions_.end(); ++definitionIt)
    {
        const CovenantDefinition& definition = definitionIt->second;
        CovenantCatalogEntry entry;
        entry.id = definition.id;
        entry.key = definition.key;
        entry.displayNameKey = definition.displayNameKey;
        entry.exclusiveMembership = definition.exclusiveMembership;
        entry.relatedQuestIds = definition.relatedQuestIds;
        entry.relatedStoryIds = definition.relatedStoryIds;

        RankMap::const_iterator ranksIt = ranks_.find(definition.id);
        if (ranksIt != ranks_.end())
        {
            entry.ranks.reserve(ranksIt->second.size());
            for (std::vector<CovenantRankDefinition>::const_iterator rankIt = ranksIt->second.begin();
                 rankIt != ranksIt->second.end(); ++rankIt)
            {
                CovenantRankCatalogEntry rankEntry;
                rankEntry.id = rankIt->id;
                rankEntry.key = rankIt->key;
                rankEntry.displayNameKey = rankIt->displayNameKey;
                rankEntry.minReputation = rankIt->minReputation;
                rankEntry.benefits = rankIt->benefits;
                entry.ranks.push_back(rankEntry);
            }
        }
        entries.push_back(entry);
    }
}
}
}
