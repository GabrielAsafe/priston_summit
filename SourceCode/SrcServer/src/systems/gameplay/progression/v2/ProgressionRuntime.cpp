#include "systems/gameplay/progression/v2/ProgressionRuntime.h"

#include <ctime>
#include <cstdlib>
#include <map>
#include <mutex>

#include "systems/gameplay/progression/v2/LegacyProgressionGateways.h"
#include "systems/gameplay/progression/v2/LegacyProgressionReaders.h"
#include "systems/gameplay/progression/v2/ProgressionTriggerRuntime.h"
#include "gameplay/progression/v2/ProgressionContent.h"
#include "gameplay/progression/v2/ProgressionTriggerCoordinator.h"
#include "systems/gameplay/progression/v2/SQLiteProgressionContentLoader.h"
#include "systems/gameplay/covenants/v2/SQLiteCovenantRepository.h"
#include "systems/gameplay/covenants/v2/SQLiteCovenantRuntimeRepository.h"
#include "gameplay/covenants/v2/CovenantCatalog.h"
#include "gameplay/covenants/v2/CovenantRuntimeTypes.h"
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
    covenants::SQLiteCovenantRuntimeRepository covenantRuntimeRepository;
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


namespace
{
struct CovenantModifierCacheEntry
{
    std::vector<modifiers::GameplayModifier> base;
    std::int64_t loadedAt;
    int covenantId;
    CovenantModifierCacheEntry() : loadedAt(0), covenantId(0) {}
};
std::map<std::string, CovenantModifierCacheEntry>& CovenantModifierCache()
{
    static std::map<std::string, CovenantModifierCacheEntry> cache;
    return cache;
}
std::mutex& CovenantModifierCacheMutex()
{
    static std::mutex mutex;
    return mutex;
}
std::int64_t CovenantNowSeconds() { return static_cast<std::int64_t>(std::time(0)); }
int CovenantSwitchCooldownSeconds()
{
    static const int seconds = []() {
        const char* raw = std::getenv("SUMMIT_COVENANT_SWITCH_COOLDOWN_SECONDS");
        if (!raw || !raw[0])
            return 300;
        const int parsed = std::atoi(raw);
        if (parsed < 0)
            return 0;
        if (parsed > 86400)
            return 86400;
        return parsed;
    }();
    return seconds;
}

bool LoadCachedBaseModifiers(const std::string& playerKey,
                             int& covenantId,
                             std::vector<modifiers::GameplayModifier>& benefits)
{
    benefits.clear(); covenantId = 0;
    const std::int64_t now = CovenantNowSeconds();
    {
        std::lock_guard<std::mutex> lock(CovenantModifierCacheMutex());
        std::map<std::string,CovenantModifierCacheEntry>::const_iterator it = CovenantModifierCache().find(playerKey);
        if (it != CovenantModifierCache().end() && now - it->second.loadedAt <= 2)
        {
            covenantId = it->second.covenantId;
            benefits = it->second.base;
            return true;
        }
    }
    covenants::CovenantMembershipState membership;
    if (!Storage().covenantService.GetActiveMembership(playerKey, membership))
        return true;
    if (!Storage().covenantService.GetAllActiveBenefits(playerKey, benefits))
        return false;
    covenantId = membership.covenantId;
    CovenantModifierCacheEntry entry; entry.base = benefits; entry.covenantId = covenantId; entry.loadedAt = now;
    std::lock_guard<std::mutex> lock(CovenantModifierCacheMutex());
    CovenantModifierCache()[playerKey] = entry;
    return true;
}

void TryGrantCovenantMastery(const std::string& playerKey, int covenantId)
{
    covenants::CovenantMembershipState membership;
    if (!Storage().covenantService.GetMembership(playerKey, covenantId, membership) || membership.rankId < 3)
        return;
    bool completed = false;
    if (!Storage().covenantRuntimeRepository.AllChallengesCompleted(playerKey, covenantId, completed) || !completed)
        return;
    (void)Storage().covenantRuntimeRepository.GrantMastery(playerKey, covenantId, CovenantNowSeconds());
}
}

void InvalidateServerCovenantModifierCache(const std::string& playerKey)
{
    std::lock_guard<std::mutex> lock(CovenantModifierCacheMutex());
    CovenantModifierCache().erase(playerKey);
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
    if (!IsServerProgressionRuntimeReady() || playerKey.empty() || covenantId <= 0)
        return false;
    ServerProgressionConditionContext context =
        MakeServerProgressionConditionContext(playerKey, playerLevel, legacyJobCode);
    const bool joined = Storage().covenantService.Join(playerKey, covenantId, context);
    if (joined)
    {
        const std::string sigilKey = std::string("sigil:") + (covenantId == 1 ? "hunters" : covenantId == 2 ? "gatherers" : covenantId == 3 ? "artisans" : covenantId == 4 ? "fortune" : "ascendants");
        (void)Storage().covenantRuntimeRepository.GrantCollectible(playerKey, covenantId, sigilKey, CovenantNowSeconds());
        (void)Storage().covenantRuntimeRepository.AddInventoryItem(playerKey, covenantId, sigilKey, 1, CovenantNowSeconds());
        InvalidateServerCovenantModifierCache(playerKey);
        (void)ProcessServerProgressionPlayer(playerKey, playerLevel, legacyJobCode);
    }
    return joined;
}

covenants::CovenantActivationResult ActivateServerCovenant(
    const std::string& playerKey,
    int covenantId,
    int playerLevel,
    int legacyJobCode)
{
    if (!IsServerProgressionRuntimeReady())
        return covenants::CovenantActivationResult::PersistenceFailure;
    covenants::CovenantMembershipState previousActive;
    if (Storage().covenantService.GetActiveMembership(playerKey, previousActive) &&
        previousActive.covenantId == covenantId)
        return covenants::CovenantActivationResult::AlreadyActive;

    std::int64_t cooldownUntil = 0;
    const std::int64_t now = CovenantNowSeconds();
    if (!Storage().covenantRuntimeRepository.GetSwitchCooldownUntil(playerKey, cooldownUntil))
        return covenants::CovenantActivationResult::PersistenceFailure;
    if (cooldownUntil > now)
        return covenants::CovenantActivationResult::CooldownActive;

    const covenants::CovenantActivationResult result =
        Storage().covenantService.Activate(playerKey, covenantId);
    if (result == covenants::CovenantActivationResult::Activated)
    {
        // Active Tier III effects belong to the specialization that produced
        // them. Switching cancels the effect immediately, but DeactivatePowers
        // preserves CooldownUntil so switching cannot reset a power cooldown.
        if (previousActive.covenantId > 0 &&
            !Storage().covenantRuntimeRepository.DeactivatePowers(
                playerKey, previousActive.covenantId))
        {
            // Membership switch committed before runtime-state writes. Compensate
            // immediately so a storage failure cannot leave the player on a new
            // specialization without the matching power/cooldown state.
            (void)Storage().covenantService.Activate(playerKey, previousActive.covenantId);
            InvalidateServerCovenantModifierCache(playerKey);
            return covenants::CovenantActivationResult::PersistenceFailure;
        }
        if (!Storage().covenantRuntimeRepository.SetSwitchCooldownUntil(
                playerKey, now + CovenantSwitchCooldownSeconds()))
        {
            if (previousActive.covenantId > 0)
                (void)Storage().covenantService.Activate(playerKey, previousActive.covenantId);
            else
                (void)Storage().covenantService.Leave(playerKey, covenantId);
            InvalidateServerCovenantModifierCache(playerKey);
            return covenants::CovenantActivationResult::PersistenceFailure;
        }
        InvalidateServerCovenantModifierCache(playerKey);
        (void)ProcessServerProgressionPlayer(playerKey, playerLevel, legacyJobCode);
    }
    return result;
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
    if (!IsServerProgressionRuntimeReady())
        return false;
    const bool applied = Storage().covenantService.AddReputation(
        playerKey, covenantId, amount, sourceKey);
    if (applied)
    {
        InvalidateServerCovenantModifierCache(playerKey);
        TryGrantCovenantMastery(playerKey, covenantId);
        (void)ProcessServerProgressionPlayer(playerKey, playerLevel, legacyJobCode);
    }
    return applied;
}

bool GetServerCovenantSnapshot(const std::string& playerKey,
                               int covenantId,
                               covenants::CovenantSnapshot& snapshot)
{
    return Storage().covenantService.GetSnapshot(playerKey, covenantId, snapshot);
}

bool GetServerActiveCovenantSnapshot(const std::string& playerKey,
                                     covenants::CovenantSnapshot& snapshot)
{
    covenants::CovenantMembershipState membership;
    return Storage().covenantService.GetActiveMembership(playerKey, membership) &&
           Storage().covenantService.GetSnapshot(playerKey, membership.covenantId, snapshot);
}

bool GetServerActiveCovenantBenefits(
    const std::string& playerKey,
    std::vector<modifiers::GameplayModifier>& benefits)
{
    return Storage().covenantService.GetAllActiveBenefits(playerKey, benefits);
}

bool GetServerCovenantModifierAggregate(
    const std::string& playerKey,
    const std::string& statKey,
    const std::string& scopeKey,
    covenants::CovenantModifierAggregate& aggregate,
    bool includeGlobalScope,
    unsigned int contextTargetSerial)
{
    aggregate = covenants::CovenantModifierAggregate();
    if (!IsServerProgressionRuntimeReady() || playerKey.empty() || statKey.empty())
        return false;
    int covenantId = 0;
    std::vector<modifiers::GameplayModifier> benefits;
    if (!LoadCachedBaseModifiers(playerKey, covenantId, benefits))
        return false;
    if (covenantId > 0)
    {
        std::vector<modifiers::GameplayModifier> power;
        if (!Storage().covenantRuntimeRepository.ListActivePowerModifiers(
                playerKey, covenantId, CovenantNowSeconds(), contextTargetSerial, power))
            return false;
        benefits.insert(benefits.end(), power.begin(), power.end());
    }
    aggregate = covenants::AggregateCovenantModifiers(benefits, statKey, scopeKey, includeGlobalScope);
    return true;
}

double ApplyServerCovenantModifierValue(
    const std::string& playerKey,
    const std::string& statKey,
    const std::string& scopeKey,
    double baseValue,
    bool includeGlobalScope,
    unsigned int contextTargetSerial)
{
    covenants::CovenantModifierAggregate aggregate;
    if (!GetServerCovenantModifierAggregate(
            playerKey, statKey, scopeKey, aggregate, includeGlobalScope,
            contextTargetSerial) || !aggregate.matched)
        return baseValue;
    return covenants::ApplyCovenantModifiers(baseValue, aggregate);
}

bool RecordServerCovenantActivity(const std::string& playerKey,
                                  const std::string& activityKey,
                                  int amount)
{
    if (!IsServerProgressionRuntimeReady() || playerKey.empty() || activityKey.empty() || amount <= 0)
        return false;
    covenants::CovenantMembershipState membership;
    if (!Storage().covenantService.GetActiveMembership(playerKey, membership))
        return false;
    int repPerUnit = 0;
    if (!Storage().covenantRuntimeRepository.GetActivityReputation(membership.covenantId, activityKey, repPerUnit))
        return false; // activity is simply not relevant to the active Covenant
    if (repPerUnit > 0 && !Storage().covenantService.AddRepeatableReputation(
            playerKey, membership.covenantId, repPerUnit * amount, activityKey))
        return false;

    std::vector<covenants::CovenantChallengeState> challenges;
    if (!Storage().covenantRuntimeRepository.IncrementChallenges(
            playerKey, membership.covenantId, activityKey, amount, challenges))
        return false;
    for (std::vector<covenants::CovenantChallengeState>::const_iterator it = challenges.begin();
         it != challenges.end(); ++it)
    {
        if (!it->completed || it->rewardClaimed || it->reputationReward <= 0)
            continue;
        const std::string source = std::string("covenant:challenge:") + it->key;
        if (Storage().covenantService.AddReputation(
                playerKey, membership.covenantId, it->reputationReward, source))
            (void)Storage().covenantRuntimeRepository.MarkChallengeRewardClaimed(
                playerKey, membership.covenantId, it->key);
    }
    InvalidateServerCovenantModifierCache(playerKey);
    TryGrantCovenantMastery(playerKey, membership.covenantId);
    return true;
}

bool UseServerCovenantPower(const std::string& playerKey,
                            const std::string& powerKey,
                            unsigned int targetSerial,
                            std::string* error)
{
    if (error) error->clear();
    if (!IsServerProgressionRuntimeReady() || playerKey.empty() || powerKey.empty())
    { if(error)*error="runtime not ready"; return false; }
    covenants::CovenantMembershipState membership;
    if (!Storage().covenantService.GetActiveMembership(playerKey, membership))
    { if(error)*error="no active covenant"; return false; }
    covenants::CovenantPowerDefinition definition;
    if (!Storage().covenantRuntimeRepository.LoadPowerDefinition(membership.covenantId, powerKey, definition))
    { if(error)*error="power unavailable for active covenant"; return false; }
    if (membership.rankId < definition.requiredRankId)
    { if(error)*error="required covenant rank not reached"; return false; }
    if (definition.targetMode == covenants::CovenantPowerTargetMode::HuntTarget && targetSerial == 0)
    { if(error)*error="hunt target required"; return false; }
    covenants::CovenantPowerState state;
    if (!Storage().covenantRuntimeRepository.LoadPowerState(playerKey, membership.covenantId, powerKey, state))
    { if(error)*error="power state unavailable"; return false; }
    const std::int64_t now = CovenantNowSeconds();
    if (state.cooldownUntil > now)
    { if(error)*error="power is on cooldown"; return false; }
    state.covenantId = membership.covenantId; state.key = powerKey;
    state.activeUntil = now + definition.durationSeconds;
    state.cooldownUntil = now + definition.cooldownSeconds;
    state.targetSerial = targetSerial;
    if (!Storage().covenantRuntimeRepository.SavePowerState(playerKey, state))
    { if(error)*error="failed to persist power"; return false; }
    InvalidateServerCovenantModifierCache(playerKey);
    return true;
}

bool GetServerCovenantChallenges(const std::string& playerKey,int covenantId,
                                 std::vector<covenants::CovenantChallengeState>& states)
{ return Storage().covenantRuntimeRepository.ListChallenges(playerKey,covenantId,states); }

bool GetServerCovenantMasteries(const std::string& playerKey,std::vector<int>& ids)
{
    std::vector<covenants::CovenantId> tmp; if(!Storage().covenantRuntimeRepository.ListMasteries(playerKey,tmp))return false;
    ids.assign(tmp.begin(),tmp.end()); return true;
}
bool GetServerCovenantCollectibles(const std::string& playerKey,std::vector<std::string>& keys)
{ return Storage().covenantRuntimeRepository.ListCollectibles(playerKey,keys); }


bool AddServerCovenantInventoryItem(const std::string& playerKey,
                                    int covenantId,
                                    const std::string& itemKey,
                                    int amount)
{
    return IsServerProgressionRuntimeReady() &&
           Storage().covenantRuntimeRepository.AddInventoryItem(
               playerKey, covenantId, itemKey, amount, CovenantNowSeconds());
}

bool ConsumeServerCovenantInventoryItem(const std::string& playerKey,
                                        int covenantId,
                                        const std::string& itemKey,
                                        int amount)
{
    return IsServerProgressionRuntimeReady() &&
           Storage().covenantRuntimeRepository.ConsumeInventoryItem(
               playerKey, covenantId, itemKey, amount);
}

bool GetServerCovenantInventory(const std::string& playerKey,
                                std::vector<covenants::CovenantInventoryItem>& items)
{
    return IsServerProgressionRuntimeReady() &&
           Storage().covenantRuntimeRepository.ListInventoryItems(playerKey, items);
}

bool GetServerCovenantArtisanMode(const std::string& playerKey, int& mode)
{
    mode = 0;
    return IsServerProgressionRuntimeReady() && !playerKey.empty() &&
           Storage().covenantRuntimeRepository.GetArtisanMode(playerKey, mode);
}

bool SetServerCovenantArtisanMode(const std::string& playerKey, int mode)
{
    if (!IsServerProgressionRuntimeReady() || playerKey.empty() || mode < 0 || mode > 3)
        return false;
    return Storage().covenantRuntimeRepository.SetArtisanMode(playerKey, mode);
}

bool RefineServerCovenantTrophy(const std::string& playerKey,
                                const std::string& trophyKey,
                                std::string* refinedKey)
{
    if (refinedKey) refinedKey->clear();
    if (!IsServerProgressionRuntimeReady() || playerKey.empty() ||
        trophyKey.compare(0, 7, "trophy:") != 0)
        return false;

    covenants::CovenantMembershipState membership;
    if (!Storage().covenantService.GetActiveMembership(playerKey, membership) ||
        membership.covenantId != covenants::catalog::kHunters || membership.rankId < 2)
        return false;

    covenants::CovenantModifierAggregate enabled;
    if (!GetServerCovenantModifierAggregate(
            playerKey, "hunt.trophy_refining_enabled", "hunt", enabled) ||
        !enabled.matched || enabled.flat <= 0.0)
        return false;

    if (!Storage().covenantRuntimeRepository.ConsumeInventoryItem(
            playerKey, membership.covenantId, trophyKey, 1))
        return false;

    const std::string output = std::string("refined_") + trophyKey;
    if (!Storage().covenantRuntimeRepository.AddInventoryItem(
            playerKey, membership.covenantId, output, 1, CovenantNowSeconds()))
    {
        // Best-effort compensation keeps the operation lossless on a write failure.
        (void)Storage().covenantRuntimeRepository.AddInventoryItem(
            playerKey, membership.covenantId, trophyKey, 1, CovenantNowSeconds());
        return false;
    }
    if (refinedKey) *refinedKey = output;
    return true;
}
bool GetServerCovenantSwitchCooldown(const std::string& playerKey,long long& cooldownUntil)
{ std::int64_t v=0;const bool ok=Storage().covenantRuntimeRepository.GetSwitchCooldownUntil(playerKey,v);cooldownUntil=v;return ok; }

bool GetServerCovenantPowerState(const std::string& playerKey,
                                  const std::string& powerKey,
                                  covenants::CovenantPowerDefinition& definition,
                                  covenants::CovenantPowerState& state)
{
    if (!IsServerProgressionRuntimeReady() || playerKey.empty() || powerKey.empty())
        return false;
    covenants::CovenantMembershipState membership;
    if (!Storage().covenantService.GetActiveMembership(playerKey, membership))
        return false;
    if (!Storage().covenantRuntimeRepository.LoadPowerDefinition(
            membership.covenantId, powerKey, definition))
        return false;
    return Storage().covenantRuntimeRepository.LoadPowerState(
        playerKey, membership.covenantId, powerKey, state);
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
    else if (!Storage().covenantRuntimeRepository.Ready())
        failure = "Covenant runtime SQLite repository is not ready";
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
