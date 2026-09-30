#include "systems/presentation/v2/legacy/LegacyAnimationAdapter.h"

#include "systems/rendering/legacy_3d/smDsx.h"
#include "systems/rendering/legacy_3d/smType.h"

class smSTAGE3D;

#include "systems/gameplay/character/core/character.h"

smCHAR *FindChrPlayer(DWORD dwObjectSerial);

namespace presentation
{
namespace legacy
{
bool LegacyAnimationAdapter::Play(gameplay::EntityId entity, const AnimationId& animation)
{
    if (!entity.Valid() || !animation.Valid())
        return false;

    unsigned long motionCode = 0ul;
    if (animation.key == "character.restart")
        motionCode = CHRMOTION_STATE_RESTART;
    else
        return false;

    smCHAR* character = FindChrPlayer(static_cast<DWORD>(entity.value));
    return character && character->SetMotionFromCode(motionCode) != 0;
}

LegacyAnimationAdapter& ClientLegacyAnimationAdapter()
{
    static LegacyAnimationAdapter adapter;
    return adapter;
}
}
}
