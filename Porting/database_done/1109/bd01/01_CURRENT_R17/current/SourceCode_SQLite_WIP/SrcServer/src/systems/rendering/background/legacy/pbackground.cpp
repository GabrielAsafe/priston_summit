#include "systems/rendering/legacy_3d/smd3d.h"
#include "systems/rendering/background/legacy/pbackground.h"
#include "systems/world/field/field.h"

struct LegacyBackgroundPolicy
{
	static const char *Asset(const char *, const char *serverPath) { return serverPath; }
	static void InitializeColorZClip() { smInitColorZClip(0, 0, 0); }
};

#include "rendering/background/LegacyBackgroundCommon.inl"
