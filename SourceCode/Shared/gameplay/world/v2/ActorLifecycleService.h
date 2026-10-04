#pragma once

namespace gameplay
{
namespace world
{
struct ActorAnimationAdvance
{
    int nextFrame;
    bool finished;

    ActorAnimationAdvance(int nextFrameValue = 0, bool finishedValue = false)
        : nextFrame(nextFrameValue), finished(finishedValue) {}
};


enum class ActorAnimationCompletionAction
{
    Continue,
    HoldFinalFrame,
    RepeatFromStart,
    ReturnToStand
};

struct ActorAnimationCompletion
{
    int nextFrame;
    bool completed;
    ActorAnimationCompletionAction action;

    ActorAnimationCompletion(int nextFrameValue = 0,
                             bool completedValue = false,
                             ActorAnimationCompletionAction actionValue = ActorAnimationCompletionAction::Continue)
        : nextFrame(nextFrameValue), completed(completedValue), action(actionValue) {}
};

// Pure lifecycle policy for one-shot actor motions. The caller owns the
// concrete legacy actor and performs Close()/buffer publication; GREEN only
// decides frame progression and when the motion has expired.
class ActorLifecycleService
{
public:
    static ActorAnimationAdvance AdvanceOneShot(int currentFrame, int endFrame, int frameStep)
    {
        const int nextFrame = currentFrame + frameStep;
        return ActorAnimationAdvance(nextFrame, nextFrame > endFrame);
    }

    // Resolves the shared legacy completion rule used by server-controlled actor
    // motions. Callers still own concrete state changes and side effects.
    static ActorAnimationCompletion ResolveAnimationCompletion(int currentFrame,
                                                                int startFrame,
                                                                int endFrame,
                                                                bool dead,
                                                                bool repeat)
    {
        if(currentFrame <= endFrame)
            return ActorAnimationCompletion(currentFrame, false, ActorAnimationCompletionAction::Continue);
        if(dead)
            return ActorAnimationCompletion(endFrame, true, ActorAnimationCompletionAction::HoldFinalFrame);
        if(repeat)
            return ActorAnimationCompletion(startFrame + (currentFrame - endFrame),
                                            true, ActorAnimationCompletionAction::RepeatFromStart);
        return ActorAnimationCompletion(currentFrame, true, ActorAnimationCompletionAction::ReturnToStand);
    }
};
}
}
