#pragma once

#include "systems/gameplay/progression/v2/ServerProgressionConditionContext.h"
#include "gameplay/progression/v2/ProgressionCustomHandlers.h"
#include "gameplay/quests/v2/QuestStateQuery.h"

namespace gameplay
{
namespace progression
{
class LegacyQuestStateReader : public IQuestStateReader
{
public:
    explicit LegacyQuestStateReader(const quests::IQuestStateQuery& quests) : quests_(quests) {}

    int QuestState(const std::string& playerKey, int questId) const;

private:
    const quests::IQuestStateQuery& quests_;
};

class LegacyProfessionLevelReader : public IProfessionLevelReader
{
public:
    int ProfessionLevel(const std::string& playerKey,
                        const std::string& professionKey) const;
};

class RegisteredCustomConditionReader : public ICustomConditionReader
{
public:
    explicit RegisteredCustomConditionReader(const ProgressionCustomHandlerRegistry& handlers)
        : handlers_(handlers)
    {
    }

    bool Evaluate(const std::string& playerKey,
                  const conditions::ConditionDefinition& definition) const;

private:
    const ProgressionCustomHandlerRegistry& handlers_;
};
}
}
