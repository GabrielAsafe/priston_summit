#pragma once

#include <cstdint>
#include <limits>

namespace summit
{
namespace rendering
{
namespace legacy3d
{
namespace object_view
{

inline bool ToInt(std::int64_t value, int &result)
{
	if(value < std::numeric_limits<int>::min() || value > std::numeric_limits<int>::max())
		return false;
	result = static_cast<int>(value);
	return true;
}

inline bool RelativeViewPosition(const POINT3D &objectPosition, const POINT3D &cameraPosition,
	int fixedShift, int &x, int &y, int &z)
{
	if(fixedShift < 0 || fixedShift >= 31)
		return false;
	return ToInt((static_cast<std::int64_t>(objectPosition.x) - cameraPosition.x) >> fixedShift, x) &&
		ToInt((static_cast<std::int64_t>(objectPosition.y) - cameraPosition.y) >> fixedShift, y) &&
		ToInt((static_cast<std::int64_t>(objectPosition.z) - cameraPosition.z) >> fixedShift, z);
}

inline bool ResolveCameraPosition(const POINT3D &objectPosition, const POINT3D &cameraPosition,
	bool fixedObjectPosition, POINT3D &result)
{
	if(fixedObjectPosition)
	{
		result = cameraPosition;
		return true;
	}
	return ToInt(static_cast<std::int64_t>(cameraPosition.x) - objectPosition.x, result.x) &&
		ToInt(static_cast<std::int64_t>(cameraPosition.y) - objectPosition.y, result.y) &&
		ToInt(static_cast<std::int64_t>(cameraPosition.z) - objectPosition.z, result.z);
}

} // namespace object_view
} // namespace legacy3d
} // namespace rendering
} // namespace summit
