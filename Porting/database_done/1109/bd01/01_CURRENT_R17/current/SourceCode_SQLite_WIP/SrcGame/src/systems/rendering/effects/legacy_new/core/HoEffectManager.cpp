#include "systems/rendering/effects/legacy_new/core/HoEffectManager.h"
#include "systems/rendering/effects/legacy_new/core/HoEffectResMgr.h"
#include "systems/rendering/effects/legacy_new/core/HoEffectView.h"
#include "systems/rendering/effects/legacy_new/core/HoNewEffectFunction.h"
#include "systems/network/client/netplay.h"
#include "systems/rendering/effects/legacy_ho/core/HoMinMax.h"
#include "systems/rendering/effects/particles/ho/HoNewParticle.h"
#include "systems/rendering/effects/particles/ho/HoNewParticleMgr.h"
#include "systems/rendering/effects/assa/hoAssaParticleEffect.h"
#include "systems/world/field/field.h"
#include "compat/legacy/sin/sinLinkHeader.h"

struct LegacyHoEffectManagerPolicy
{
	using Vector3 = D3DXVECTOR3;

	static void DisableZWrite() { d3ddev->SetRenderState(D3DRS_ZWRITEENABLE, FALSE); }
	static void EnableZWrite() { d3ddev->SetRenderState(D3DRS_ZWRITEENABLE, TRUE); }
};

#include "rendering/effects/legacy_new/HoEffectManagerLegacyCommon.inl"
