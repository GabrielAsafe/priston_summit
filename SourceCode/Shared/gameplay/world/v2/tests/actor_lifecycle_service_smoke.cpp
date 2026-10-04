#include <cassert>

#include "gameplay/world/v2/ActorLifecycleService.h"

int main()
{
    using gameplay::world::ActorLifecycleService;

    const gameplay::world::ActorAnimationAdvance active =
        ActorLifecycleService::AdvanceOneShot(100, 160, 20);
    assert(active.nextFrame == 120);
    assert(!active.finished);

    const gameplay::world::ActorAnimationAdvance boundary =
        ActorLifecycleService::AdvanceOneShot(140, 160, 20);
    assert(boundary.nextFrame == 160);
    assert(!boundary.finished);

    const gameplay::world::ActorAnimationAdvance expired =
        ActorLifecycleService::AdvanceOneShot(160, 160, 20);
    assert(expired.nextFrame == 180);
    assert(expired.finished);

    const gameplay::world::ActorAnimationCompletion notComplete =
        ActorLifecycleService::ResolveAnimationCompletion(160, 80, 160, false, false);
    assert(!notComplete.completed);
    assert(notComplete.nextFrame == 160);
    assert(notComplete.action == gameplay::world::ActorAnimationCompletionAction::Continue);

    const gameplay::world::ActorAnimationCompletion dead =
        ActorLifecycleService::ResolveAnimationCompletion(180, 80, 160, true, false);
    assert(dead.completed);
    assert(dead.nextFrame == 160);
    assert(dead.action == gameplay::world::ActorAnimationCompletionAction::HoldFinalFrame);

    const gameplay::world::ActorAnimationCompletion repeat =
        ActorLifecycleService::ResolveAnimationCompletion(180, 80, 160, false, true);
    assert(repeat.completed);
    assert(repeat.nextFrame == 100);
    assert(repeat.action == gameplay::world::ActorAnimationCompletionAction::RepeatFromStart);

    const gameplay::world::ActorAnimationCompletion stand =
        ActorLifecycleService::ResolveAnimationCompletion(180, 80, 160, false, false);
    assert(stand.completed);
    assert(stand.nextFrame == 180);
    assert(stand.action == gameplay::world::ActorAnimationCompletionAction::ReturnToStand);
    return 0;
}
