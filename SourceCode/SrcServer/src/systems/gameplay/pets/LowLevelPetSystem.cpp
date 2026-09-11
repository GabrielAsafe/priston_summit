#include "core/server/legacy_sin_runtime/sinLinkHeader.h"
#include "systems/gameplay/pets/LowLevelPetSystem.h"
#include "core/server/gameplay_runtime/playmain.h"
#include "systems/rendering/effects/assa/hoAssaParticleEffect.h"
#include "systems/rendering/effects/particles/ho/HoNewParticleLinkHeader.h"
#include "systems/audio/effects/legacy/effectsnd.h"
POINT3D HoPetRenderPoint;
#include "systems/combat/damage/damage.h"

struct LegacyLowLevelPetPolicy
{
	using Vector3 = D3DVECTOR;
};

#include "gameplay/pets/LowLevelPetLegacyCommon.inl"
