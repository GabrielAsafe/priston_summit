#include "core/server/legacy_sin_runtime/sinLinkHeader.h"
#include "systems/gameplay/pets/PCBangPetSystem.h"
#include "core/server/gameplay_runtime/playmain.h"
#include "systems/rendering/effects/assa/hoAssaParticleEffect.h"
#include "systems/rendering/effects/particles/ho/HoNewParticleLinkHeader.h"
#include "systems/audio/effects/legacy/effectsnd.h"
POINT3D HoPcBangPetRenderPoint;
#include "systems/combat/damage/damage.h"

struct LegacyPCBangPetPolicy
{
	using Vector3 = D3DVECTOR;
};

#include "gameplay/pets/PCBangPetLegacyCommon.inl"
