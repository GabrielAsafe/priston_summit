#pragma once

#include <cmath>
#include <cstdint>
#include <limits>

namespace summit
{
namespace rendering
{
namespace legacy3d
{
namespace stage_point_lighting
{

inline short AddLegacyLightChannel(short current, std::int64_t delta)
{
	std::int64_t value = static_cast<std::int64_t>(current) + delta;
	if(value > 360)
		value = 360;
	if(value < static_cast<std::int64_t>(std::numeric_limits<short>::min()))
		value = std::numeric_limits<short>::min();
	return static_cast<short>(value);
}

inline bool ApplyRadialVertexLight(smSTAGE3D &stage, const POINT3D &lightPosition,
	int red, int green, int blue, int range)
{
	if(stage.nVertex < 0 || stage.VertexCapacity < stage.nVertex ||
		(stage.nVertex > 0 && !stage.Vertex))
		return false;

	const std::int64_t radiusBase =
		(static_cast<std::int64_t>(range / 256) * 72) >> FLOATNS;
	const std::int64_t radiusSquared = radiusBase * radiusBase;
	if(radiusSquared <= 0)
		return true;

	for(int vertexIndex = 0; vertexIndex < stage.nVertex; ++vertexIndex)
	{
		const smSTAGE_VERTEX &vertex = stage.Vertex[vertexIndex];
		const std::int64_t x = (static_cast<std::int64_t>(lightPosition.x) - vertex.x) >> FLOATNS;
		const std::int64_t y = (static_cast<std::int64_t>(lightPosition.y) - vertex.y) >> FLOATNS;
		const std::int64_t z = (static_cast<std::int64_t>(lightPosition.z) - vertex.z) >> FLOATNS;
		const std::int64_t distanceSquared = x * x + y * y + z * z;
		if(distanceSquared >= radiusSquared)
			continue;

		const int distance = static_cast<int>(std::sqrt(static_cast<long double>(distanceSquared)));
		const std::int64_t remaining = static_cast<std::int64_t>(range >> FLOATNS) - distance;
		const std::int64_t lightFactor = fONE - ((remaining * fONE) / radiusSquared);
		const std::int64_t redDelta = (static_cast<std::int64_t>(red) * lightFactor) >> FLOATNS;
		const std::int64_t greenDelta = (static_cast<std::int64_t>(green) * lightFactor) >> FLOATNS;
		const std::int64_t blueDelta = (static_cast<std::int64_t>(blue) * lightFactor) >> FLOATNS;

		smSTAGE_VERTEX &target = stage.Vertex[vertexIndex];
		target.sDef_Color[SMC_R] = AddLegacyLightChannel(target.sDef_Color[SMC_R], redDelta);
		target.sDef_Color[SMC_G] = AddLegacyLightChannel(target.sDef_Color[SMC_G], greenDelta);
		target.sDef_Color[SMC_B] = AddLegacyLightChannel(target.sDef_Color[SMC_B], blueDelta);
	}
	return true;
}

} // namespace stage_point_lighting
} // namespace legacy3d
} // namespace rendering
} // namespace summit
