#pragma once

#include <algorithm>
#include <string>

namespace gameplay
{
namespace quests
{
enum class QuestObjectiveType
{
    Kill,
    Collect,
    Talk,
    Interact,
    Visit,
    Escort,
    Gather,
    Craft,
    Fish,
    Mine,
    Hunt,
    Defend,
    Custom
};

struct QuestObjectiveDefinition
{
    int objectiveId;
    QuestObjectiveType type;
    std::string targetKey;
    int targetMap;
    int requiredCount;
    bool allowPartyCredit;

    QuestObjectiveDefinition()
        : objectiveId(0),
          type(QuestObjectiveType::Custom),
          targetMap(-1),
          requiredCount(1),
          allowPartyCredit(true)
    {
    }
};

struct QuestObjectiveState
{
    int currentCount;

    QuestObjectiveState() : currentCount(0) {}

    bool IsComplete(const QuestObjectiveDefinition& definition) const
    {
        return definition.requiredCount > 0 && currentCount >= definition.requiredCount;
    }
};

struct QuestObjectiveSignal
{
    QuestObjectiveType type;
    std::string targetKey;
    int map;
    int amount;
    bool partyCredit;

    QuestObjectiveSignal()
        : type(QuestObjectiveType::Custom), map(-1), amount(1), partyCredit(false)
    {
    }
};

class QuestObjectiveEvaluator
{
public:
    static bool Matches(const QuestObjectiveDefinition& definition,
                        const QuestObjectiveSignal& signal)
    {
        if (definition.requiredCount <= 0 || signal.amount <= 0)
            return false;
        if (definition.type != signal.type)
            return false;
        if (signal.partyCredit && !definition.allowPartyCredit)
            return false;
        if (definition.targetMap >= 0 && definition.targetMap != signal.map)
            return false;
        if (!definition.targetKey.empty() && definition.targetKey != signal.targetKey)
            return false;
        return true;
    }

    static bool Apply(const QuestObjectiveDefinition& definition,
                      const QuestObjectiveSignal& signal,
                      QuestObjectiveState& state)
    {
        if (!Matches(definition, signal) || state.IsComplete(definition))
            return false;

        const int safeCurrent = std::max(0, state.currentCount);
        const int remaining = definition.requiredCount - safeCurrent;
        state.currentCount = safeCurrent + std::min(signal.amount, remaining);
        return true;
    }
};
}
}
