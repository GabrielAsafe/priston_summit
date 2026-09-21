#include "systems/gameplay/progression/v2/ProgressionRuntime.h"

#include <map>
#include <mutex>

#include "systems/gameplay/progression/v2/LegacyProgressionGateways.h"
#include "systems/gameplay/progression/v2/LegacyProgressionReaders.h"
#include "systems/gameplay/progression/v2/ProgressionTriggerRuntime.h"
#include "gameplay/progression/v2/ProgressionContent.h"
#include "gameplay/progression/v2/ProgressionTriggerCoordinator.h"
#include "systems/gameplay/progression/v2/SQLiteProgressionContentLoader.h"
#include "systems/gameplay/covenants/v2/SQLiteCovenantRepository.h"
#include "systems/gameplay/reputation/v2/SQLiteReputationRepository.h"
#include "gameplay/covenants/v2/CovenantRewardProcessor.h"
#include "systems/gameplay/events/v2/GameplayEventRuntime.h"
#include "systems/gameplay/story/v2/SQLiteStoryRepository.h"

namespace gameplay
{
namespace progression
{
namespace
{
bool ValidateCustomConditions(const std::vector<conditions::ConditionDefinition>& conditions,
                              const ProgressionCustomHandlerRegistry& handlers,
                              std::string* error)
{
    for (std::vector<conditions::ConditionDefinition>::const_iterator it = conditions.begin();
         it != conditions.end(); ++it)
    {
        if (it->type == conditions::ConditionType::Custom && !handlers.ValidateCondition(*it))
        {
            if (error) *error = "progression content contains an unregistered custom condition";
            return false;
        }
    }
    return true;
}

bool ValidateCustomConditionCatalog(const ProgressionContentCatalog& catalog,
                                    const ProgressionCustomHandlerRegistry& handlers,
                                    std::string* error)
{
    for (std::vector<CovenantContentDefinition>::const_iterator covenant = catalog.covenants.begin();
         covenant != catalog.covenants.end(); ++covenant)
    {
        if (!ValidateCustomConditions(covenant->definition.joinConditions, handlers, error))
            return false;
    }
    for (std::vector<StoryContentDefinition>::const_iterator story = catalog.stories.begin();
         story != catalog.stories.end(); ++story)
    {
        if (!ValidateCustomConditions(story->definition.startConditions, handlers, error))
            return false;
        for (std::vector<story::StoryChapterDefinition>::const_iterator chapter = story->chapters.begin();
             chapter != story->chapters.end(); ++chapter)
            if (!ValidateCustomConditions(chapter->entryConditions, handlers, error))
                return false;
        for (std::vector<story::StoryNodeDefinition>::const_iterator node = story->nodes.begin();
             node != story->nodes.end(); ++node)
        {
            if (!ValidateCustomConditions(node->entryConditions, handlers, error) ||
                !ValidateCustomConditions(node->completionConditions, handlers, error))
                return false;
            for (std::vector<story::StoryDialogueChoiceDefinition>::const_iterator choice =
                     node->dialogueChoices.begin(); choice != node->dialogueChoices.end(); ++choice)
                if (!ValidateCustomConditions(choice->conditions, handlers, error))
                    return false;
        }
    }
    return true;
}

ServerProgressionRuntimeState& InitializationState()
{
    static ServerProgressionRuntimeState state = ServerProgressionRuntimeState::NotStarted;
    return state;
}

std::string& InitializationFailure()
{
    static std::string failure;
    return failure;
}

ProgressionTriggerCoordinator& PlayerProcessingCoordinator()
{
    static ProgressionTriggerCoordinator coordinator;
    return coordinator;
}

std::recursive_mutex& PlayerProcessingMutex()
{
    static std::recursive_mutex mutex;
    return mutex;
}

struct RuntimePlayerSnapshot
{
    int playerLevel;
    int legacyJobCode;

    RuntimePlayerSnapshot() : playerLevel(0), legacyJobCode(0) {}
    RuntimePlayerSnapshot(int level, int job) : playerLevel(level), legacyJobCode(job) {}
};

std::map<std::string, RuntimePlayerSnapshot>& PlayerProcessingSnapshots()
{
    static std::map<std::string, RuntimePlayerSnapshot> snapshots;
    return snapshots;
}

void AccumulateReport(ProgressionPlayerRunReport& total,
                      const ProgressionPlayerRunReport& current)
{
    total.storiesStarted += current.storiesStarted;
    total.storiesAdvanced += current.storiesAdvanced;
    total.advanceSteps += current.advanceSteps;
    total.stepLimitHits += current.stepLimitHits;
    total.failures += current.failures;
    total.storiesUnchanged = current.storiesUnchanged;
}

struct RuntimeStorage
{
    covenants::SQLiteCovenantRepository covenantRepository;
    covenants::CovenantService covenantService;
    reputation::SQLiteReputationRepository reputationRepository;
    reputation::ReputationService reputationService;
    story::SQLiteStoryRepository storyRepository;
    ProgressionCustomHandlerRegistry customHandlers;
    LegacyProgressionRewardGateway rewardGateway;
    rewards::RewardService rewardService;
    covenants::CovenantRewardProcessor covenantRewardProcessor;
    LegacyProgressionStoryCommandGateway commandGateway;
    story::StoryService storyService;
    LegacyQuestStateReader questReader;
    LegacyProfessionLevelReader professionReader;
    RegisteredCustomConditionReader customConditionReader;

    RuntimeStorage()
        : covenantService(covenantRepository, &events::ServerGameplayEventHub()),
          reputationService(reputationRepository, &events::ServerGameplayEventHub()),
          rewardGateway(storyRepository, covenantService, reputationService, customHandlers),
          rewardService(rewardGateway),
          covenantRewardProcessor(covenantRepository, covenantService, rewardService),
          commandGateway(covenantService, customHandlers),
          storyService(storyRepository, commandGateway, rewardService,
                       &events::ServerGameplayEventHub()),
          customConditionReader(customHandlers)
    {
    }
};

RuntimeStorage& Storage()
{
    static RuntimeStorage storage;
    return storage;
}
}

covenants::SQLiteCovenantRepository& ServerCovenantRepository()
{
    return Storage().covenantRepository;
}

story::SQLiteStoryRepository& ServerStoryRepository()
{
    return Storage().storyRepository;
}

covenants::CovenantService& ServerCovenantService()
{
    return Storage().covenantService;
}

reputation::ReputationService& ServerReputationService()
{
    return Storage().reputationService;
}

covenants::CovenantRewardProcessor& ServerCovenantRewardProcessor()
{
    return Storage().covenantRewardProcessor;
}

rewards::RewardService& ServerProgressionRewardService()
{
    return Storage().rewardService;
}

story::StoryService& ServerStoryService()
{
    return Storage().storyService;
}

ProgressionCustomHandlerRegistry& ServerProgressionCustomHandlers()
{
    return Storage().customHandlers;
}

ServerProgressionConditionContext MakeServerProgressionConditionContext(
    const std::string& playerKey, int playerLevel, int legacyJobCode)
{
    return ServerProgressionConditionContext(
        playerKey, playerLevel, legacyJobCode, Storage().storyRepository, Storage().covenantService,
        Storage().reputationService, &Storage().questReader, &Storage().professionReader, &Storage().customConditionReader);
}

ProgressionPlayerRunReport ProcessServerProgressionPlayer(
    const std::string& playerKey, int playerLevel, int legacyJobCode)
{
    ProgressionPlayerRunReport total;
    if (playerKey.empty() || !IsServerProgressionRuntimeReady())
    {
        total.failures = 1;
        return total;
    }

    std::lock_guard<std::recursive_mutex> lock(PlayerProcessingMutex());
    ProgressionTriggerCoordinator& coordinator = PlayerProcessingCoordinator();
    const bool ownsSnapshot = !coordinator.IsRunning(playerKey);
    PlayerProcessingSnapshots()[playerKey] = RuntimePlayerSnapshot(playerLevel, legacyJobCode);

    const bool accepted = coordinator.Request(playerKey, [&]()
    {
        if (!Storage().covenantRewardProcessor.ProcessPlayer(playerKey))
        {
            ++total.failures;
            return false;
        }

        // Reward processing can itself trigger a reentrant request (for example XP
        // causing a level-up). Always use the newest authoritative snapshot.
        const RuntimePlayerSnapshot snapshot = PlayerProcessingSnapshots()[playerKey];
        ServerProgressionConditionContext context =
            MakeServerProgressionConditionContext(
                playerKey, snapshot.playerLevel, snapshot.legacyJobCode);
        const ProgressionPlayerRunReport current =
            ProgressionPlayerStoryRunner::Run(Storage().storyService, playerKey, context);
        AccumulateReport(total, current);
        return current.Ok();
    });

    if (ownsSnapshot)
        PlayerProcessingSnapshots().erase(playerKey);
    if (!accepted && total.failures == 0)
        total.failures = 1;
    return total;
}

void GetServerCovenantCatalog(std::vector<covenants::CovenantCatalogEntry>& entries)
{
    Storage().covenantService.GetCatalog(entries);
}

void GetServerReputationCatalog(std::vector<reputation::ReputationCatalogEntry>& entries)
{
    Storage().reputationService.GetCatalog(entries);
}

void GetServerStoryCatalog(std::vector<story::StoryCatalogEntry>& entries)
{
    Storage().storyService.GetCatalog(entries);
}

bool GetServerProgressionPlayerSnapshot(const std::string& playerKey,
                                        ProgressionPlayerSnapshot& snapshot)
{
    snapshot = ProgressionPlayerSnapshot();
    if (playerKey.empty() || !IsServerProgressionRuntimeReady())
        return false;

    if (!Storage().covenantService.GetActiveSnapshots(playerKey, snapshot.activeCovenants) ||
        !Storage().reputationService.GetSnapshots(playerKey, snapshot.reputations) ||
        !Storage().storyService.GetStartedSnapshots(playerKey, snapshot.startedStories))
        return false;

    std::vector<story::StoryEffectReceiptEntry> storyReceipts;
    std::vector<covenants::CovenantEffectReceiptEntry> covenantReceipts;
    if (!Storage().storyService.ListPendingEffects(playerKey, storyReceipts) ||
        !Storage().covenantRewardProcessor.ListPendingEffects(playerKey, covenantReceipts))
        return false;

    snapshot.pendingStoryEffectCount = static_cast<int>(storyReceipts.size());
    snapshot.pendingCovenantEffectCount = static_cast<int>(covenantReceipts.size());
    return true;
}

covenants::CovenantJoinEligibility CheckServerCovenantJoinEligibility(
    const std::string& playerKey,
    int covenantId,
    int playerLevel,
    int legacyJobCode)
{
    if (!IsServerProgressionRuntimeReady())
        return covenants::CovenantJoinEligibility::PersistenceFailure;
    ServerProgressionConditionContext context =
        MakeServerProgressionConditionContext(playerKey, playerLevel, legacyJobCode);
    return Storage().covenantService.CheckJoinEligibility(playerKey, covenantId, context);
}

bool JoinServerCovenant(const std::string& playerKey,
                        int covenantId,
                        int playerLevel,
                        int legacyJobCode)
{
    ServerProgressionConditionContext context =
        MakeServerProgressionConditionContext(playerKey, playerLevel, legacyJobCode);
    const bool joined = Storage().covenantService.Join(playerKey, covenantId, context);
    if (joined)
        (void)ProcessServerProgressionPlayer(playerKey, playerLevel, legacyJobCode);
    return joined;
}

bool LeaveServerCovenant(const std::string& playerKey, int covenantId)
{
    return Storage().covenantService.Leave(playerKey, covenantId);
}

bool LeaveServerCovenant(const std::string& playerKey,
                         int covenantId,
                         int playerLevel,
                         int legacyJobCode)
{
    const bool left = Storage().covenantService.Leave(playerKey, covenantId);
    if (left)
        (void)ProcessServerProgressionPlayer(playerKey, playerLevel, legacyJobCode);
    return left;
}

bool GetServerReputationValue(const std::string& playerKey,
                              const std::string& reputationKey,
                              int& value)
{
    return IsServerProgressionRuntimeReady() &&
           Storage().reputationService.GetValue(playerKey, reputationKey, value);
}

bool AddServerReputation(const std::string& playerKey,
                         const std::string& reputationKey,
                         int amount,
                         const std::string& sourceKey)
{
    return IsServerProgressionRuntimeReady() &&
           Storage().reputationService.Add(playerKey, reputationKey, amount, sourceKey);
}

bool AddServerCovenantReputation(const std::string& playerKey,
                                 int covenantId,
                                 int amount,
                                 const std::string& sourceKey,
                                 int playerLevel,
                                 int legacyJobCode)
{
    const bool applied = Storage().covenantService.AddReputation(
        playerKey, covenantId, amount, sourceKey);
    if (applied)
        (void)ProcessServerProgressionPlayer(playerKey, playerLevel, legacyJobCode);
    return applied;
}

bool GetServerCovenantSnapshot(const std::string& playerKey,
                               int covenantId,
                               covenants::CovenantSnapshot& snapshot)
{
    return Storage().covenantService.GetSnapshot(playerKey, covenantId, snapshot);
}

bool GetServerActiveCovenantBenefits(
    const std::string& playerKey,
    std::vector<modifiers::GameplayModifier>& benefits)
{
    return Storage().covenantService.GetAllActiveBenefits(playerKey, benefits);
}

bool GetServerStorySnapshot(const std::string& playerKey,
                            int storyId,
                            story::StorySnapshot& snapshot)
{
    return Storage().storyService.GetSnapshot(playerKey, storyId, snapshot);
}

story::StoryStartResult StartServerStory(
    const std::string& playerKey,
    int storyId,
    int playerLevel,
    int legacyJobCode)
{
    if (!IsServerProgressionRuntimeReady() || playerKey.empty() || storyId <= 0)
        return story::StoryStartResult::Failed;

    ServerProgressionConditionContext context =
        MakeServerProgressionConditionContext(playerKey, playerLevel, legacyJobCode);
    const story::StoryStartResult result =
        Storage().storyService.StartDetailed(playerKey, storyId, context);
    if (result == story::StoryStartResult::Started ||
        result == story::StoryStartResult::AlreadyStarted)
        (void)ProcessServerProgressionPlayer(playerKey, playerLevel, legacyJobCode);
    return result;
}

story::StoryChoiceResult ChooseServerStoryDialogueOption(
    const std::string& playerKey,
    int storyId,
    const std::string& choiceKey,
    int playerLevel,
    int legacyJobCode)
{
    ServerProgressionConditionContext context =
        MakeServerProgressionConditionContext(playerKey, playerLevel, legacyJobCode);
    const story::StoryChoiceResult result =
        Storage().storyService.ChooseDialogueOption(playerKey, storyId, choiceKey, context);
    if (result == story::StoryChoiceResult::Selected)
        (void)ProcessServerProgressionPlayer(playerKey, playerLevel, legacyJobCode);
    return result;
}

bool GetServerAvailableStoryDialogueChoices(
    const std::string& playerKey,
    int storyId,
    int playerLevel,
    int legacyJobCode,
    std::vector<std::string>& choiceKeys)
{
    ServerProgressionConditionContext context =
        MakeServerProgressionConditionContext(playerKey, playerLevel, legacyJobCode);
    return Storage().storyService.GetAvailableDialogueChoices(
        playerKey, storyId, context, choiceKeys);
}

bool GetServerAvailableStoryDialogueChoiceSnapshots(
    const std::string& playerKey,
    int storyId,
    int playerLevel,
    int legacyJobCode,
    std::vector<story::StoryDialogueChoiceSnapshot>& choices)
{
    ServerProgressionConditionContext context =
        MakeServerProgressionConditionContext(playerKey, playerLevel, legacyJobCode);
    return Storage().storyService.GetAvailableDialogueChoiceSnapshots(
        playerKey, storyId, context, choices);
}

story::StoryEffectReceiptLoadResult GetServerStoryEffectReceiptStatus(
    const std::string& playerKey,
    int storyId,
    int nodeId,
    const std::string& effectKey,
    story::StoryEffectReceiptStatus& status)
{
    return Storage().storyService.GetEffectReceiptStatus(
        playerKey, storyId, nodeId, effectKey, status);
}

bool GetServerPendingStoryEffects(
    const std::string& playerKey,
    std::vector<story::StoryEffectReceiptEntry>& receipts)
{
    return Storage().storyService.ListPendingEffects(playerKey, receipts);
}

bool AcknowledgeServerStoryPendingEffect(const std::string& playerKey,
                                         int storyId,
                                         int nodeId,
                                         const std::string& effectKey)
{
    return Storage().storyService.AcknowledgePendingEffect(
        playerKey, storyId, nodeId, effectKey);
}

covenants::CovenantEffectReceiptLoadResult GetServerCovenantEffectReceiptStatus(
    const std::string& playerKey,
    int covenantId,
    const std::string& effectKey,
    covenants::CovenantEffectReceiptStatus& status)
{
    return Storage().covenantRewardProcessor.GetReceiptStatus(
        playerKey, covenantId, effectKey, status);
}

bool GetServerPendingCovenantEffects(
    const std::string& playerKey,
    std::vector<covenants::CovenantEffectReceiptEntry>& receipts)
{
    return Storage().covenantRewardProcessor.ListPendingEffects(playerKey, receipts);
}

bool AcknowledgeServerCovenantPendingEffect(const std::string& playerKey,
                                             int covenantId,
                                             const std::string& effectKey)
{
    return Storage().covenantRewardProcessor.AcknowledgePendingEffect(
        playerKey, covenantId, effectKey);
}

bool ValidateServerProgressionDefinitions(std::string* error)
{
    if (!Storage().reputationService.ValidateDefinitions(error))
        return false;
    if (!Storage().covenantService.ValidateDefinitions(error))
        return false;
    if (!Storage().covenantService.ValidateRewards(Storage().rewardService, error))
        return false;
    return Storage().storyService.ValidateAllStories(error);
}

bool IsServerProgressionRuntimeReady()
{
    return InitializationState() == ServerProgressionRuntimeState::Ready;
}

ServerProgressionRuntimeStatus GetServerProgressionRuntimeStatus()
{
    ServerProgressionRuntimeStatus status;
    status.state = InitializationState();
    status.failure = InitializationFailure();
    status.reputationDefinitionCount =
        static_cast<int>(Storage().reputationService.ReputationKeys().size());
    status.covenantDefinitionCount =
        static_cast<int>(Storage().covenantService.CovenantIds().size());
    status.storyDefinitionCount =
        static_cast<int>(Storage().storyService.StoryIds().size());
    return status;
}

bool InitializeServerProgressionRuntime(std::string* error)
{
    ServerProgressionRuntimeState& state = InitializationState();
    std::string& failure = InitializationFailure();

    if (state == ServerProgressionRuntimeState::Ready ||
        state == ServerProgressionRuntimeState::CatalogLoaded)
        return true;
    if (state == ServerProgressionRuntimeState::Failed)
    {
        if (error) *error = failure;
        return false;
    }

    if (!Storage().covenantRepository.Ready())
        failure = "Covenant SQLite repository is not ready";
    else if (!Storage().reputationRepository.Ready())
        failure = "Generic Reputation SQLite repository is not ready";
    else if (!Storage().storyRepository.Ready())
        failure = "Story SQLite repository is not ready";
    else
    {
        ProgressionContentCatalog catalog;
        SQLiteProgressionContentLoader loader;
        if (!loader.Load(catalog, &failure))
        {
            // failure already populated by the loader.
        }
        else if (!ValidateCustomConditionCatalog(catalog, Storage().customHandlers, &failure))
        {
            // Custom conditions are code-registered capabilities, never arbitrary content strings.
        }
        else if (!Storage().reputationService.RegisterDefinitions(catalog.reputations))
        {
            failure = "failed to register generic Reputation definitions";
        }
        else if (!ProgressionContentRegistrar::Register(
                     catalog, Storage().covenantService, Storage().storyService, &failure))
        {
            // failure already populated by the registrar/definition validator.
        }
        else if (!ValidateServerProgressionDefinitions(&failure))
        {
            // Keep a stable startup failure instead of entering gameplay partially configured.
        }
        else
        {
            // The content catalog is structurally valid, but Item rewards cannot
            // be verified against the authoritative item catalog until InitItems().
            // Keep runtime mutation/triggers disabled until the second startup gate.
            state = ServerProgressionRuntimeState::CatalogLoaded;
            return true;
        }
    }

    if (failure.empty())
        failure = "Progression V2 initialization failed";
    state = ServerProgressionRuntimeState::Failed;
    if (error) *error = failure;
    return false;
}

bool FinalizeServerProgressionRuntimeAfterItems(std::string* error)
{
    ServerProgressionRuntimeState& state = InitializationState();
    std::string& failure = InitializationFailure();

    if (state == ServerProgressionRuntimeState::Ready)
        return true;
    if (state == ServerProgressionRuntimeState::Failed)
    {
        if (error) *error = failure;
        return false;
    }
    if (state != ServerProgressionRuntimeState::CatalogLoaded)
    {
        failure = "Progression V2 content catalog was not initialized before item finalization";
        state = ServerProgressionRuntimeState::Failed;
        if (error) *error = failure;
        return false;
    }

    Storage().rewardGateway.SetItemCatalogReady(true);
    if (!ValidateServerProgressionDefinitions(&failure))
    {
        if (failure.empty())
            failure = "Progression V2 post-item validation failed";
        state = ServerProgressionRuntimeState::Failed;
        if (error) *error = failure;
        return false;
    }

    state = ServerProgressionRuntimeState::Ready;
    RegisterServerProgressionEventTriggers();
    return true;
}

}
}
