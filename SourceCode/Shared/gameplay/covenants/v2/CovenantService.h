#pragma once

#include <map>
#include <string>
#include <vector>

#include "gameplay/conditions/v2/Condition.h"
#include "gameplay/covenants/v2/CovenantEvents.h"
#include "gameplay/covenants/v2/ICovenantRepository.h"
#include "gameplay/events/v2/GameplayEventHub.h"

namespace gameplay
{
namespace covenants
{
class CovenantService
{
public:
    CovenantService(ICovenantRepository& repository,
                    events::GameplayEventHub* eventHub = 0);

    bool RegisterCovenant(const CovenantDefinition& definition);
    bool RegisterRank(const CovenantRankDefinition& definition);
    bool ValidateDefinitions(std::string* error = 0) const;
    bool ValidateRewards(const rewards::RewardService& rewardService, std::string* error = 0) const;

    CovenantJoinEligibility CheckJoinEligibility(
        const std::string& playerKey,
        CovenantId covenantId,
        const conditions::ConditionContext& context);
    bool CanJoin(const std::string& playerKey,
                 CovenantId covenantId,
                 const conditions::ConditionContext& context);
    bool Join(const std::string& playerKey,
              CovenantId covenantId,
              const conditions::ConditionContext& context);
    bool Leave(const std::string& playerKey, CovenantId covenantId);
    bool AddReputation(const std::string& playerKey,
                       CovenantId covenantId,
                       int amount,
                       const std::string& sourceKey);
    bool ReevaluateRank(const std::string& playerKey, CovenantId covenantId);

    bool GetMembership(const std::string& playerKey,
                       CovenantId covenantId,
                       CovenantMembershipState& membership);
    CovenantRankId GetRank(const std::string& playerKey, CovenantId covenantId);
    bool HasRankAtLeast(const std::string& playerKey,
                        CovenantId covenantId,
                        CovenantRankId rankId);
    std::vector<modifiers::GameplayModifier> GetActiveBenefits(const std::string& playerKey,
                                                               CovenantId covenantId);
    bool GetAllActiveBenefits(const std::string& playerKey,
                              std::vector<modifiers::GameplayModifier>& benefits);
    bool GetEarnedRewards(CovenantId covenantId,
                          CovenantRankId rankId,
                          std::vector<CovenantRewardDefinition>& rewards) const;
    bool GetSnapshot(const std::string& playerKey,
                     CovenantId covenantId,
                     CovenantSnapshot& snapshot);
    bool GetActiveSnapshots(const std::string& playerKey,
                            std::vector<CovenantSnapshot>& snapshots);
    std::vector<CovenantId> CovenantIds() const;
    void GetCatalog(std::vector<CovenantCatalogEntry>& entries) const;

private:
    typedef std::map<CovenantId, CovenantDefinition> DefinitionMap;
    typedef std::map<CovenantId, std::vector<CovenantRankDefinition> > RankMap;

    bool LoadMemberships(const std::string& playerKey,
                         std::vector<CovenantMembershipState>& memberships);
    const CovenantDefinition* FindDefinition(CovenantId covenantId) const;
    const CovenantRankDefinition* FindRank(CovenantId covenantId, CovenantRankId rankId) const;
    CovenantRankId CalculateRank(CovenantId covenantId, int reputation) const;
    int ClampReputation(const CovenantDefinition& definition, long long value) const;

    ICovenantRepository& repository_;
    events::GameplayEventHub* eventHub_;
    DefinitionMap definitions_;
    RankMap ranks_;
};
}
}
