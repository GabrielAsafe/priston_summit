#pragma once

#include <string>
#include <vector>

#include "gameplay/conditions/v2/Condition.h"
#include "gameplay/rewards/v2/Reward.h"

namespace gameplay
{
namespace story
{
typedef int StoryId;
typedef int StoryArcId;
typedef int StoryChapterId;
typedef int StoryNodeId;

enum class StoryNodeStatus
{
    Locked = 0,
    Active = 1,
    Completed = 2
};

enum class StoryStartMode
{
    Automatic = 0,
    Explicit = 1
};

enum class StoryActionType
{
    SetPlayerFlag,
    SetWorldFlag,
    StartQuest,
    AddCovenantReputation,
    ActivateNextNode,
    Custom
};

struct StoryActionDefinition
{
    StoryActionType type;
    std::string key;
    int id;
    int value;

    StoryActionDefinition()
        : type(StoryActionType::Custom), id(0), value(0)
    {
    }
};

struct StoryArcDefinition
{
    StoryArcId id;
    StoryId storyId;
    std::string key;
    std::string displayNameKey;

    StoryArcDefinition() : id(0), storyId(0) {}
};

struct StoryChapterDefinition
{
    StoryChapterId id;
    StoryId storyId;
    StoryArcId arcId;
    std::string key;
    std::string displayNameKey;
    std::vector<conditions::ConditionDefinition> entryConditions;

    StoryChapterDefinition() : id(0), storyId(0), arcId(0) {}
};

struct StoryDialogueChoiceDefinition
{
    std::string key;
    std::string displayKey;
    StoryNodeId targetNodeId;
    std::vector<conditions::ConditionDefinition> conditions;

    StoryDialogueChoiceDefinition() : targetNodeId(0) {}
};

struct StoryNodeDefinition
{
    StoryNodeId id;
    StoryChapterId chapterId;
    std::string key;
    std::vector<conditions::ConditionDefinition> entryConditions;
    std::vector<StoryActionDefinition> onEnterActions;
    std::vector<conditions::ConditionDefinition> completionConditions;
    std::vector<StoryActionDefinition> onCompleteActions;
    std::vector<StoryNodeId> transitions;
    std::vector<rewards::RewardDefinition> completionRewards;
    std::string dialogueKey;
    std::string speakerKey;
    std::vector<std::string> dialogueOptionKeys;
    std::vector<StoryDialogueChoiceDefinition> dialogueChoices;
    int questId;
    int covenantId;

    StoryNodeDefinition()
        : id(0), chapterId(0), questId(0), covenantId(0)
    {
    }
};

struct StoryDefinition
{
    StoryId id;
    std::string key;
    std::string displayNameKey;
    StoryStartMode startMode;
    StoryChapterId initialChapterId;
    StoryNodeId entryNodeId;
    std::vector<conditions::ConditionDefinition> startConditions;

    StoryDefinition()
        : id(0), startMode(StoryStartMode::Automatic), initialChapterId(0), entryNodeId(0)
    {
    }
};

struct StoryProgressState
{
    StoryId storyId;
    StoryChapterId chapterId;
    StoryNodeId nodeId;
    StoryNodeStatus status;
    bool enterActionsApplied;
    bool completionActionsApplied;

    StoryProgressState()
        : storyId(0), chapterId(0), nodeId(0), status(StoryNodeStatus::Locked),
          enterActionsApplied(false), completionActionsApplied(false)
    {
    }
};

struct StoryCatalogEntry
{
    StoryId id;
    std::string key;
    std::string displayNameKey;
    StoryStartMode startMode;

    StoryCatalogEntry() : id(0), startMode(StoryStartMode::Automatic) {}
};

struct StoryDialogueChoiceSnapshot
{
    std::string key;
    std::string displayKey;
    StoryNodeId targetNodeId;

    StoryDialogueChoiceSnapshot() : targetNodeId(0) {}
};

struct StorySnapshot
{
    StoryProgressState progress;
    std::string storyKey;
    std::string storyDisplayNameKey;
    StoryArcId arcId;
    std::string arcKey;
    std::string chapterKey;
    std::string nodeKey;
    std::string dialogueKey;
    std::string speakerKey;
    std::vector<std::string> dialogueOptionKeys;
    std::vector<std::string> dialogueChoiceKeys;
    std::vector<StoryDialogueChoiceSnapshot> dialogueChoices;
    int questId;
    int covenantId;
    bool terminal;
    bool awaitingChoice;
    bool awaitingTransition;

    StorySnapshot()
        : arcId(0), questId(0), covenantId(0), terminal(false),
          awaitingChoice(false), awaitingTransition(false)
    {
    }
};
}
}
