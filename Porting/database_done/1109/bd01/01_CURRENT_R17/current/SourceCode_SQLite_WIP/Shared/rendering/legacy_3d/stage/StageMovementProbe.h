#pragma once

#include <cstddef>

namespace summit
{
namespace rendering
{
namespace legacy3d
{
namespace stage_movement_probe
{

static const int ProbeLineCount = 4;

template <typename Resolver>
inline bool BuildMovementProbe(const POINT3D &position, const POINT3D &angle,
	int distance, int objectWidth, int objectHeight,
	smLINE3D *lines, std::size_t lineCapacity, POINT3D &endpoint,
	Resolver resolveOffset)
{
	if(!lines || lineCapacity < static_cast<std::size_t>(ProbeLineCount))
		return false;

	const int fixedOne = (1 << FLOATNS);
	const int extendedDistance = distance + fixedOne * 12;
	const int lowerHeight = fixedOne * 12;
	const int upperHeight = objectHeight - (objectHeight >> 2);
	const int halfWidth = objectWidth >> 2;

	auto pointFrom = [&](int localX, int localY, int localZ) {
		const POINT3D offset = resolveOffset(localX, localY, localZ, angle);
		POINT3D point;
		point.x = position.x + offset.x;
		point.y = position.y + offset.y;
		point.z = position.z + offset.z;
		return point;
	};

	endpoint = pointFrom(0, 0, distance);

	lines[0].sp = pointFrom(0, lowerHeight, 0);
	lines[0].ep = pointFrom(0, lowerHeight, extendedDistance);
	lines[1].sp = pointFrom(0, upperHeight, 0);
	lines[1].ep = pointFrom(0, upperHeight, extendedDistance);
	lines[2].sp = pointFrom(-halfWidth, lowerHeight, extendedDistance);
	lines[2].ep = pointFrom(halfWidth, lowerHeight, extendedDistance);
	lines[3].sp = pointFrom(-halfWidth, upperHeight, extendedDistance);
	lines[3].ep = pointFrom(halfWidth, upperHeight, extendedDistance);
	return true;
}

} // namespace stage_movement_probe
} // namespace legacy3d
} // namespace rendering
} // namespace summit
