#pragma once

#include <string>

#include "gameplay/covenants/v2/CovenantService.h"
#include "gameplay/rewards/v2/Reward.h"
#include "gameplay/reputation/v2/ReputationService.h"
#include "gameplay/story/v2/StoryService.h"
#include "gameplay/progression/v2/ProgressionCustomHandlers.h"
#include "gameplay/progression/v2/ProgressionPlayerRuntime.h"
#include "gameplay/progression/v2/ProgressionSnapshot.h"
#include "systems/gameplay/progression/v2/ServerProgressionConditionContext.h"

namespace gameplay
{
namespace covenants { class SQLiteCovenantRepository; class CovenantRewardProcessor; }
namespace reputation { class SQLiteReputationRepository; }
namespace story { class SQLiteStoryRepository; }
namespace progression
{
class ProgressionRewardGateway;
class ProgressionStoryCommandGateway;

enum class ServerProgressionRuntimeState
{
    NotStarted,
    CatalogLoaded,
    Ready,
    Failed
};

struct ServerProgressionRuntimeStatus
{
    ServerProgressionRuntimeState state;
    std::string failure;
    int reputationDefinitionCount;
    int covenantDefinitionCount;
    int storyDefinitionCount;

    ServerProgressionRuntimeStatus()
        : state(ServerProgressionRuntimeState::NotStarted),
          reputationDefinitionCount(0), covenantDefinitionCount(0), storyDefinitionCount(0)
    {
    }
};

covenants::SQLiteCovenantRepository& ServerCovenantRepository();
story::SQLiteStoryRepository& ServerStoryRepository();
covenants::CovenantService& ServerCovenantService();
reputation::ReputationService& ServerReputationService();
covenants::CovenantRewardProcessor& ServerCovenantRewardProcessor();
rewards::RewardService& ServerProgressionRewardService();
story::StoryService& ServerStoryService();
ProgressionCustomHandlerRegistry& ServerProgressionCustomHandlers();
ServerProgressionConditionContext MakeServerProgressionConditionContext(
    const std::string& playerKey, int playerLevel, int legacyJobCode);
ProgressionPlayerRunReport ProcessServerProgressionPlayer(
    const std::string& playerKey, int playerLevel, int legacyJobCode);
void GetServerCovenantCatalog(std::vector<covenants::CovenantCatalogEntry>& entries);
void GetServerReputationCatalog(std::vector<reputation::ReputationCatalogEntry>& entries);
void GetServerStoryCatalog(std::vector<story::StoryCatalogEntry>& entries);
bool GetServerProgressionPlayerSnapshot(const std::string& playerKey,
                                        ProgressionPlayerSnapshot& snapshot);

covenants::CovenantJoinEligibility CheckServerCovenantJoinEligibility(
    const std::string& playerKey,
    int covenantId,
    int playerLevel,
    int legacyJobCode);
bool JoinServerCovenant(const std::string& playerKey,
                        int covenantId,
                        int playerLevel,
                        int legacyJobCode);
bool LeaveServerCovenant(const std::string& playerKey, int covenantId);
bool LeaveServerCovenant(const std::string& playerKey,
                         int covenantId,
                         int playerLevel,
                         int legacyJobCode);
bool GetServerReputationValue(const std::string& playerKey,
                              const std::string& reputationKey,
                              int& value);
bool AddServerReputation(const std::string& playerKey,
                         const std::string& reputationKey,
                         int amount,
                         const std::string& sourceKey);
bool AddServerCovenantReputation(const std::string& playerKey,
                                 int covenantId,
                                 int amount,
                                 const std::string& sourceKey,
                                 int playerLevel,
                                 int legacyJobCode);
bool GetServerCovenantSnapshot(const std::string& playerKey,
                               int covenantId,
                               covenants::CovenantSnapshot& snapshot);
bool GetServerActiveCovenantBenefits(
    const std::string& playerKey,
    std::vector<modifiers::GameplayModifier>& benefits);
bool GetServerStorySnapshot(const std::string& playerKey,
                            int storyId,
                            story::StorySnapshot& snapshot);
story::StoryStartResult StartServerStory(
    const std::string& playerKey,
    int storyId,
    int playerLevel,
    int legacyJobCode);
story::StoryChoiceResult ChooseServerStoryDialogueOption(
    const std::string& playerKey,
    int storyId,
    const std::string& choiceKey,
    int playerLevel,
    int legacyJobCode);
bool GetServerAvailableStoryDialogueChoices(
    const std::string& playerKey,
    int storyId,
    int playerLevel,
    int legacyJobCode,
    std::vector<std::string>& choiceKeys);
bool GetServerAvailableStoryDialogueChoiceSnapshots(
    const std::string& playerKey,
    int storyId,
    int playerLevel,
    int legacyJobCode,
    std::vector<story::StoryDialogueChoiceSnapshot>& choices);
story::StoryEffectReceiptLoadResult GetServerStoryEffectReceiptStatus(
    const std::string& playerKey,
    int storyId,
    int nodeId,
    const std::string& effectKey,
    story::StoryEffectReceiptStatus& status);
bool GetServerPendingStoryEffects(
    const std::string& playerKey,
    std::vector<story::StoryEffectReceiptEntry>& receipts);
bool AcknowledgeServerStoryPendingEffect(const std::string& playerKey,
                                         int storyId,
                                         int nodeId,
                                         const std::string& effectKey);

covenants::CovenantEffectReceiptLoadResult GetServerCovenantEffectReceiptStatus(
    const std::string& playerKey,
    int covenantId,
    const std::string& effectKey,
    covenants::CovenantEffectReceiptStatus& status);
bool GetServerPendingCovenantEffects(
    const std::string& playerKey,
    std::vector<covenants::CovenantEffectReceiptEntry>& receipts);
bool AcknowledgeServerCovenantPendingEffect(const std::string& playerKey,
                                             int covenantId,
                                             const std::string& effectKey);

bool IsServerProgressionRuntimeReady();
ServerProgressionRuntimeStatus GetServerProgressionRuntimeStatus();
bool ValidateServerProgressionDefinitions(std::string* error = 0);
bool InitializeServerProgressionRuntime(std::string* error = 0);
bool FinalizeServerProgressionRuntimeAfterItems(std::string* error = 0);
}
}
