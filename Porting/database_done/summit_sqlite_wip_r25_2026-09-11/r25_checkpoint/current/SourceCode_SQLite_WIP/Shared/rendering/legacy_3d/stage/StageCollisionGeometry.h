#pragma once

#include <cstdint>
#include <limits>

#include "rendering/legacy_3d/stage/StageGeometryAccess.h"

namespace summit
{
namespace rendering
{
namespace legacy3d
{
namespace stage_collision
{

inline bool FitsInt(std::int64_t value)
{
	return value >= static_cast<std::int64_t>(std::numeric_limits<int>::min()) &&
		value <= static_cast<std::int64_t>(std::numeric_limits<int>::max());
}

inline std::int64_t EdgePlaneProduct(const POINT3D &p1, const POINT3D &p2,
	const POINT3D &p3, const POINT3D &point)
{
	const std::int64_t ux = (static_cast<std::int64_t>(p2.x) - p1.x) >> 6;
	const std::int64_t uy = (static_cast<std::int64_t>(p2.y) - p1.y) >> 6;
	const std::int64_t uz = (static_cast<std::int64_t>(p2.z) - p1.z) >> 6;
	const std::int64_t vx = (static_cast<std::int64_t>(p3.x) - p1.x) >> 6;
	const std::int64_t vy = (static_cast<std::int64_t>(p3.y) - p1.y) >> 6;
	const std::int64_t vz = (static_cast<std::int64_t>(p3.z) - p1.z) >> 6;
	const std::int64_t nx = (uy * vz - uz * vy) >> 2;
	const std::int64_t ny = (uz * vx - ux * vz) >> 2;
	const std::int64_t nz = (ux * vy - uy * vx) >> 2;
	return nx * ((static_cast<std::int64_t>(point.x) - p1.x) >> 6) +
		ny * ((static_cast<std::int64_t>(point.y) - p1.y) >> 6) +
		nz * ((static_cast<std::int64_t>(point.z) - p1.z) >> 6);
}

inline std::int64_t FacePlaneProduct(const smSTAGE_FACE &face,
	const smSTAGE_VERTEX &origin, const POINT3D &point)
{
	return static_cast<std::int64_t>(face.VectNormal[0] >> 4) *
		((static_cast<std::int64_t>(point.x) - origin.x) >> 4) +
		static_cast<std::int64_t>(face.VectNormal[1] >> 4) *
		((static_cast<std::int64_t>(point.y) - origin.y) >> 4) +
		static_cast<std::int64_t>(face.VectNormal[2] >> 4) *
		((static_cast<std::int64_t>(point.z) - origin.z) >> 4);
}

inline bool OutsideVerticalRange(const POINT3D &start, const POINT3D &end,
	const POINT3D (&triangle)[3])
{
	const bool startBelow = start.y < triangle[0].y && start.y < triangle[1].y && start.y < triangle[2].y;
	const bool startAbove = start.y > triangle[0].y && start.y > triangle[1].y && start.y > triangle[2].y;
	const bool endBelow = end.y < triangle[0].y && end.y < triangle[1].y && end.y < triangle[2].y;
	const bool endAbove = end.y > triangle[0].y && end.y > triangle[1].y && end.y > triangle[2].y;
	return (startBelow || startAbove) && (endBelow || endAbove);
}

inline bool BuildImpactVector(const POINT3D &start, const POINT3D &end,
	bool reverse, POINT3D &vector)
{
	const std::int64_t direction = reverse ? -1 : 1;
	const std::int64_t x = direction * (static_cast<std::int64_t>(end.x) - start.x) << 8;
	const std::int64_t y = direction * (static_cast<std::int64_t>(end.y) - start.y) << 8;
	const std::int64_t z = direction * (static_cast<std::int64_t>(end.z) - start.z) << 8;
	if(!FitsInt(x) || !FitsInt(y) || !FitsInt(z))
		return false;
	vector.x = static_cast<int>(x);
	vector.y = static_cast<int>(y);
	vector.z = static_cast<int>(z);
	return true;
}

inline bool AddVector(const POINT3D &point, const POINT3D &vector, POINT3D &result)
{
	const std::int64_t x = static_cast<std::int64_t>(point.x) + vector.x;
	const std::int64_t y = static_cast<std::int64_t>(point.y) + vector.y;
	const std::int64_t z = static_cast<std::int64_t>(point.z) + vector.z;
	if(!FitsInt(x) || !FitsInt(y) || !FitsInt(z))
		return false;
	result.x = static_cast<int>(x);
	result.y = static_cast<int>(y);
	result.z = static_cast<int>(z);
	return true;
}

inline bool LineIntersectsFace(const smSTAGE3D &stage, const smSTAGE_FACE &face,
	const smLINE3D &line)
{
	const smSTAGE_VERTEX *vertices[3] = { 0, 0, 0 };
	if(!summit::rendering::legacy3d::stage_geometry::FaceVertices(stage, face, vertices))
		return false;
	POINT3D triangle[3];
	for(int corner = 0; corner < 3; ++corner)
	{
		triangle[corner].x = vertices[corner]->x;
		triangle[corner].y = vertices[corner]->y;
		triangle[corner].z = vertices[corner]->z;
	}
	if(OutsideVerticalRange(line.sp, line.ep, triangle))
		return false;

	const std::int64_t startSide = FacePlaneProduct(face, *vertices[0], line.sp);
	const std::int64_t endSide = FacePlaneProduct(face, *vertices[0], line.ep);
	if((startSide <= 0 && endSide <= 0) || (startSide > 0 && endSide > 0))
		return false;

	POINT3D impactVector;
	if(!BuildImpactVector(line.sp, line.ep, startSide > 0, impactVector))
		return false;
	for(int edge = 0; edge < 3; ++edge)
	{
		POINT3D extruded;
		if(!AddVector(triangle[edge], impactVector, extruded))
			return false;
		if(EdgePlaneProduct(triangle[edge], triangle[(edge + 1) % 3], extruded, line.sp) > 0)
			return false;
	}
	return true;
}

inline bool SegmentIntersectsFacePlane(const smSTAGE3D &stage, const smSTAGE_FACE &face,
	const POINT3D &start, const POINT3D &end)
{
	const smSTAGE_VERTEX *vertices[3] = { 0, 0, 0 };
	if(!summit::rendering::legacy3d::stage_geometry::FaceVertices(stage, face, vertices))
		return false;

	std::int64_t px[3], py[3], pz[3];
	for(int corner = 0; corner < 3; ++corner)
	{
		px[corner] = static_cast<std::int64_t>(vertices[corner]->x) >> 5;
		py[corner] = static_cast<std::int64_t>(vertices[corner]->y) >> 5;
		pz[corner] = static_cast<std::int64_t>(vertices[corner]->z) >> 5;
	}
	const std::int64_t sx = static_cast<std::int64_t>(start.x) >> 5;
	const std::int64_t sy = static_cast<std::int64_t>(start.y) >> 5;
	const std::int64_t sz = static_cast<std::int64_t>(start.z) >> 5;
	const std::int64_t ex = static_cast<std::int64_t>(end.x) >> 5;
	const std::int64_t ey = static_cast<std::int64_t>(end.y) >> 5;
	const std::int64_t ez = static_cast<std::int64_t>(end.z) >> 5;

	const std::int64_t ux = px[1] - px[0];
	const std::int64_t uy = py[1] - py[0];
	const std::int64_t uz = pz[1] - pz[0];
	const std::int64_t vx = px[2] - px[0];
	const std::int64_t vy = py[2] - py[0];
	const std::int64_t vz = pz[2] - pz[0];
	const std::int64_t nx = (uy * vz - uz * vy) >> 3;
	const std::int64_t ny = (uz * vx - ux * vz) >> 3;
	const std::int64_t nz = (ux * vy - uy * vx) >> 3;
	const std::int64_t lx = ex - sx;
	const std::int64_t ly = ey - sy;
	const std::int64_t lz = ez - sz;
	const std::int64_t denominator = (nx * lx + ny * ly + nz * lz) >> 3;
	if(denominator == 0)
		return false;

	const std::int64_t numerator =
		(-nx * sx + nx * px[0]) + (-ny * sy + ny * py[0]) + (-nz * sz + nz * pz[0]);
	const std::int64_t t = (numerator << 3) / denominator;
	return t >= 0 && t <= 8;
}

inline bool AnyLineIntersectsFace(const smSTAGE3D &stage, const smSTAGE_FACE &face,
	const smLINE3D *lines, int lineCount)
{
	if(!lines || lineCount <= 0)
		return false;
	for(int index = 0; index < lineCount; ++index)
	{
		if(LineIntersectsFace(stage, face, lines[index]))
			return true;
	}
	return false;
}

} // namespace stage_collision
} // namespace legacy3d
} // namespace rendering
} // namespace summit
