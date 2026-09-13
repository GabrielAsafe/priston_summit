#include "systems/rendering/legacy_3d/smd3d.h"
#include "systems/rendering/background/legacy/pbackground.h"
#include "systems/world/field/field.h"

struct LegacyBackgroundPolicy
{
	static const char *Asset(const char *clientPath, const char *) { return clientPath; }
	static void InitializeColorZClip() { smInitColorZClip(21, 17, 14); }
};

#include "rendering/background/LegacyBackgroundCommon.inl"
