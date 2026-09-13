#include <cstring>
#include "systems/presentation/opening/legacy_runtime/HoLinkHeader.h"
#include "systems/rendering/effects/particles/ho/HoNewParticleLinkHeader.h"

struct LegacyHoNewParticleMgrPolicy
{
	using Vector3 = D3DVECTOR;
};

#include "rendering/effects/particles/HoNewParticleMgrLegacyCommon.inl"
