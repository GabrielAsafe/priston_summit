#include "gameplay/progression/v2/ProgressionContent.h"

#include <map>
#include <set>
#include <utility>

#include "gameplay/covenants/v2/CovenantService.h"
#include "gameplay/story/v2/StoryService.h"

namespace gameplay
{
namespace progression
{
namespace
{
typedef std::pair<int, int> PairKey;

void SetError(std::string* error, const char* message)
{
    if (error)
        *error = message ? message : "progression content registration failed";
}

bool ValidateConditionForCurrentScope(const conditions::ConditionDefinition& condition,
                                      std::string* error)
{
    if (!conditions::ConditionService::Validate(condition))
    {
        SetError(error, "progression content contains an invalid condition");
        return false;
    }
    // Character/classes are intentionally frozen until the character system is rebuilt.
    if (condition.type == conditions::ConditionType::ClassIs)
    {
        SetError(error, "ClassIs content is disabled while character/classes are frozen");
        return false;
    }
    return true;
}

bool ValidateConditions(const std::vector<conditions::ConditionDefinition>& conditions,
                        std::string* error)
{
    for (std::vector<conditions::ConditionDefinition>::const_iterator it = conditions.begin();
         it != conditions.end(); ++it)
    {
        if (!ValidateConditionForCurrentScope(*it, error))
        {
            if (error && error->empty())
                *error = "progression content contains an invalid condition";
            return false;
        }
    }
    return true;
}

struct CatalogReferences
{
    std::set<std::string> reputationKeys;
    std::set<int> covenantIds;
    std::set<int> storyIds;
    std::set<PairKey> covenantRanks;
    std::set<PairKey> storyArcs;
    std::set<PairKey> storyChapters;
    std::set<PairKey> storyNodes;
};

bool BuildCatalogReferences(const ProgressionContentCatalog& catalog,
                            CatalogReferences& references,
                            std::string* error)
{
    for (std::vector<reputation::ReputationDefinition>::const_iterator reputation =
             catalog.reputations.begin(); reputation != catalog.reputations.end(); ++reputation)
    {
        if (reputation->key.empty() || reputation->key.size() > 255u ||
            reputation->displayNameKey.size() > 255u ||
            (reputation->hasBounds && (reputation->minValue > 0 || reputation->maxValue < 0 ||
                                        reputation->minValue > reputation->maxValue)) ||
            !references.reputationKeys.insert(reputation->key).second)
        {
            SetError(error, "progression content contains an invalid or duplicate reputation");
            return false;
        }
    }

    for (std::vector<CovenantContentDefinition>::const_iterator covenant = catalog.covenants.begin();
         covenant != catalog.covenants.end(); ++covenant)
    {
        if (covenant->definition.id <= 0 || covenant->definition.key.empty() ||
            !references.covenantIds.insert(covenant->definition.id).second)
        {
            SetError(error, "progression content contains an invalid or duplicate covenant");
            return false;
        }
        std::set<int> rankIds;
        for (std::vector<covenants::CovenantRankDefinition>::const_iterator rank = covenant->ranks.begin();
             rank != covenant->ranks.end(); ++rank)
        {
            if (rank->covenantId != covenant->definition.id || rank->id <= 0 ||
                !rankIds.insert(rank->id).second ||
                !references.covenantRanks.insert(PairKey(rank->covenantId, rank->id)).second)
            {
                SetError(error, "progression content contains an invalid or duplicate covenant rank");
                return false;
            }
        }
    }

    for (std::vector<StoryContentDefinition>::const_iterator storyContent = catalog.stories.begin();
         storyContent != catalog.stories.end(); ++storyContent)
    {
        const int storyId = storyContent->definition.id;
        const int startMode = static_cast<int>(storyContent->definition.startMode);
        if (storyId <= 0 || storyContent->definition.key.empty() ||
            startMode < static_cast<int>(story::StoryStartMode::Automatic) ||
            startMode > static_cast<int>(story::StoryStartMode::Explicit) ||
            !references.storyIds.insert(storyId).second)
        {
            SetError(error, "progression content contains an invalid or duplicate story");
            return false;
        }
        for (std::vector<story::StoryArcDefinition>::const_iterator arc = storyContent->arcs.begin();
             arc != storyContent->arcs.end(); ++arc)
        {
            if (arc->storyId != storyId || arc->id <= 0 ||
                !references.storyArcs.insert(PairKey(storyId, arc->id)).second)
            {
                SetError(error, "progression content contains an invalid or duplicate story arc");
                return false;
            }
        }
        for (std::vector<story::StoryChapterDefinition>::const_iterator chapter = storyContent->chapters.begin();
             chapter != storyContent->chapters.end(); ++chapter)
        {
            if (chapter->storyId != storyId || chapter->id <= 0 ||
                !references.storyChapters.insert(PairKey(storyId, chapter->id)).second)
            {
                SetError(error, "progression content contains an invalid or duplicate story chapter");
                return false;
            }
        }
        for (std::vector<story::StoryNodeDefinition>::const_iterator node = storyContent->nodes.begin();
             node != storyContent->nodes.end(); ++node)
        {
            if (node->id <= 0 ||
                !references.storyNodes.insert(PairKey(storyId, node->id)).second)
            {
                SetError(error, "progression content contains an invalid or duplicate story node");
                return false;
            }
        }
    }
    return true;
}

bool ValidateConditionReference(const conditions::ConditionDefinition& condition,
                                const CatalogReferences& references,
                                std::string* error)
{
    switch (condition.type)
    {
    case conditions::ConditionType::ReputationAtLeast:
        if (references.reputationKeys.count(condition.key) == 0u)
        {
            SetError(error, "ReputationAtLeast condition references an unknown reputation");
            return false;
        }
        break;
    case conditions::ConditionType::StoryNodeState:
        if (references.storyNodes.count(PairKey(condition.id, condition.auxiliaryId)) == 0u)
        {
            SetError(error, "StoryNodeState condition references an unknown story node");
            return false;
        }
        break;
    case conditions::ConditionType::CovenantMembership:
    case conditions::ConditionType::CovenantReputationAtLeast:
        if (references.covenantIds.count(condition.id) == 0u)
        {
            SetError(error, "condition references an unknown covenant");
            return false;
        }
        break;
    case conditions::ConditionType::CovenantRankAtLeast:
        if (references.covenantRanks.count(PairKey(condition.id, condition.value)) == 0u)
        {
            SetError(error, "CovenantRankAtLeast condition references an unknown covenant rank");
            return false;
        }
        break;
    case conditions::ConditionType::Always:
    case conditions::ConditionType::PlayerLevelAtLeast:
    case conditions::ConditionType::ClassIs:
    case conditions::ConditionType::HasStoryFlag:
    case conditions::ConditionType::HasWorldStoryFlag:
    case conditions::ConditionType::QuestState:
    case conditions::ConditionType::ProfessionLevelAtLeast:
    case conditions::ConditionType::Custom:
        break;
    }
    return true;
}

bool ValidateConditionReferences(const std::vector<conditions::ConditionDefinition>& conditions,
                                 const CatalogReferences& references,
                                 std::string* error)
{
    for (std::vector<conditions::ConditionDefinition>::const_iterator condition = conditions.begin();
         condition != conditions.end(); ++condition)
    {
        if (!ValidateConditionReference(*condition, references, error))
            return false;
    }
    return true;
}

bool ValidateRewardReference(const rewards::RewardDefinition& reward,
                             const CatalogReferences& references,
                             std::string* error)
{
    if (reward.type == rewards::RewardType::Reputation)
    {
        if (reward.key.empty() || reward.key.size() > 255u || reward.id != 0 ||
            reward.amount == 0 || references.reputationKeys.count(reward.key) == 0u)
        {
            SetError(error, "Reputation reward references an unknown reputation or invalid delta");
            return false;
        }
    }
    if (reward.type == rewards::RewardType::CovenantReputation)
    {
        if (reward.amount == 0 || references.covenantIds.count(reward.id) == 0u)
        {
            SetError(error, "CovenantReputation reward references an unknown covenant or no-op delta");
            return false;
        }
    }
    return true;
}

bool ValidateCovenantRewardReferences(const covenants::CovenantDefinition& definition,
                                       const std::vector<covenants::CovenantRankDefinition>& ranks,
                                       const CatalogReferences& references,
                                       std::string* error)
{
    for (std::vector<covenants::CovenantRewardDefinition>::const_iterator reward =
             definition.joinRewards.begin(); reward != definition.joinRewards.end(); ++reward)
        if (!ValidateRewardReference(reward->reward, references, error))
            return false;
    for (std::vector<covenants::CovenantRankDefinition>::const_iterator rank = ranks.begin();
         rank != ranks.end(); ++rank)
        for (std::vector<covenants::CovenantRewardDefinition>::const_iterator reward =
                 rank->entryRewards.begin(); reward != rank->entryRewards.end(); ++reward)
            if (!ValidateRewardReference(reward->reward, references, error))
                return false;
    return true;
}

bool ValidateCatalogScope(const ProgressionContentCatalog& catalog, std::string* error)
{
    CatalogReferences references;
    if (!BuildCatalogReferences(catalog, references, error))
        return false;

    for (std::vector<CovenantContentDefinition>::const_iterator covenant = catalog.covenants.begin();
         covenant != catalog.covenants.end(); ++covenant)
    {
        if (!ValidateConditions(covenant->definition.joinConditions, error) ||
            !ValidateConditionReferences(covenant->definition.joinConditions, references, error))
            return false;

        if (!ValidateCovenantRewardReferences(
                covenant->definition, covenant->ranks, references, error))
            return false;

        for (std::vector<int>::const_iterator storyId = covenant->definition.relatedStoryIds.begin();
             storyId != covenant->definition.relatedStoryIds.end(); ++storyId)
        {
            if (references.storyIds.count(*storyId) == 0u)
            {
                SetError(error, "covenant related story references an unknown enabled story");
                return false;
            }
        }
    }

    for (std::vector<StoryContentDefinition>::const_iterator storyContent = catalog.stories.begin();
         storyContent != catalog.stories.end(); ++storyContent)
    {
        const int storyId = storyContent->definition.id;
        if (!ValidateConditions(storyContent->definition.startConditions, error) ||
            !ValidateConditionReferences(storyContent->definition.startConditions, references, error))
            return false;
        if (references.storyChapters.count(PairKey(storyId, storyContent->definition.initialChapterId)) == 0u ||
            references.storyNodes.count(PairKey(storyId, storyContent->definition.entryNodeId)) == 0u)
        {
            SetError(error, "story initial chapter or entry node is not present in the catalog");
            return false;
        }

        for (std::vector<story::StoryChapterDefinition>::const_iterator chapter = storyContent->chapters.begin();
             chapter != storyContent->chapters.end(); ++chapter)
        {
            if (chapter->arcId > 0 && references.storyArcs.count(PairKey(storyId, chapter->arcId)) == 0u)
            {
                SetError(error, "story chapter references an unknown arc");
                return false;
            }
            if (!ValidateConditions(chapter->entryConditions, error) ||
                !ValidateConditionReferences(chapter->entryConditions, references, error))
                return false;
        }

        for (std::vector<story::StoryNodeDefinition>::const_iterator node = storyContent->nodes.begin();
             node != storyContent->nodes.end(); ++node)
        {
            if (references.storyChapters.count(PairKey(storyId, node->chapterId)) == 0u)
            {
                SetError(error, "story node references an unknown chapter");
                return false;
            }
            if (node->covenantId > 0 && references.covenantIds.count(node->covenantId) == 0u)
            {
                SetError(error, "story node references an unknown covenant");
                return false;
            }
            if (!ValidateConditions(node->entryConditions, error) ||
                !ValidateConditions(node->completionConditions, error) ||
                !ValidateConditionReferences(node->entryConditions, references, error) ||
                !ValidateConditionReferences(node->completionConditions, references, error))
                return false;

            for (std::vector<story::StoryNodeId>::const_iterator target = node->transitions.begin();
                 target != node->transitions.end(); ++target)
            {
                if (*target == node->id || references.storyNodes.count(PairKey(storyId, *target)) == 0u)
                {
                    SetError(error, "story transition references an invalid target node");
                    return false;
                }
            }

            if (!node->dialogueChoices.empty() && !node->transitions.empty())
            {
                SetError(error, "story node cannot mix automatic transitions and dialogue choices");
                return false;
            }
            std::set<std::string> choiceKeys;
            for (std::vector<story::StoryDialogueChoiceDefinition>::const_iterator choice =
                     node->dialogueChoices.begin(); choice != node->dialogueChoices.end(); ++choice)
            {
                if (choice->key.empty() || choice->key.size() > 255u ||
                    choice->displayKey.empty() || choice->displayKey.size() > 255u ||
                    !choiceKeys.insert(choice->key).second ||
                    choice->targetNodeId == node->id ||
                    references.storyNodes.count(PairKey(storyId, choice->targetNodeId)) == 0u)
                {
                    SetError(error, "story dialogue choice references an invalid target or duplicate key");
                    return false;
                }
                if (!ValidateConditions(choice->conditions, error) ||
                    !ValidateConditionReferences(choice->conditions, references, error))
                    return false;
            }

            const std::vector<story::StoryActionDefinition>* actionLists[] =
            {
                &node->onEnterActions, &node->onCompleteActions
            };
            for (std::size_t listIndex = 0; listIndex < 2; ++listIndex)
            {
                for (std::vector<story::StoryActionDefinition>::const_iterator action = actionLists[listIndex]->begin();
                     action != actionLists[listIndex]->end(); ++action)
                {
                    if (action->type == story::StoryActionType::AddCovenantReputation)
                    {
                        if (references.covenantIds.count(action->id) == 0u)
                        {
                            SetError(error, "story action references an unknown covenant");
                            return false;
                        }
                        if (action->value == 0)
                        {
                            SetError(error, "covenant reputation action cannot be a no-op");
                            return false;
                        }
                    }
                }
            }
            for (std::vector<rewards::RewardDefinition>::const_iterator reward = node->completionRewards.begin();
                 reward != node->completionRewards.end(); ++reward)
                if (!ValidateRewardReference(*reward, references, error))
                    return false;
        }
    }
    return true;
}
}

bool ProgressionContentRegistrar::Register(const ProgressionContentCatalog& catalog,
                                           covenants::CovenantService& covenantService,
                                           story::StoryService& storyService,
                                           std::string* error)
{
    if (!ValidateCatalogScope(catalog, error))
        return false;

    for (std::vector<CovenantContentDefinition>::const_iterator covenant = catalog.covenants.begin();
         covenant != catalog.covenants.end(); ++covenant)
    {
        if (!covenantService.RegisterCovenant(covenant->definition))
        {
            SetError(error, "failed to register covenant definition");
            return false;
        }
        for (std::vector<covenants::CovenantRankDefinition>::const_iterator rank = covenant->ranks.begin();
             rank != covenant->ranks.end(); ++rank)
        {
            if (!covenantService.RegisterRank(*rank))
            {
                SetError(error, "failed to register covenant rank");
                return false;
            }
        }
    }

    for (std::vector<StoryContentDefinition>::const_iterator storyContent = catalog.stories.begin();
         storyContent != catalog.stories.end(); ++storyContent)
    {
        if (!storyService.RegisterStory(storyContent->definition))
        {
            SetError(error, "failed to register story definition");
            return false;
        }
        for (std::vector<story::StoryArcDefinition>::const_iterator arc = storyContent->arcs.begin();
             arc != storyContent->arcs.end(); ++arc)
        {
            if (!storyService.RegisterArc(*arc))
            {
                SetError(error, "failed to register story arc");
                return false;
            }
        }
        for (std::vector<story::StoryChapterDefinition>::const_iterator chapter = storyContent->chapters.begin();
             chapter != storyContent->chapters.end(); ++chapter)
        {
            if (!storyService.RegisterChapter(*chapter))
            {
                SetError(error, "failed to register story chapter");
                return false;
            }
        }
        for (std::vector<story::StoryNodeDefinition>::const_iterator node = storyContent->nodes.begin();
             node != storyContent->nodes.end(); ++node)
        {
            if (!storyService.RegisterNode(storyContent->definition.id, *node))
            {
                SetError(error, "failed to register story node");
                return false;
            }
        }
    }

    if (!covenantService.ValidateDefinitions(error))
        return false;
    return storyService.ValidateAllStories(error);
}
}
}
