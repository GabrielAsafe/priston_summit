#include "compat/legacy/sin/sinLinkHeader.h"

struct LegacySinEffectPolicy
{
	using TLVertex = D3DTLVERTEX;
	static void BeginMeshScene() {}
	static void EndMeshScene() {}
	static void PrepareDebugText(HDC)
	{
		SetFontTextColor(RGB(255, 255, 255));
	}
};

#include "rendering/effects/SinEffectLegacyCommon.inl"
