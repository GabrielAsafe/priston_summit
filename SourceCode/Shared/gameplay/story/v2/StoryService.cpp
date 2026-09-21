#include "gameplay/story/v2/StoryService.h"

#include <set>
#include <sstream>

namespace gameplay
{
namespace story
{
namespace
{
bool SameProgressState(const StoryProgressState& left, const StoryProgressState& right)
{
    return left.storyId == right.storyId && left.chapterId == right.chapterId &&
           left.nodeId == right.nodeId && left.status == right.status &&
           left.enterActionsApplied == right.enterActionsApplied &&
           left.completionActionsApplied == right.completionActionsApplied;
}

bool ValidateConditionList(const std::vector<conditions::ConditionDefinition>& definitions)
{
    return conditions::ConditionService::ValidateAll(definitions);
}
}

StoryService::StoryService(IStoryRepository& repository,
                           IStoryCommandGateway& commands,
                           rewards::RewardService& rewards,
                           events::GameplayEventHub* eventHub)
    : repository_(repository), commands_(commands), rewards_(rewards), eventHub_(eventHub)
{
}

bool StoryService::RegisterStory(const StoryDefinition& definition)
{
    const int startMode = static_cast<int>(definition.startMode);
    if (definition.id <= 0 || definition.key.empty() || definition.key.size() > 255u ||
        definition.displayNameKey.size() > 255u || startMode < 0 || startMode > 1 ||
        definition.initialChapterId <= 0 || definition.entryNodeId <= 0)
        return false;

    for (StoryMap::const_iterator it = stories_.begin(); it != stories_.end(); ++it)
        if (it->second.key == definition.key)
            return false;

    return stories_.insert(std::make_pair(definition.id, definition)).second;
}

bool StoryService::RegisterArc(const StoryArcDefinition& definition)
{
    if (!FindStory(definition.storyId) || definition.id <= 0 || definition.key.empty() ||
        definition.key.size() > 255u || definition.displayNameKey.size() > 255u)
        return false;

    ArcMap& arcs = arcs_[definition.storyId];
    for (ArcMap::const_iterator it = arcs.begin(); it != arcs.end(); ++it)
        if (it->second.key == definition.key)
            return false;
    return arcs.insert(std::make_pair(definition.id, definition)).second;
}

bool StoryService::RegisterChapter(const StoryChapterDefinition& definition)
{
    if (!FindStory(definition.storyId) || definition.id <= 0 || definition.key.empty() ||
        definition.key.size() > 255u || definition.displayNameKey.size() > 255u)
        return false;
    if (definition.arcId > 0 && !FindArc(definition.storyId, definition.arcId))
        return false;

    ChapterMap& chapters = chapters_[definition.storyId];
    for (ChapterMap::const_iterator it = chapters.begin(); it != chapters.end(); ++it)
        if (it->second.key == definition.key)
            return false;
    return chapters.insert(std::make_pair(definition.id, definition)).second;
}

bool StoryService::RegisterNode(StoryId storyId, const StoryNodeDefinition& node)
{
    if (!FindStory(storyId) || node.id <= 0 || node.key.empty() || node.key.size() > 255u ||
        node.chapterId <= 0 || node.dialogueKey.size() > 255u ||
        node.speakerKey.size() > 255u || node.questId < 0 || node.covenantId < 0 ||
        !FindChapter(storyId, node.chapterId))
        return false;

    NodeMap& nodes = nodes_[storyId];
    for (NodeMap::const_iterator it = nodes.begin(); it != nodes.end(); ++it)
        if (it->second.key == node.key)
            return false;
    return nodes.insert(std::make_pair(node.id, node)).second;
}

const StoryDefinition* StoryService::FindStory(StoryId storyId) const
{
    StoryMap::const_iterator it = stories_.find(storyId);
    return it == stories_.end() ? 0 : &it->second;
}

const StoryArcDefinition* StoryService::FindArc(StoryId storyId, StoryArcId arcId) const
{
    StoryArcMap::const_iterator storyIt = arcs_.find(storyId);
    if (storyIt == arcs_.end())
        return 0;
    ArcMap::const_iterator arcIt = storyIt->second.find(arcId);
    return arcIt == storyIt->second.end() ? 0 : &arcIt->second;
}

const StoryChapterDefinition* StoryService::FindChapter(StoryId storyId,
                                                        StoryChapterId chapterId) const
{
    StoryChapterMap::const_iterator storyIt = chapters_.find(storyId);
    if (storyIt == chapters_.end())
        return 0;
    ChapterMap::const_iterator chapterIt = storyIt->second.find(chapterId);
    return chapterIt == storyIt->second.end() ? 0 : &chapterIt->second;
}

const StoryNodeDefinition* StoryService::FindNode(StoryId storyId, StoryNodeId nodeId) const
{
    StoryNodeMap::const_iterator storyIt = nodes_.find(storyId);
    if (storyIt == nodes_.end())
        return 0;
    NodeMap::const_iterator nodeIt = storyIt->second.find(nodeId);
    return nodeIt == storyIt->second.end() ? 0 : &nodeIt->second;
}

bool StoryService::ValidateStory(StoryId storyId, std::string* error) const
{
    const StoryDefinition* story = FindStory(storyId);
    if (!story)
    {
        if (error) *error = "story not registered";
        return false;
    }

    if (!ValidateConditionList(story->startConditions))
    {
        if (error) *error = "story has an invalid start condition";
        return false;
    }

    const StoryChapterDefinition* initialChapter = FindChapter(storyId, story->initialChapterId);
    if (!initialChapter)
    {
        if (error) *error = "initial chapter not registered";
        return false;
    }

    const StoryNodeDefinition* entry = FindNode(storyId, story->entryNodeId);
    if (!entry)
    {
        if (error) *error = "entry node not registered";
        return false;
    }
    if (entry->chapterId != initialChapter->id)
    {
        if (error) *error = "entry node is not in the initial chapter";
        return false;
    }

    StoryNodeMap::const_iterator storyNodes = nodes_.find(storyId);
    if (storyNodes == nodes_.end())
    {
        if (error) *error = "story has no nodes";
        return false;
    }

    for (NodeMap::const_iterator nodeIt = storyNodes->second.begin();
         nodeIt != storyNodes->second.end(); ++nodeIt)
    {
        const StoryNodeDefinition& node = nodeIt->second;
        const StoryChapterDefinition* chapter = FindChapter(storyId, node.chapterId);
        if (!chapter)
        {
            if (error) *error = "node references an unknown chapter";
            return false;
        }
        if (!ValidateConditionList(chapter->entryConditions) ||
            !ValidateConditionList(node.entryConditions) ||
            !ValidateConditionList(node.completionConditions))
        {
            if (error) *error = "story contains an invalid condition";
            return false;
        }
        if (!node.dialogueChoices.empty() && !node.transitions.empty())
        {
            if (error) *error = "story node cannot mix automatic transitions and dialogue choices";
            return false;
        }
        std::set<std::string> optionKeys;
        for (std::vector<std::string>::const_iterator option = node.dialogueOptionKeys.begin();
             option != node.dialogueOptionKeys.end(); ++option)
        {
            if (option->empty() || option->size() > 255u || !optionKeys.insert(*option).second)
            {
                if (error) *error = "story contains an invalid dialogue option key";
                return false;
            }
        }
        std::set<std::string> choiceKeys;
        for (std::vector<StoryDialogueChoiceDefinition>::const_iterator choice =
                 node.dialogueChoices.begin(); choice != node.dialogueChoices.end(); ++choice)
        {
            if (choice->key.empty() || choice->key.size() > 255u ||
                choice->displayKey.empty() || choice->displayKey.size() > 255u ||
                !choiceKeys.insert(choice->key).second ||
                choice->targetNodeId <= 0 || choice->targetNodeId == node.id ||
                !FindNode(storyId, choice->targetNodeId) ||
                !ValidateConditionList(choice->conditions))
            {
                if (error) *error = "story contains an invalid dialogue choice";
                return false;
            }
        }
        const std::vector<StoryActionDefinition>* actionLists[] =
        {
            &node.onEnterActions, &node.onCompleteActions
        };
        for (std::size_t listIndex = 0; listIndex < 2; ++listIndex)
        {
            for (std::vector<StoryActionDefinition>::const_iterator action = actionLists[listIndex]->begin();
                 action != actionLists[listIndex]->end(); ++action)
            {
                bool valid = false;
                switch (action->type)
                {
                case StoryActionType::SetPlayerFlag:
                case StoryActionType::SetWorldFlag:
                    valid = !action->key.empty() && action->key.size() <= 255u;
                    break;
                case StoryActionType::ActivateNextNode:
                    valid = true;
                    break;
                case StoryActionType::StartQuest:
                    valid = action->id > 0 && commands_.ValidateAction(*action);
                    break;
                case StoryActionType::AddCovenantReputation:
                    valid = action->id > 0 && commands_.ValidateAction(*action);
                    break;
                case StoryActionType::Custom:
                    valid = action->key.size() <= 255u && commands_.ValidateAction(*action);
                    break;
                }
                if (!valid)
                {
                    if (error) *error = "story contains an unsupported or malformed action";
                    return false;
                }
            }
        }
        for (std::vector<rewards::RewardDefinition>::const_iterator reward = node.completionRewards.begin();
             reward != node.completionRewards.end(); ++reward)
        {
            if (!rewards_.Supports(*reward) || !rewards_.Validate(*reward))
            {
                if (error) *error = "story contains an unsupported reward";
                return false;
            }
            bool valid = false;
            switch (reward->type)
            {
            case rewards::RewardType::Item:
                valid = !reward->key.empty() && reward->key.size() <= 255u && reward->amount > 0;
                break;
            case rewards::RewardType::Gold:
            case rewards::RewardType::Experience:
                valid = reward->amount > 0;
                break;
            case rewards::RewardType::Reputation:
                valid = !reward->key.empty() && reward->key.size() <= 255u &&
                        reward->id == 0 && reward->amount != 0;
                break;
            case rewards::RewardType::StoryFlag:
                valid = !reward->key.empty() && reward->key.size() <= 255u;
                break;
            case rewards::RewardType::CovenantReputation:
                valid = reward->id > 0;
                break;
            case rewards::RewardType::Custom:
                valid = reward->key.size() <= 255u && (!reward->key.empty() || reward->id > 0);
                break;
            }
            if (!valid)
            {
                if (error) *error = "story contains a malformed reward";
                return false;
            }
        }
        for (std::vector<StoryNodeId>::const_iterator target = node.transitions.begin();
             target != node.transitions.end(); ++target)
        {
            if (*target <= 0 || *target == node.id || !FindNode(storyId, *target))
            {
                if (error) *error = "node transition references an invalid target";
                return false;
            }
        }
    }
    return true;
}

bool StoryService::ValidateAllStories(std::string* error) const
{
    for (StoryMap::const_iterator it = stories_.begin(); it != stories_.end(); ++it)
        if (!ValidateStory(it->first, error))
            return false;
    return true;
}

std::vector<StoryId> StoryService::StoryIds() const
{
    std::vector<StoryId> ids;
    ids.reserve(stories_.size());
    for (StoryMap::const_iterator it = stories_.begin(); it != stories_.end(); ++it)
        ids.push_back(it->first);
    return ids;
}

bool StoryService::IsAutomaticStart(StoryId storyId) const
{
    const StoryDefinition* story = FindStory(storyId);
    return story && story->startMode == StoryStartMode::Automatic;
}

void StoryService::GetCatalog(std::vector<StoryCatalogEntry>& entries) const
{
    entries.clear();
    entries.reserve(stories_.size());
    for (StoryMap::const_iterator it = stories_.begin(); it != stories_.end(); ++it)
    {
        StoryCatalogEntry entry;
        entry.id = it->second.id;
        entry.key = it->second.key;
        entry.displayNameKey = it->second.displayNameKey;
        entry.startMode = it->second.startMode;
        entries.push_back(entry);
    }
}

bool StoryService::CanEnterNode(StoryId storyId,
                                const StoryNodeDefinition& node,
                                const conditions::ConditionContext& context) const
{
    const StoryChapterDefinition* chapter = FindChapter(storyId, node.chapterId);
    return chapter &&
           conditions::ConditionService::EvaluateAll(chapter->entryConditions, context) &&
           conditions::ConditionService::EvaluateAll(node.entryConditions, context);
}

std::string StoryService::SourceKey(StoryId storyId, StoryNodeId nodeId, const char* phase) const
{
    std::ostringstream stream;
    stream << "story." << storyId << ".node." << nodeId << "." << phase;
    return stream.str();
}

std::string StoryService::IndexedEffectKey(const char* phase, std::size_t index) const
{
    std::ostringstream stream;
    stream << phase << "." << index;
    return stream.str();
}

bool StoryService::IsActionRetrySafe(const StoryActionDefinition& action) const
{
    switch (action.type)
    {
    case StoryActionType::SetPlayerFlag:
    case StoryActionType::SetWorldFlag:
    case StoryActionType::ActivateNextNode:
        return true;
    case StoryActionType::StartQuest:
    case StoryActionType::AddCovenantReputation:
    case StoryActionType::Custom:
        return commands_.IsRetrySafe(action);
    }
    return false;
}

bool StoryService::ApplyActions(const std::string& playerKey,
                                StoryId storyId,
                                StoryNodeId nodeId,
                                const std::vector<StoryActionDefinition>& actions,
                                const char* phase)
{
    const std::string sourcePrefix = SourceKey(storyId, nodeId, phase);
    for (std::size_t index = 0; index < actions.size(); ++index)
    {
        const StoryActionType type = actions[index].type;
        const bool gatewayAction = type == StoryActionType::StartQuest ||
                                   type == StoryActionType::AddCovenantReputation ||
                                   type == StoryActionType::Custom;
        if (gatewayAction && !commands_.Supports(type))
            return false;
    }

    for (std::size_t index = 0; index < actions.size(); ++index)
    {
        const StoryActionDefinition& action = actions[index];
        const std::string effectKey = IndexedEffectKey(phase, index);
        const StoryEffectClaimResult claim =
            repository_.ClaimEffect(playerKey, storyId, nodeId, effectKey);
        if (claim == StoryEffectClaimResult::AlreadyApplied)
            continue;
        if (claim == StoryEffectClaimResult::Pending && !IsActionRetrySafe(action))
            return false;
        if (claim != StoryEffectClaimResult::Claimed && claim != StoryEffectClaimResult::Pending)
            return false;

        std::ostringstream source;
        source << sourcePrefix << "." << index;
        bool ok = false;
        switch (action.type)
        {
        case StoryActionType::SetPlayerFlag:
            ok = !action.key.empty() && repository_.SetPlayerFlag(playerKey, action.key, action.value);
            break;
        case StoryActionType::SetWorldFlag:
            ok = !action.key.empty() && repository_.SetWorldFlag(action.key, action.value);
            break;
        case StoryActionType::StartQuest:
            ok = action.id > 0 && commands_.StartQuest(playerKey, action.id);
            break;
        case StoryActionType::AddCovenantReputation:
            ok = action.id > 0 && commands_.AddCovenantReputation(playerKey, action.id,
                                                                   action.value, source.str());
            break;
        case StoryActionType::ActivateNextNode:
            ok = true;
            break;
        case StoryActionType::Custom:
            ok = commands_.ExecuteCustomAction(playerKey, action, source.str());
            break;
        }
        if (!ok || !repository_.MarkEffectApplied(playerKey, storyId, nodeId, effectKey))
            return false;

        if (action.type == StoryActionType::SetWorldFlag && eventHub_)
        {
            StoryWorldFlagChangedEvent event;
            event.flagKey = action.key;
            event.value = action.value;
            eventHub_->Publish(event);
        }
    }
    return true;
}

bool StoryService::ApplyRewards(const std::string& playerKey,
                                StoryId storyId,
                                StoryNodeId nodeId,
                                const std::vector<rewards::RewardDefinition>& rewardDefinitions)
{
    const std::string sourcePrefix = SourceKey(storyId, nodeId, "reward");
    for (std::size_t index = 0; index < rewardDefinitions.size(); ++index)
        if (!rewards_.Supports(rewardDefinitions[index]))
            return false;

    for (std::size_t index = 0; index < rewardDefinitions.size(); ++index)
    {
        const std::string effectKey = IndexedEffectKey("reward", index);
        const StoryEffectClaimResult claim =
            repository_.ClaimEffect(playerKey, storyId, nodeId, effectKey);
        if (claim == StoryEffectClaimResult::AlreadyApplied)
            continue;
        if (claim == StoryEffectClaimResult::Pending && !rewards_.IsRetrySafe(rewardDefinitions[index]))
            return false;
        if (claim != StoryEffectClaimResult::Claimed && claim != StoryEffectClaimResult::Pending)
            return false;

        std::ostringstream source;
        source << sourcePrefix << "." << index;
        if (rewards_.Grant(playerKey, rewardDefinitions[index], source.str()) !=
            rewards::RewardResult::Granted)
            return false;
        if (!repository_.MarkEffectApplied(playerKey, storyId, nodeId, effectKey))
            return false;
    }
    return true;
}

bool StoryService::EnsureEnterActions(const std::string& playerKey,
                                      StoryId storyId,
                                      const StoryNodeDefinition& node,
                                      StoryProgressState& state)
{
    if (state.enterActionsApplied)
        return true;
    if (!ApplyActions(playerKey, storyId, node.id, node.onEnterActions, "enter"))
        return false;

    const StoryProgressState expected = state;
    StoryProgressState updated = state;
    updated.enterActionsApplied = true;
    const StoryMutationResult result =
        repository_.SaveProgressIfUnchanged(playerKey, expected, updated);
    if (result != StoryMutationResult::Applied)
        return false;
    state = updated;
    return true;
}

bool StoryService::EnsureCompletionActions(const std::string& playerKey,
                                           StoryId storyId,
                                           const StoryNodeDefinition& node,
                                           StoryProgressState& state)
{
    if (state.completionActionsApplied)
        return true;
    if (!ApplyActions(playerKey, storyId, node.id, node.onCompleteActions, "complete"))
        return false;
    if (!ApplyRewards(playerKey, storyId, node.id, node.completionRewards))
        return false;

    const StoryProgressState expected = state;
    StoryProgressState updated = state;
    updated.completionActionsApplied = true;
    const StoryMutationResult result =
        repository_.SaveProgressIfUnchanged(playerKey, expected, updated);
    if (result != StoryMutationResult::Applied)
        return false;
    state = updated;
    return true;
}

StoryService::TransitionAttemptResult StoryService::TryTransition(
    const std::string& playerKey,
    StoryId storyId,
    const StoryNodeDefinition& node,
    StoryProgressState& state,
    const conditions::ConditionContext& context)
{
    for (std::vector<StoryNodeId>::const_iterator it = node.transitions.begin();
         it != node.transitions.end(); ++it)
    {
        const StoryNodeDefinition* next = FindNode(storyId, *it);
        if (!next || !CanEnterNode(storyId, *next, context))
            continue;

        StoryProgressState nextState;
        nextState.storyId = storyId;
        nextState.chapterId = next->chapterId;
        nextState.nodeId = next->id;
        nextState.status = StoryNodeStatus::Active;
        nextState.enterActionsApplied = false;
        nextState.completionActionsApplied = false;

        const bool wasAlreadyCompleted = state.status == StoryNodeStatus::Completed;
        const StoryMutationResult mutation =
            repository_.TransitionIfUnchanged(playerKey, state, nextState);
        if (mutation == StoryMutationResult::Conflict)
            return TransitionAttemptResult::Conflict;
        if (mutation != StoryMutationResult::Applied)
            return TransitionAttemptResult::Failed;

        if (eventHub_)
        {
            if (!wasAlreadyCompleted)
            {
                StoryNodeCompletedEvent completedEvent;
                completedEvent.playerKey = playerKey;
                completedEvent.storyId = storyId;
                completedEvent.chapterId = node.chapterId;
                completedEvent.nodeId = node.id;
                eventHub_->Publish(completedEvent);
            }

            StoryNodeEnteredEvent enteredEvent;
            enteredEvent.playerKey = playerKey;
            enteredEvent.storyId = storyId;
            enteredEvent.chapterId = next->chapterId;
            enteredEvent.nodeId = next->id;
            eventHub_->Publish(enteredEvent);
        }

        state = nextState;
        if (!EnsureEnterActions(playerKey, storyId, *next, state))
            return TransitionAttemptResult::Failed;
        return TransitionAttemptResult::Transitioned;
    }
    return TransitionAttemptResult::NoEligibleTarget;
}

StoryStartResult StoryService::StartDetailed(const std::string& playerKey,
                                                   StoryId storyId,
                                                   const conditions::ConditionContext& context)
{
    const StoryDefinition* story = FindStory(storyId);
    if (!story || playerKey.empty() || !ValidateStory(storyId))
        return StoryStartResult::Failed;

    StoryProgressState existing;
    const StoryProgressLoadResult loadResult =
        repository_.LoadProgressResult(playerKey, storyId, existing);
    if (loadResult == StoryProgressLoadResult::Found)
        return StoryStartResult::AlreadyStarted;
    if (loadResult == StoryProgressLoadResult::Failed)
        return StoryStartResult::Failed;
    if (!conditions::ConditionService::EvaluateAll(story->startConditions, context))
        return StoryStartResult::NotEligible;

    const StoryNodeDefinition* entry = FindNode(storyId, story->entryNodeId);
    if (!entry)
        return StoryStartResult::Failed;
    if (!CanEnterNode(storyId, *entry, context))
        return StoryStartResult::NotEligible;

    StoryProgressState state;
    state.storyId = storyId;
    state.chapterId = entry->chapterId;
    state.nodeId = entry->id;
    state.status = StoryNodeStatus::Active;
    state.enterActionsApplied = false;
    state.completionActionsApplied = false;

    const StoryMutationResult created = repository_.CreateInitialProgressIfAbsent(playerKey, state);
    if (created == StoryMutationResult::AlreadyExists || created == StoryMutationResult::Conflict)
        return StoryStartResult::AlreadyStarted;
    if (created != StoryMutationResult::Applied)
        return StoryStartResult::Failed;

    if (eventHub_)
    {
        StoryStartedEvent startedEvent;
        startedEvent.playerKey = playerKey;
        startedEvent.storyId = storyId;
        startedEvent.chapterId = entry->chapterId;
        startedEvent.nodeId = entry->id;
        eventHub_->Publish(startedEvent);

        StoryNodeEnteredEvent enteredEvent;
        enteredEvent.playerKey = playerKey;
        enteredEvent.storyId = storyId;
        enteredEvent.chapterId = entry->chapterId;
        enteredEvent.nodeId = entry->id;
        eventHub_->Publish(enteredEvent);
    }

    return EnsureEnterActions(playerKey, storyId, *entry, state)
               ? StoryStartResult::Started
               : StoryStartResult::Failed;
}

bool StoryService::Start(const std::string& playerKey,
                         StoryId storyId,
                         const conditions::ConditionContext& context)
{
    return StartDetailed(playerKey, storyId, context) == StoryStartResult::Started;
}

StoryEvaluateResult StoryService::EvaluateDetailed(
    const std::string& playerKey,
    StoryId storyId,
    const conditions::ConditionContext& context)
{
    StoryProgressState state;
    const StoryProgressLoadResult loaded = repository_.LoadProgressResult(playerKey, storyId, state);
    if (loaded == StoryProgressLoadResult::NotFound)
        return StoryEvaluateResult::NoChange;
    if (loaded != StoryProgressLoadResult::Found)
        return StoryEvaluateResult::Failed;

    const StoryNodeDefinition* node = FindNode(storyId, state.nodeId);
    if (!node)
        return StoryEvaluateResult::Failed;

    if (state.status == StoryNodeStatus::Completed)
    {
        if (!node->dialogueChoices.empty())
            return StoryEvaluateResult::NoChange;
        const StoryProgressState before = state;
        const TransitionAttemptResult transition =
            TryTransition(playerKey, storyId, *node, state, context);
        if (transition == TransitionAttemptResult::Transitioned)
            return StoryEvaluateResult::Advanced;
        if (transition == TransitionAttemptResult::NoEligibleTarget)
            return StoryEvaluateResult::NoChange;
        if (transition == TransitionAttemptResult::Conflict)
        {
            StoryProgressState current;
            if (repository_.LoadProgressResult(playerKey, storyId, current) !=
                StoryProgressLoadResult::Found)
                return StoryEvaluateResult::Failed;
            return SameProgressState(current, before) ? StoryEvaluateResult::NoChange
                                                      : StoryEvaluateResult::Advanced;
        }
        return StoryEvaluateResult::Failed;
    }
    if (state.status != StoryNodeStatus::Active)
        return StoryEvaluateResult::Failed;

    StoryProgressState beforeEnter = state;
    if (!EnsureEnterActions(playerKey, storyId, *node, state))
    {
        StoryProgressState current;
        if (repository_.LoadProgressResult(playerKey, storyId, current) == StoryProgressLoadResult::Found &&
            !SameProgressState(current, beforeEnter))
            return StoryEvaluateResult::Advanced;
        return StoryEvaluateResult::Failed;
    }
    if (!conditions::ConditionService::EvaluateAll(node->completionConditions, context))
        return StoryEvaluateResult::NoChange;

    StoryProgressState beforeCompletionEffects = state;
    if (!EnsureCompletionActions(playerKey, storyId, *node, state))
    {
        StoryProgressState current;
        if (repository_.LoadProgressResult(playerKey, storyId, current) == StoryProgressLoadResult::Found &&
            !SameProgressState(current, beforeCompletionEffects))
            return StoryEvaluateResult::Advanced;
        return StoryEvaluateResult::Failed;
    }

    const StoryProgressState completedExpected = state;
    if (node->dialogueChoices.empty())
    {
        const TransitionAttemptResult transition =
            TryTransition(playerKey, storyId, *node, state, context);
        if (transition == TransitionAttemptResult::Transitioned)
            return StoryEvaluateResult::Advanced;
        if (transition == TransitionAttemptResult::Conflict)
        {
            StoryProgressState current;
            if (repository_.LoadProgressResult(playerKey, storyId, current) !=
                StoryProgressLoadResult::Found)
                return StoryEvaluateResult::Failed;
            return SameProgressState(current, completedExpected) ? StoryEvaluateResult::NoChange
                                                                 : StoryEvaluateResult::Advanced;
        }
        if (transition == TransitionAttemptResult::Failed)
            return StoryEvaluateResult::Failed;
    }

    StoryProgressState completed = completedExpected;
    completed.status = StoryNodeStatus::Completed;
    const StoryMutationResult completion =
        repository_.CompleteIfUnchanged(playerKey, completedExpected, completed);
    if (completion == StoryMutationResult::Conflict)
    {
        StoryProgressState current;
        if (repository_.LoadProgressResult(playerKey, storyId, current) !=
            StoryProgressLoadResult::Found)
            return StoryEvaluateResult::Failed;
        return SameProgressState(current, completedExpected) ? StoryEvaluateResult::NoChange
                                                             : StoryEvaluateResult::Advanced;
    }
    if (completion != StoryMutationResult::Applied)
        return StoryEvaluateResult::Failed;

    if (eventHub_)
    {
        StoryNodeCompletedEvent event;
        event.playerKey = playerKey;
        event.storyId = storyId;
        event.chapterId = node->chapterId;
        event.nodeId = node->id;
        eventHub_->Publish(event);

        if (node->transitions.empty() && node->dialogueChoices.empty())
        {
            StoryCompletedEvent completedEvent;
            completedEvent.playerKey = playerKey;
            completedEvent.storyId = storyId;
            completedEvent.chapterId = node->chapterId;
            completedEvent.nodeId = node->id;
            eventHub_->Publish(completedEvent);
        }
    }
    return StoryEvaluateResult::Advanced;
}

bool StoryService::Evaluate(const std::string& playerKey,
                            StoryId storyId,
                            const conditions::ConditionContext& context)
{
    return EvaluateDetailed(playerKey, storyId, context) == StoryEvaluateResult::Advanced;
}

StoryChoiceResult StoryService::ChooseDialogueOption(
    const std::string& playerKey,
    StoryId storyId,
    const std::string& choiceKey,
    const conditions::ConditionContext& context)
{
    if (playerKey.empty() || storyId <= 0 || choiceKey.empty())
        return StoryChoiceResult::Failed;

    StoryProgressState state;
    if (repository_.LoadProgressResult(playerKey, storyId, state) !=
        StoryProgressLoadResult::Found)
        return StoryChoiceResult::Failed;
    if (state.status != StoryNodeStatus::Completed)
        return StoryChoiceResult::NotAvailable;

    const StoryNodeDefinition* node = FindNode(storyId, state.nodeId);
    if (!node || node->dialogueChoices.empty())
        return StoryChoiceResult::NotAvailable;

    const StoryDialogueChoiceDefinition* selected = 0;
    for (std::vector<StoryDialogueChoiceDefinition>::const_iterator choice =
             node->dialogueChoices.begin(); choice != node->dialogueChoices.end(); ++choice)
    {
        if (choice->key == choiceKey)
        {
            selected = &*choice;
            break;
        }
    }
    if (!selected)
        return StoryChoiceResult::NotAvailable;
    if (!conditions::ConditionService::EvaluateAll(selected->conditions, context))
        return StoryChoiceResult::NotAvailable;

    const StoryNodeDefinition* next = FindNode(storyId, selected->targetNodeId);
    if (!next || !CanEnterNode(storyId, *next, context))
        return StoryChoiceResult::NotAvailable;

    StoryProgressState nextState;
    nextState.storyId = storyId;
    nextState.chapterId = next->chapterId;
    nextState.nodeId = next->id;
    nextState.status = StoryNodeStatus::Active;
    nextState.enterActionsApplied = false;
    nextState.completionActionsApplied = false;

    const StoryMutationResult mutation =
        repository_.TransitionIfUnchanged(playerKey, state, nextState);
    if (mutation == StoryMutationResult::Conflict)
        return StoryChoiceResult::Conflict;
    if (mutation != StoryMutationResult::Applied)
        return StoryChoiceResult::Failed;

    if (eventHub_)
    {
        StoryDialogueChoiceSelectedEvent choiceEvent;
        choiceEvent.playerKey = playerKey;
        choiceEvent.storyId = storyId;
        choiceEvent.chapterId = state.chapterId;
        choiceEvent.nodeId = state.nodeId;
        choiceEvent.choiceKey = selected->key;
        choiceEvent.targetNodeId = next->id;
        eventHub_->Publish(choiceEvent);

        StoryNodeEnteredEvent enteredEvent;
        enteredEvent.playerKey = playerKey;
        enteredEvent.storyId = storyId;
        enteredEvent.chapterId = next->chapterId;
        enteredEvent.nodeId = next->id;
        eventHub_->Publish(enteredEvent);
    }

    return EnsureEnterActions(playerKey, storyId, *next, nextState)
               ? StoryChoiceResult::Selected
               : StoryChoiceResult::Failed;
}

bool StoryService::GetAvailableDialogueChoiceSnapshots(
    const std::string& playerKey,
    StoryId storyId,
    const conditions::ConditionContext& context,
    std::vector<StoryDialogueChoiceSnapshot>& choices)
{
    choices.clear();
    if (playerKey.empty() || storyId <= 0)
        return false;

    StoryProgressState state;
    if (repository_.LoadProgressResult(playerKey, storyId, state) !=
        StoryProgressLoadResult::Found)
        return false;
    if (state.status != StoryNodeStatus::Completed)
        return true;

    const StoryNodeDefinition* node = FindNode(storyId, state.nodeId);
    if (!node)
        return false;

    for (std::vector<StoryDialogueChoiceDefinition>::const_iterator choice =
             node->dialogueChoices.begin(); choice != node->dialogueChoices.end(); ++choice)
    {
        const StoryNodeDefinition* target = FindNode(storyId, choice->targetNodeId);
        if (target && conditions::ConditionService::EvaluateAll(choice->conditions, context) &&
            CanEnterNode(storyId, *target, context))
        {
            StoryDialogueChoiceSnapshot snapshot;
            snapshot.key = choice->key;
            snapshot.displayKey = choice->displayKey;
            snapshot.targetNodeId = choice->targetNodeId;
            choices.push_back(snapshot);
        }
    }
    return true;
}

bool StoryService::GetAvailableDialogueChoices(
    const std::string& playerKey,
    StoryId storyId,
    const conditions::ConditionContext& context,
    std::vector<std::string>& choiceKeys)
{
    choiceKeys.clear();
    std::vector<StoryDialogueChoiceSnapshot> choices;
    if (!GetAvailableDialogueChoiceSnapshots(playerKey, storyId, context, choices))
        return false;
    for (std::vector<StoryDialogueChoiceSnapshot>::const_iterator choice = choices.begin();
         choice != choices.end(); ++choice)
        choiceKeys.push_back(choice->key);
    return true;
}

StoryProgressLoadResult StoryService::GetProgressResult(const std::string& playerKey,
                                                        StoryId storyId,
                                                        StoryProgressState& state)
{
    if (playerKey.empty() || storyId <= 0)
        return StoryProgressLoadResult::Failed;
    return repository_.LoadProgressResult(playerKey, storyId, state);
}

bool StoryService::GetProgress(const std::string& playerKey,
                               StoryId storyId,
                               StoryProgressState& state)
{
    return GetProgressResult(playerKey, storyId, state) == StoryProgressLoadResult::Found;
}

bool StoryService::BuildSnapshot(const StoryProgressState& progress,
                                 StorySnapshot& snapshot) const
{
    const StoryDefinition* story = FindStory(progress.storyId);
    const StoryChapterDefinition* chapter = FindChapter(progress.storyId, progress.chapterId);
    const StoryNodeDefinition* node = FindNode(progress.storyId, progress.nodeId);
    if (!story || !chapter || !node)
        return false;

    snapshot = StorySnapshot();
    snapshot.progress = progress;
    snapshot.storyKey = story->key;
    snapshot.storyDisplayNameKey = story->displayNameKey;
    snapshot.chapterKey = chapter->key;
    if (chapter->arcId > 0)
    {
        const StoryArcDefinition* arc = FindArc(progress.storyId, chapter->arcId);
        if (!arc)
            return false;
        snapshot.arcId = arc->id;
        snapshot.arcKey = arc->key;
    }
    snapshot.nodeKey = node->key;
    snapshot.dialogueKey = node->dialogueKey;
    snapshot.speakerKey = node->speakerKey;
    snapshot.dialogueOptionKeys = node->dialogueOptionKeys;
    for (std::vector<StoryDialogueChoiceDefinition>::const_iterator choice =
             node->dialogueChoices.begin(); choice != node->dialogueChoices.end(); ++choice)
    {
        snapshot.dialogueChoiceKeys.push_back(choice->key);
        StoryDialogueChoiceSnapshot choiceSnapshot;
        choiceSnapshot.key = choice->key;
        choiceSnapshot.displayKey = choice->displayKey;
        choiceSnapshot.targetNodeId = choice->targetNodeId;
        snapshot.dialogueChoices.push_back(choiceSnapshot);
    }
    snapshot.questId = node->questId;
    snapshot.covenantId = node->covenantId;
    if (progress.status == StoryNodeStatus::Completed)
    {
        snapshot.awaitingChoice = !node->dialogueChoices.empty();
        snapshot.awaitingTransition = node->dialogueChoices.empty() && !node->transitions.empty();
        snapshot.terminal = node->dialogueChoices.empty() && node->transitions.empty();
    }
    return true;
}

bool StoryService::GetSnapshot(const std::string& playerKey,
                               StoryId storyId,
                               StorySnapshot& snapshot)
{
    StoryProgressState progress;
    if (repository_.LoadProgressResult(playerKey, storyId, progress) !=
        StoryProgressLoadResult::Found)
        return false;
    return BuildSnapshot(progress, snapshot);
}

bool StoryService::GetStartedSnapshots(const std::string& playerKey,
                                       std::vector<StorySnapshot>& snapshots)
{
    snapshots.clear();
    if (playerKey.empty())
        return false;

    const std::vector<StoryId> ids = StoryIds();
    for (std::vector<StoryId>::const_iterator id = ids.begin(); id != ids.end(); ++id)
    {
        StoryProgressState progress;
        const StoryProgressLoadResult loaded =
            repository_.LoadProgressResult(playerKey, *id, progress);
        if (loaded == StoryProgressLoadResult::NotFound)
            continue;
        if (loaded != StoryProgressLoadResult::Found)
            return false;
        StorySnapshot snapshot;
        if (!BuildSnapshot(progress, snapshot))
            return false;
        snapshots.push_back(snapshot);
    }
    return true;
}

StoryEffectReceiptLoadResult StoryService::GetEffectReceiptStatus(
    const std::string& playerKey,
    StoryId storyId,
    StoryNodeId nodeId,
    const std::string& effectKey,
    StoryEffectReceiptStatus& status)
{
    if (playerKey.empty() || storyId <= 0 || nodeId <= 0 || effectKey.empty())
        return StoryEffectReceiptLoadResult::Failed;
    return repository_.GetEffectReceiptStatus(playerKey, storyId, nodeId, effectKey, status);
}

bool StoryService::ListPendingEffects(
    const std::string& playerKey,
    std::vector<StoryEffectReceiptEntry>& receipts)
{
    receipts.clear();
    return !playerKey.empty() && repository_.ListPendingEffects(playerKey, receipts);
}

bool StoryService::AcknowledgePendingEffect(const std::string& playerKey,
                                             StoryId storyId,
                                             StoryNodeId nodeId,
                                             const std::string& effectKey)
{
    if (playerKey.empty() || storyId <= 0 || nodeId <= 0 || effectKey.empty())
        return false;
    return repository_.AcknowledgePendingEffect(playerKey, storyId, nodeId, effectKey);
}
}
}
