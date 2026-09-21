#pragma once

#include <set>
#include <string>

#include "gameplay/story/v2/StoryService.h"

namespace gameplay
{
namespace progression
{
struct ProgressionPlayerRunReport
{
    int storiesStarted;
    int storiesAdvanced;
    int storiesUnchanged;
    int advanceSteps;
    int stepLimitHits;
    int failures;

    ProgressionPlayerRunReport()
        : storiesStarted(0), storiesAdvanced(0), storiesUnchanged(0),
          advanceSteps(0), stepLimitHits(0), failures(0)
    {
    }

    bool Ok() const { return failures == 0; }
};

class ProgressionPlayerStoryRunner
{
public:
    static ProgressionPlayerRunReport Run(story::StoryService& storyService,
                                          const std::string& playerKey,
                                          const conditions::ConditionContext& context)
    {
        static const int kMaxAdvanceStepsPerStory = 64;
        static const int kMaxGlobalPasses = 64;

        ProgressionPlayerRunReport report;
        if (playerKey.empty())
        {
            report.failures = 1;
            return report;
        }

        const std::vector<story::StoryId> storyIds = storyService.StoryIds();
        std::set<story::StoryId> startedStories;
        std::set<story::StoryId> advancedStories;
        std::set<story::StoryId> changedStories;
        std::set<story::StoryId> failedStories;

        for (int pass = 0; pass < kMaxGlobalPasses; ++pass)
        {
            bool passChanged = false;
            for (std::vector<story::StoryId>::const_iterator it = storyIds.begin();
                 it != storyIds.end(); ++it)
            {
                const story::StoryId storyId = *it;
                if (failedStories.count(storyId) != 0u)
                    continue;

                if (storyService.IsAutomaticStart(storyId))
                {
                    const story::StoryStartResult start =
                        storyService.StartDetailed(playerKey, storyId, context);
                    if (start == story::StoryStartResult::Started)
                    {
                        startedStories.insert(storyId);
                        changedStories.insert(storyId);
                        passChanged = true;
                    }
                    else if (start == story::StoryStartResult::NotEligible)
                    {
                        continue;
                    }
                    else if (start == story::StoryStartResult::Failed)
                    {
                        failedStories.insert(storyId);
                        ++report.failures;
                        continue;
                    }
                }
                else
                {
                    story::StoryProgressState existing;
                    const story::StoryProgressLoadResult load =
                        storyService.GetProgressResult(playerKey, storyId, existing);
                    if (load == story::StoryProgressLoadResult::NotFound)
                        continue;
                    if (load != story::StoryProgressLoadResult::Found)
                    {
                        failedStories.insert(storyId);
                        ++report.failures;
                        continue;
                    }
                }

                int steps = 0;
                for (; steps < kMaxAdvanceStepsPerStory; ++steps)
                {
                    const story::StoryEvaluateResult evaluated =
                        storyService.EvaluateDetailed(playerKey, storyId, context);
                    if (evaluated == story::StoryEvaluateResult::Advanced)
                    {
                        changedStories.insert(storyId);
                        advancedStories.insert(storyId);
                        passChanged = true;
                        ++report.advanceSteps;
                        continue;
                    }
                    if (evaluated == story::StoryEvaluateResult::NoChange)
                        break;

                    failedStories.insert(storyId);
                    ++report.failures;
                    break;
                }

                if (steps == kMaxAdvanceStepsPerStory)
                {
                    failedStories.insert(storyId);
                    ++report.stepLimitHits;
                    ++report.failures;
                }
            }

            if (!passChanged)
                break;

            if (pass == kMaxGlobalPasses - 1)
            {
                ++report.stepLimitHits;
                ++report.failures;
            }
        }

        report.storiesStarted = static_cast<int>(startedStories.size());
        report.storiesAdvanced = static_cast<int>(advancedStories.size());
        const std::size_t accounted = changedStories.size() + failedStories.size();
        report.storiesUnchanged = accounted >= storyIds.size()
                                      ? 0
                                      : static_cast<int>(storyIds.size() - accounted);
        return report;
    }
};
}
}
