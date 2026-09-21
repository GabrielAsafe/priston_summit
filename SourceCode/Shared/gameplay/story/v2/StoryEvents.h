#pragma once

#include <string>

#include "gameplay/story/v2/StoryTypes.h"

namespace gameplay
{
namespace story
{
struct StoryStartedEvent
{
    std::string playerKey;
    StoryId storyId;
    StoryChapterId chapterId;
    StoryNodeId nodeId;
};

struct StoryNodeEnteredEvent
{
    std::string playerKey;
    StoryId storyId;
    StoryChapterId chapterId;
    StoryNodeId nodeId;
};

struct StoryNodeCompletedEvent
{
    std::string playerKey;
    StoryId storyId;
    StoryChapterId chapterId;
    StoryNodeId nodeId;
};

struct StoryCompletedEvent
{
    std::string playerKey;
    StoryId storyId;
    StoryChapterId chapterId;
    StoryNodeId nodeId;
};

struct StoryDialogueChoiceSelectedEvent
{
    std::string playerKey;
    StoryId storyId;
    StoryChapterId chapterId;
    StoryNodeId nodeId;
    std::string choiceKey;
    StoryNodeId targetNodeId;
};

struct StoryWorldFlagChangedEvent
{
    std::string flagKey;
    int value;
};
}
}
