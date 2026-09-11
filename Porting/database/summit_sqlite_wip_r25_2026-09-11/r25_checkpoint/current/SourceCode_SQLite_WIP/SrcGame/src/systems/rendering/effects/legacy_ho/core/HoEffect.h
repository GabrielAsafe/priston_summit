#ifndef _HO_EFFECT_
#define _HO_EFFECT_

#include "systems/rendering/legacy_3d/smType.h"

struct LegacyHoEffectPolicy
{
	using TLVertex = D3DTLVERTEX;
	using Vector3 = D3DXVECTOR3;
};

#include "rendering/effects/legacy_ho/HoEffectLegacyBody.h"
#endif
