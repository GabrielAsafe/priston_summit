
#include "compat/legacy/sin/sinLinkHeader.h"

struct LegacySinAssaParticlePolicy
{
	using Vector3 = D3DXVECTOR3;
	static int GreenLightRange() { return 9000; }
};

#include "rendering/effects/SinAssaParticleLegacyCommon.inl"
