#include "systems/rendering/effects/particle/legacy/particle.h"
#include "systems/presentation/opening/legacy_runtime/HoLinkHeader.h"

struct LegacyHoParticlePolicy
{
	using Vector3 = D3DXVECTOR3;

	static void DisableZWrite() { d3ddev->SetRenderState(D3DRS_ZWRITEENABLE, FALSE); }
	static void EnableZWrite() { d3ddev->SetRenderState(D3DRS_ZWRITEENABLE, TRUE); }
};

#include "rendering/effects/particles/HoParticleLegacyCommon.inl"
