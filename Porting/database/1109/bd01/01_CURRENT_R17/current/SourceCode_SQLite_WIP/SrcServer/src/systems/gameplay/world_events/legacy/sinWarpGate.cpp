#include "core/server/legacy_sin_runtime/sinLinkHeader.h"
#include "systems/world/field/field.h"
#include "core/server/legacy_runtime/onserver.h"

struct LegacySinWarpGatePolicy
{
	static const char* WarpMapPath() { return "Image//SinImage//Help//WG_map.tga"; }
	static const char* IconPath() { return "Image//SinImage//Help//WG_icon.tga"; }
	static const char* SelectPath() { return "Image//SinImage//Help//WG_slt.tga"; }
	static DWORD HelpColor() { return RGBA_MAKE(0, 15, 128, 125); }
};

#include "gameplay/world_events/SinWarpGateLegacyCommon.inl"
