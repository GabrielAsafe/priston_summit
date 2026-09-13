#include "systems/rendering/effects/legacy_new/core/HoEffectManager.h"
#include "systems/rendering/effects/legacy_new/core/HoEffectResMgr.h"
#include "systems/rendering/effects/legacy_new/core/HoEffectView.h"
#include "systems/rendering/effects/legacy_new/core/HoNewEffectFunction.h"
#include "systems/network/server_gameplay/netplay.h"
#include "systems/rendering/effects/legacy_ho/HoMinMax.h"
#include "systems/rendering/effects/particles/ho/HoNewParticle.h"
#include "systems/rendering/effects/particles/ho/HoNewParticleMgr.h"
#include "systems/rendering/effects/assa/hoAssaParticleEffect.h"
#include "systems/world/field/field.h"
#include "core/server/legacy_sin_runtime/sinLinkHeader.h"

struct LegacyHoEffectManagerPolicy
{
	using Vector3 = D3DVECTOR;

	static void DisableZWrite() { renderDevice.SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE); }
	static void EnableZWrite() { renderDevice.SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, TRUE); }
};

#include "rendering/effects/legacy_new/HoEffectManagerLegacyCommon.inl"
