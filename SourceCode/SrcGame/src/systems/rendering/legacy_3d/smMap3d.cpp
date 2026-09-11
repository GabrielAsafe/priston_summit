#include "systems/rendering/legacy_3d/smd3d.h"

struct LegacySmMapPolicy
{
	static DWORD Color(int r, int g, int b, int a) { return D3DCOLOR_RGBA(r, g, b, a); }
};

#include "rendering/legacy_3d/SmMap3dLegacyCommon.inl"
