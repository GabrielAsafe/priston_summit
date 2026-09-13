#pragma once

#include <cstdint>
#include <limits>

namespace summit
{
namespace rendering
{
namespace legacy3d
{
namespace object_visibility
{

struct Viewport
{
	int midpointX;
	int midpointY;
	int projectionDistance;
	int viewLength;
	int screenWidth;
	int screenHeight;
};

inline bool ToInt(std::int64_t value, int &result)
{
	if(value < std::numeric_limits<int>::min() || value > std::numeric_limits<int>::max())
		return false;
	result = static_cast<int>(value);
	return true;
}

inline bool ProjectSphere(int x, int y, int z, int bound,
	int angleX, int angleY, int angleZ,
	const int *sinTable, const int *cosTable, int angleMask, int trigShift,
	int fixedShift, const Viewport &viewport)
{
	if(!sinTable || !cosTable || angleMask < 0 || trigShift < 0 || fixedShift < 0 ||
		fixedShift >= 31 || bound < 0 || viewport.projectionDistance <= 0 ||
		viewport.viewLength <= 0 || viewport.screenWidth <= 0 || viewport.screenHeight <= 0)
		return false;

	const int gx = (-angleX) & angleMask;
	const int gy = (-angleY) & angleMask;
	const int gz = (-angleZ) & angleMask;
	const int trig[6] = {
		sinTable[gx] >> trigShift,
		cosTable[gx] >> trigShift,
		sinTable[gy] >> trigShift,
		cosTable[gy] >> trigShift,
		sinTable[gz] >> trigShift,
		cosTable[gz] >> trigShift
	};

	const std::int64_t scale = static_cast<std::int64_t>(1) << fixedShift;
	const std::int64_t wx = static_cast<std::int64_t>(x) * scale;
	const std::int64_t wy = static_cast<std::int64_t>(y) * scale;
	const std::int64_t wz = static_cast<std::int64_t>(z) * scale;

	const std::int64_t i = (wz * trig[2] + wx * trig[3]) >> fixedShift;
	const std::int64_t j0 = wy;
	const std::int64_t k = (wz * trig[3] - wx * trig[2]) >> fixedShift;
	const std::int64_t rotatedY = (j0 * trig[1] - k * trig[0]) >> fixedShift;
	const std::int64_t rotatedZ = (j0 * trig[0] + k * trig[1]) >> fixedShift;
	const std::int64_t rotatedX = (i * trig[5] - rotatedY * trig[4]) >> fixedShift;
	const std::int64_t finalY = (i * trig[4] + rotatedY * trig[5]) >> fixedShift;

	const std::int64_t pz = rotatedZ >> fixedShift;
	if((pz + bound) <= 0 || pz >= viewport.viewLength)
		return false;

	std::int64_t screenX64 = -32767;
	std::int64_t screenY64 = -32767;
	if(rotatedZ > 0)
	{
		screenX64 = static_cast<std::int64_t>(viewport.midpointX) +
			(rotatedX * viewport.projectionDistance) / rotatedZ;
		screenY64 = static_cast<std::int64_t>(viewport.midpointY) -
			(finalY * viewport.projectionDistance) / rotatedZ;
	}
	const std::int64_t absPz = pz < 0 ? -pz : pz;
	const std::int64_t safePz = absPz == 0 ? 1 : absPz;
	const std::int64_t projectedBound = (static_cast<std::int64_t>(bound) * viewport.projectionDistance) / safePz;

	int screenX = 0;
	int screenY = 0;
	int screenBound = 0;
	if(!ToInt(screenX64, screenX) || !ToInt(screenY64, screenY) || !ToInt(projectedBound, screenBound))
		return false;

	return static_cast<std::int64_t>(screenX) + screenBound > 0 &&
		static_cast<std::int64_t>(screenX) - screenBound < viewport.screenWidth &&
		static_cast<std::int64_t>(screenY) + screenBound > 0 &&
		static_cast<std::int64_t>(screenY) - screenBound < viewport.screenHeight;
}

} // namespace object_visibility
} // namespace legacy3d
} // namespace rendering
} // namespace summit
