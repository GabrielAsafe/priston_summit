#include "systems/presentation/v2/legacy/LegacyVisualEffectAdapter.h"

// Narrow ABI declaration: importing HoEffect.h would expose renderer/character
// types to the presentation adapter just to call this stable entrypoint.
extern int StartEffect(int x, int y, int z, int effectType, int level);

namespace presentation
{
namespace legacy
{
namespace
{
const int kAgingEffectIndex = 5001; // EFFECT_AGING
const int kGameStartEffectIndex = 500; // EFFECT_GAME_START1
}

bool LegacyVisualEffectAdapter::PlayAt(const EffectId& effect, const PresentationTarget& target)
{
    if (!effect.Valid())
        return false;

    int effectIndex = -1;
    if (effect.key == "effect.ui.stat_reset")
        effectIndex = kAgingEffectIndex;
    else if (effect.key == "effect.character.resurrection_restart")
        effectIndex = kGameStartEffectIndex;
    else
        return false;

    return StartEffect(target.x, target.y, target.z, effectIndex, 1) != 0;
}

LegacyVisualEffectAdapter& ClientLegacyVisualEffectAdapter()
{
    static LegacyVisualEffectAdapter adapter;
    return adapter;
}
}
}
