#pragma once

#include <cstdint>
#include <limits>

#include "rendering/legacy_3d/stage/StageAreaQuery.h"

namespace summit
{
namespace rendering
{
namespace legacy3d
{
namespace stage_height
{

struct EdgeSample
{
	int x;
	int y;
};

struct OrderedTriangle
{
	const smSTAGE_VERTEX *top;
	const smSTAGE_VERTEX *middle;
	const smSTAGE_VERTEX *bottom;
};

inline bool FitsInt(std::int64_t value)
{
	return value >= static_cast<std::int64_t>(std::numeric_limits<int>::min()) &&
		value <= static_cast<std::int64_t>(std::numeric_limits<int>::max());
}

inline bool OrderTriangleByZ(const smSTAGE_VERTEX &first, const smSTAGE_VERTEX &second,
	const smSTAGE_VERTEX &third, OrderedTriangle &ordered)
{
	const smSTAGE_VERTEX *points[3] = { &first, &second, &third };
	ordered.top = points[0];
	ordered.bottom = points[1];
	ordered.middle = points[2];

	for(int index = 0; index < 3; ++index)
	{
		if(points[index]->z < ordered.top->z)
			ordered.top = points[index];
	}
	if(ordered.top == ordered.bottom)
		ordered.bottom = ordered.middle;

	for(int index = 0; index < 3; ++index)
	{
		if(points[index]->z > ordered.bottom->z && ordered.top != points[index])
			ordered.bottom = points[index];
	}
	for(int index = 0; index < 3; ++index)
	{
		if(ordered.top != points[index] && ordered.bottom != points[index])
		{
			ordered.middle = points[index];
			break;
		}
	}
	return ordered.top && ordered.middle && ordered.bottom;
}

inline bool FixedSlope(int delta, int distance, std::int64_t &slope)
{
	if(distance == 0)
	{
		slope = 0;
		return true;
	}
	const std::int64_t numerator = static_cast<std::int64_t>(delta) << FLOATNS;
	slope = numerator / distance;
	return true;
}

inline bool SampleCoordinate(int origin, std::int64_t slope, int distance, int &value)
{
	const std::int64_t sample =
		((slope * static_cast<std::int64_t>(distance)) >> FLOATNS) + origin;
	if(!FitsInt(sample))
		return false;
	value = static_cast<int>(sample);
	return true;
}

inline bool SampleEdgeAtZ(const smSTAGE_VERTEX &start, const smSTAGE_VERTEX &end,
	int z, EdgeSample &sample)
{
	const int dz = end.z - start.z;
	std::int64_t xSlope = 0;
	std::int64_t ySlope = 0;
	if(!FixedSlope(end.x - start.x, dz, xSlope) ||
		!FixedSlope(end.y - start.y, dz, ySlope))
		return false;
	const int offset = z - start.z;
	return SampleCoordinate(start.x, xSlope, offset, sample.x) &&
		SampleCoordinate(start.y, ySlope, offset, sample.y);
}

inline int TriangleHeightAtXZ(const smSTAGE_VERTEX &first, const smSTAGE_VERTEX &second,
	const smSTAGE_VERTEX &third, int x, int z)
{
	OrderedTriangle triangle;
	if(!OrderTriangleByZ(first, second, third, triangle))
		return CLIP_OUT;
	if(z < triangle.top->z || z > triangle.bottom->z)
		return CLIP_OUT;

	EdgeSample longEdge;
	EdgeSample shortEdge;
	if(!SampleEdgeAtZ(*triangle.top, *triangle.bottom, z, longEdge))
		return CLIP_OUT;
	if(z < triangle.middle->z)
	{
		if(!SampleEdgeAtZ(*triangle.top, *triangle.middle, z, shortEdge))
			return CLIP_OUT;
	}
	else
	{
		if(!SampleEdgeAtZ(*triangle.middle, *triangle.bottom, z, shortEdge))
			return CLIP_OUT;
	}

	if(longEdge.x > shortEdge.x)
	{
		const EdgeSample temporary = longEdge;
		longEdge = shortEdge;
		shortEdge = temporary;
	}
	if(x < longEdge.x || x > shortEdge.x)
		return CLIP_OUT;

	const int dx = shortEdge.x - longEdge.x;
	if(dx == 0)
		return longEdge.y;

	std::int64_t ySlope = 0;
	if(!FixedSlope(shortEdge.y - longEdge.y, dx, ySlope))
		return CLIP_OUT;
	int height = 0;
	return SampleCoordinate(longEdge.y, ySlope, x - longEdge.x, height)
		? height : CLIP_OUT;
}

inline int FaceHeightAtXZ(const smSTAGE3D &stage, const smSTAGE_FACE &face, int x, int z)
{
	const smSTAGE_VERTEX *vertices[3] = { 0, 0, 0 };
	if(!summit::rendering::legacy3d::stage_geometry::FaceVertices(stage, face, vertices))
		return CLIP_OUT;
	return TriangleHeightAtXZ(*vertices[0], *vertices[1], *vertices[2], x, z);
}

template <typename Predicate>
inline int HighestHeightInCell(smSTAGE3D &stage, int areaX, int areaZ, int x, int z,
	Predicate includeFace, int floorHeight)
{
	const int token = summit::rendering::legacy3d::stage_query::BeginFaceQuery(stage);
	int height = floorHeight;
	summit::rendering::legacy3d::stage_area_query::VisitUniqueFacesInCell(
		stage, areaX, areaZ, token, [&](smSTAGE_FACE &face) {
			if(!includeFace(face))
				return true;
			const int candidate = FaceHeightAtXZ(stage, face, x, z);
			if(candidate != CLIP_OUT && candidate > height)
				height = candidate;
			return true;
		});
	return height;
}

} // namespace stage_height
} // namespace legacy3d
} // namespace rendering
} // namespace summit
