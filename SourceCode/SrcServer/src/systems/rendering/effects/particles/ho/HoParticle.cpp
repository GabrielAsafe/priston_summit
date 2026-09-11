#include "systems/rendering/effects/particle_legacy/particle.h"
#include "systems/presentation/opening/legacy_runtime/HoLinkHeader.h"

struct LegacyHoParticlePolicy
{
	using Vector3 = D3DVECTOR;

	static void DisableZWrite() { renderDevice.SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE); }
	static void EnableZWrite() { renderDevice.SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, TRUE); }
};

#include "rendering/effects/particles/HoParticleLegacyCommon.inl"
