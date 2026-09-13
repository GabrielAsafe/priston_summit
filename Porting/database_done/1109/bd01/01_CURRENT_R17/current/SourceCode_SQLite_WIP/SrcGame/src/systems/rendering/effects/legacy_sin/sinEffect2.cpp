#include "compat/legacy/sin/sinLinkHeader.h"

struct LegacySinEffect2Policy
{
	static void BeginEffectScene() {}
	static void EndEffectScene() {}
	static void PrepareDebugText(HDC)
	{
		SetFontTextColor32(RGB(255, 255, 255));
	}
	static void PrepareTextureRenderState()
	{
		d3ddev->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
		d3ddev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		d3ddev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		d3ddev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	}
};

#include "rendering/effects/SinEffect2LegacyCommon.inl"
