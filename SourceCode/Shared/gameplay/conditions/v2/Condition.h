#pragma once

#include <string>
#include <vector>

namespace gameplay
{
namespace conditions
{
enum class QuestProgressState
{
    NotActive = 0,
    InProgress = 1,
    Completed = 2
};

enum class ConditionType
{
    Always,
    PlayerLevelAtLeast,
    ClassIs,
    HasStoryFlag,
    StoryNodeState,
    QuestState,
    CovenantMembership,
    CovenantRankAtLeast,
    CovenantReputationAtLeast,
    ProfessionLevelAtLeast,
    Custom,
    HasWorldStoryFlag,
    ReputationAtLeast
};

struct ConditionDefinition
{
    ConditionType type;
    std::string key;
    int id;
    int value;
    int auxiliaryId;

    ConditionDefinition()
        : type(ConditionType::Always), id(0), value(0), auxiliaryId(0)
    {
    }
};

class ConditionContext
{
public:
    virtual ~ConditionContext() {}

    virtual int PlayerLevel() const = 0;
    virtual int LegacyJobCode() const = 0;
    virtual bool HasStoryFlag(const std::string& key) const = 0;
    virtual bool HasWorldStoryFlag(const std::string& key) const { (void)key; return false; }
    virtual int StoryNodeState(int storyId, int nodeId) const = 0;
    virtual int QuestState(int questId) const = 0;
    virtual bool HasCovenantMembership(int covenantId) const = 0;
    virtual int CovenantRank(int covenantId) const = 0;
    virtual int CovenantReputation(int covenantId) const = 0;
    virtual int ProfessionLevel(const std::string& professionKey) const = 0;
    virtual int Reputation(const std::string& reputationKey) const
    {
        (void)reputationKey;
        return (-2147483647 - 1);
    }
    virtual bool EvaluateCustomCondition(const ConditionDefinition& definition) const = 0;
};

class ConditionService
{
public:
    static bool Validate(const ConditionDefinition& definition)
    {
        switch (definition.type)
        {
        case ConditionType::Always:
            return true;
        case ConditionType::PlayerLevelAtLeast:
        case ConditionType::ClassIs:
            return definition.value >= 0;
        case ConditionType::HasStoryFlag:
        case ConditionType::HasWorldStoryFlag:
            return !definition.key.empty() && definition.key.size() <= 255u;
        case ConditionType::StoryNodeState:
            return definition.id > 0 && definition.auxiliaryId > 0 &&
                   definition.value >= 0 && definition.value <= 2;
        case ConditionType::QuestState:
            return definition.id > 0 &&
                   definition.value >= static_cast<int>(QuestProgressState::NotActive) &&
                   definition.value <= static_cast<int>(QuestProgressState::Completed);
        case ConditionType::CovenantMembership:
            return definition.id > 0;
        case ConditionType::CovenantRankAtLeast:
            return definition.id > 0 && definition.value > 0;
        case ConditionType::CovenantReputationAtLeast:
            return definition.id > 0;
        case ConditionType::ProfessionLevelAtLeast:
            return !definition.key.empty() && definition.key.size() <= 255u &&
                   definition.value >= 0;
        case ConditionType::ReputationAtLeast:
            return !definition.key.empty() && definition.key.size() <= 255u;
        case ConditionType::Custom:
            return !definition.key.empty() && definition.key.size() <= 255u;
        }
        return false;
    }

    static bool ValidateAll(const std::vector<ConditionDefinition>& definitions)
    {
        for (std::vector<ConditionDefinition>::const_iterator it = definitions.begin();
             it != definitions.end(); ++it)
            if (!Validate(*it))
                return false;
        return true;
    }

    static bool Evaluate(const ConditionDefinition& definition,
                         const ConditionContext& context)
    {
        switch (definition.type)
        {
        case ConditionType::Always:
            return true;
        case ConditionType::PlayerLevelAtLeast:
            return context.PlayerLevel() >= definition.value;
        case ConditionType::ClassIs:
            return context.LegacyJobCode() == definition.value;
        case ConditionType::HasStoryFlag:
            return context.HasStoryFlag(definition.key);
        case ConditionType::HasWorldStoryFlag:
            return context.HasWorldStoryFlag(definition.key);
        case ConditionType::StoryNodeState:
            return context.StoryNodeState(definition.id, definition.auxiliaryId) == definition.value;
        case ConditionType::QuestState:
            return context.QuestState(definition.id) == definition.value;
        case ConditionType::CovenantMembership:
            return context.HasCovenantMembership(definition.id);
        case ConditionType::CovenantRankAtLeast:
            return context.CovenantRank(definition.id) >= definition.value;
        case ConditionType::CovenantReputationAtLeast:
            return context.CovenantReputation(definition.id) >= definition.value;
        case ConditionType::ProfessionLevelAtLeast:
            return context.ProfessionLevel(definition.key) >= definition.value;
        case ConditionType::ReputationAtLeast:
            return context.Reputation(definition.key) >= definition.value;
        case ConditionType::Custom:
            return context.EvaluateCustomCondition(definition);
        }
        return false;
    }

    static bool EvaluateAll(const std::vector<ConditionDefinition>& definitions,
                            const ConditionContext& context)
    {
        for (std::vector<ConditionDefinition>::const_iterator it = definitions.begin();
             it != definitions.end(); ++it)
        {
            if (!Evaluate(*it, context))
                return false;
        }
        return true;
    }
};
}
}
