#include "systems/presentation/v2/legacy/LegacyLightingAdapter.h"

extern int SetDynLight(int x, int y, int z,
                       int r, int g, int b, int a,
                       int power, int decayPower);

namespace presentation
{
namespace legacy
{
bool LegacyLightingAdapter::PulseAt(const LightId& light, const PresentationTarget& target)
{
    if (!light.Valid())
        return false;

    if (light.key != "light.character.resurrection_restart")
        return false;

    return SetDynLight(target.x, target.y, target.z,
                       100, 100, 100, 0, 200, 10) != 0;
}

LegacyLightingAdapter& ClientLegacyLightingAdapter()
{
    static LegacyLightingAdapter adapter;
    return adapter;
}
}
}
