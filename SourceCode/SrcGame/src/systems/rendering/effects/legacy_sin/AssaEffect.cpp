#include "compat/legacy/sin/sinLinkHeader.h"
#include "systems/world/field/field.h"

struct LegacyAssaEffectPolicy
{
    static void InitializeEffects()
    {
        // Opening initializes the effect catalog before character selection.
        // The old path recursively closed/rescanned both effect trees again on
        // every InitAssaEffect() call, causing a large synchronous Wine/FS stall.
        // Keep the catalog process-lifetime resident and only reset active slots.
        static bool catalogReady = false;
        if (!catalogReady)
        {
            InitAssaRes();
            LoadAssaRes();
            catalogReady = true;
        }
        memset(cAssaEffect, 0, sizeof(cAssaEffect));
    }
    static void BeginEffectScene() {}
    static void EndEffectScene() {}
};

#include "rendering/effects/AssaEffectLegacyCommon.inl"
