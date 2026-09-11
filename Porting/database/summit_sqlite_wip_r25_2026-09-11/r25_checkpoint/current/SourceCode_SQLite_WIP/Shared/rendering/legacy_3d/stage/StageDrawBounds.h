#pragma once

#include <cstdint>

namespace summit
{
namespace rendering
{
namespace legacy3d
{
namespace stage_draw_bounds
{

static const int OuterMargin = 30 * 64 * fONE;
static const int SecondaryMargin = 320 * 64 * fONE;

inline bool InsideOuterBounds(const smSTAGE3D &stage, int x, int z)
{
	const std::int64_t px = x;
	const std::int64_t pz = z;
	return pz >= static_cast<std::int64_t>(stage.StageMapRect.top) - OuterMargin &&
		pz <= static_cast<std::int64_t>(stage.StageMapRect.bottom) + OuterMargin &&
		px >= static_cast<std::int64_t>(stage.StageMapRect.left) - OuterMargin &&
		px <= static_cast<std::int64_t>(stage.StageMapRect.right) + OuterMargin;
}

inline bool InsideSecondaryBounds(const smSTAGE3D &stage, int x, int z)
{
	if(!InsideOuterBounds(stage, x, z))
		return false;
	const std::int64_t px = x;
	const std::int64_t pz = z;
	return pz >= static_cast<std::int64_t>(stage.StageMapRect.bottom) - SecondaryMargin &&
		pz <= static_cast<std::int64_t>(stage.StageMapRect.top) + SecondaryMargin &&
		px >= static_cast<std::int64_t>(stage.StageMapRect.right) - SecondaryMargin &&
		px <= static_cast<std::int64_t>(stage.StageMapRect.left) + SecondaryMargin;
}

} // namespace stage_draw_bounds
} // namespace legacy3d
} // namespace rendering
} // namespace summit
