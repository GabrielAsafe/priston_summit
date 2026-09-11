#include "core/server/legacy_sin_runtime/sinLinkHeader.h"

struct LegacySinEffectPolicy
{
	using TLVertex = DRZD3DTLVERTEX;
	static void BeginMeshScene() { renderDevice.BeginScene(); }
	static void EndMeshScene() { renderDevice.EndScene(); }
	static void PrepareDebugText(HDC hdc)
	{
		SelectObject(hdc, sinFont);
		SetBkMode(hdc, TRANSPARENT);
		SetTextColor(hdc, RGB(255, 255, 255));
	}
};

#include "rendering/effects/SinEffectLegacyCommon.inl"
