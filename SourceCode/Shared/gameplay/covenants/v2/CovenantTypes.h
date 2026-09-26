#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "gameplay/conditions/v2/Condition.h"
#include "gameplay/modifiers/v2/GameplayModifier.h"
#include "gameplay/rewards/v2/Reward.h"

namespace gameplay
{
namespace covenants
{
typedef int CovenantId;
typedef int CovenantRankId;

enum class CovenantJoinEligibility
{
    Eligible,
    InvalidRequest,
    UnknownCovenant,
    ConditionsNotMet,
    AlreadyActive,
    AlreadyMember,
    ExclusiveConflict,
    PersistenceFailure
};

enum class CovenantActivationResult
{
    Activated,
    InvalidRequest,
    UnknownCovenant,
    NotMember,
    AlreadyActive,
    CooldownActive,
    PersistenceFailure
};

// Reward keys are embedded in durable source keys (covenant:<id>:reward:<key>).
// Keep headroom below the 255-byte persistence/source-key boundary.
static const std::size_t kCovenantRewardKeyMaxLength = 200u;

struct CovenantRewardDefinition
{
    std::string key;
    rewards::RewardDefinition reward;
};

struct CovenantRankDefinition
{
    CovenantRankId id;
    CovenantId covenantId;
    std::string key;
    std::string displayNameKey;
    int minReputation;
    std::vector<modifiers::GameplayModifier> benefits;
    std::vector<CovenantRewardDefinition> entryRewards;

    CovenantRankDefinition()
        : id(0), covenantId(0), minReputation(0)
    {
    }
};

struct CovenantDefinition
{
    CovenantId id;
    std::string key;
    std::string displayNameKey;
    bool exclusiveMembership;
    bool hasReputationBounds;
    int minReputation;
    int maxReputation;
    std::vector<conditions::ConditionDefinition> joinConditions;
    std::vector<int> relatedQuestIds;
    std::vector<int> relatedStoryIds;
    std::vector<CovenantRewardDefinition> joinRewards;

    CovenantDefinition()
        : id(0), exclusiveMembership(false), hasReputationBounds(false),
          minReputation(0), maxReputation(0)
    {
    }
};

struct CovenantMembershipState
{
    CovenantId covenantId;
    int reputation;
    CovenantRankId rankId;
    bool active;

    CovenantMembershipState()
        : covenantId(0), reputation(0), rankId(0), active(false)
    {
    }
};


struct CovenantRankCatalogEntry
{
    CovenantRankId id;
    std::string key;
    std::string displayNameKey;
    int minReputation;
    std::vector<modifiers::GameplayModifier> benefits;

    CovenantRankCatalogEntry() : id(0), minReputation(0) {}
};

struct CovenantCatalogEntry
{
    CovenantId id;
    std::string key;
    std::string displayNameKey;
    bool exclusiveMembership;
    std::vector<CovenantRankCatalogEntry> ranks;
    std::vector<int> relatedQuestIds;
    std::vector<int> relatedStoryIds;

    CovenantCatalogEntry() : id(0), exclusiveMembership(false) {}
};

struct CovenantSnapshot
{
    CovenantId covenantId;
    std::string key;
    std::string displayNameKey;
    CovenantMembershipState membership;
    std::string rankKey;
    std::string rankDisplayNameKey;
    std::vector<modifiers::GameplayModifier> activeBenefits;
    std::vector<int> relatedQuestIds;
    std::vector<int> relatedStoryIds;

    CovenantSnapshot() : covenantId(0) {}
};
}
}
