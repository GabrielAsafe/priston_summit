#include "compat/legacy/sin/sinLinkHeader.h"
#include "systems/gameplay/pets/LowLevelPetSystem.h"
#include "core/gameplay/runtime/playmain.h"
#include "systems/rendering/effects/assa/hoAssaParticleEffect.h"
#include "systems/rendering/effects/particles/ho/HoNewParticleLinkHeader.h"
#include "systems/audio/effects/legacy/effectsnd.h"
POINT3D HoPetRenderPoint;
#include "systems/combat/damage/damage.h"
#include "systems/world/field/field.h"

struct LegacyLowLevelPetPolicy
{
	using Vector3 = D3DXVECTOR3;
};

#include "gameplay/pets/LowLevelPetLegacyCommon.inl"
