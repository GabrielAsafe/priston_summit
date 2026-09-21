#pragma once

#include <string>

#include "gameplay/conditions/v2/Condition.h"
#include "gameplay/covenants/v2/CovenantService.h"
#include "gameplay/story/v2/IStoryRepository.h"
#include "gameplay/reputation/v2/ReputationService.h"

namespace gameplay
{
namespace progression
{
static const int kProgressionUnavailableValue = (-2147483647 - 1);

class IQuestStateReader
{
public:
    virtual ~IQuestStateReader() {}
    virtual int QuestState(const std::string& playerKey, int questId) const = 0;
};

class IProfessionLevelReader
{
public:
    virtual ~IProfessionLevelReader() {}
    virtual int ProfessionLevel(const std::string& playerKey,
                                const std::string& professionKey) const = 0;
};

class ICustomConditionReader
{
public:
    virtual ~ICustomConditionReader() {}
    virtual bool Evaluate(const std::string& playerKey,
                          const conditions::ConditionDefinition& definition) const = 0;
};

class ServerProgressionConditionContext : public conditions::ConditionContext
{
public:
    ServerProgressionConditionContext(const std::string& playerKey,
                                      int playerLevel,
                                      int legacyJobCode,
                                      story::IStoryRepository& storyRepository,
                                      covenants::CovenantService& covenantService,
                                      reputation::ReputationService& reputationService,
                                      const IQuestStateReader* questReader = 0,
                                      const IProfessionLevelReader* professionReader = 0,
                                      const ICustomConditionReader* customReader = 0);

    int PlayerLevel() const;
    int LegacyJobCode() const;
    bool HasStoryFlag(const std::string& key) const;
    bool HasWorldStoryFlag(const std::string& key) const;
    int StoryNodeState(int storyId, int nodeId) const;
    int QuestState(int questId) const;
    bool HasCovenantMembership(int covenantId) const;
    int CovenantRank(int covenantId) const;
    int CovenantReputation(int covenantId) const;
    int ProfessionLevel(const std::string& professionKey) const;
    int Reputation(const std::string& reputationKey) const;
    bool EvaluateCustomCondition(const conditions::ConditionDefinition& definition) const;

private:
    std::string playerKey_;
    int playerLevel_;
    int legacyJobCode_;
    story::IStoryRepository& storyRepository_;
    covenants::CovenantService& covenantService_;
    reputation::ReputationService& reputationService_;
    const IQuestStateReader* questReader_;
    const IProfessionLevelReader* professionReader_;
    const ICustomConditionReader* customReader_;
};
}
}
