#include "core/server/legacy_sin_runtime/sinLinkHeader.h"

struct LegacySinAssaParticlePolicy
{
	using Vector3 = D3DVECTOR;
	static int GreenLightRange() { return 7000; }
};

#include "rendering/effects/SinAssaParticleLegacyCommon.inl"
