#include "compat/legacy/sin/sinLinkHeader.h"
#include "systems/world/field/field.h"
#include "compat/client_server/server_legacy/onserver.h"

struct LegacySinWarpGatePolicy
{
	static const char* WarpMapPath() { return "image//Sinimage//help//WG_map.tga"; }
	static const char* IconPath() { return "image//Sinimage//help//WG_icon.tga"; }
	static const char* SelectPath() { return "image//Sinimage//help//WG_slt.tga"; }
	static DWORD HelpColor() { return D3DCOLOR_RGBA(0, 15, 128, 125); }
};

#include "gameplay/world_events/SinWarpGateLegacyCommon.inl"
