#include "core/server/legacy_sin_runtime/sinLinkHeader.h"

struct LegacySinEffect2Policy
{
	static void BeginEffectScene() { renderDevice.BeginScene(); }
	static void EndEffectScene() { renderDevice.EndScene(); }
	static void PrepareDebugText(HDC hdc)
	{
		SelectObject(hdc, sinFont);
		SetBkMode(hdc, TRANSPARENT);
		SetTextColor(hdc, RGB(255, 255, 255));
	}
	static void PrepareTextureRenderState()
	{
		renderDevice.SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, TRUE);
		renderDevice.SetRenderState(D3DRENDERSTATE_ZFUNC, D3DCMP_LESSEQUAL);
		renderDevice.SetRenderState(D3DRENDERSTATE_TEXTUREHANDLE, 0);
		renderDevice.SetRenderState(D3DRENDERSTATE_DITHERENABLE, TRUE);
		renderDevice.SetRenderState(D3DRENDERSTATE_TEXTUREADDRESS, D3DTADDRESS_WRAP);
		renderDevice.SetRenderState(D3DRENDERSTATE_TEXTUREPERSPECTIVE, TRUE);
		renderDevice.SetRenderState(D3DRENDERSTATE_TEXTUREMAG, D3DFILTER_LINEAR);
		renderDevice.SetRenderState(D3DRENDERSTATE_TEXTUREMIN, D3DFILTER_LINEAR);
		renderDevice.SetRenderState(D3DRENDERSTATE_SPECULARENABLE, TRUE);
		renderDevice.SetRenderState(D3DRENDERSTATE_COLORKEYENABLE, FALSE);
		renderDevice.SetRenderState(D3DRENDERSTATE_TEXTUREMAPBLEND, D3DTBLEND_MODULATE);
		renderDevice.SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);
		renderDevice.SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA);
		renderDevice.SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA);
		renderDevice.SetTextureStageState(0, D3DTSS_MINFILTER, D3DTFN_LINEAR);
		renderDevice.SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTFN_LINEAR);
	}
};

#include "rendering/effects/SinEffect2LegacyCommon.inl"
