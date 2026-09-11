#include "compat/legacy/sin/sinLinkHeader.h"
#include "systems/gameplay/pets/PCBangPetSystem.h"
#include "core/gameplay/runtime/playmain.h"
#include "systems/rendering/effects/assa/hoAssaParticleEffect.h"
#include "systems/rendering/effects/particles/ho/HoNewParticleLinkHeader.h"
#include "systems/audio/effects/legacy/effectsnd.h"
POINT3D HoPcBangPetRenderPoint;
#include "systems/combat/damage/damage.h"
#include "systems/world/field/field.h"

struct LegacyPCBangPetPolicy
{
	using Vector3 = D3DXVECTOR3;
};

#include "gameplay/pets/PCBangPetLegacyCommon.inl"
